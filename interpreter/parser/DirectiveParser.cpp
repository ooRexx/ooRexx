/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
/*                                                                            */
/* Parsing methods for processing directive instructions.                     */
/*                                                                            */
/******************************************************************************/


void LanguageParser::checkDirective(int errorCode)
/******************************************************************************/
/* Function:  Verify that no code follows a directive except for more         */
/*            directive instructions.                                         */
/******************************************************************************/
{
    // save the clause location so we can reset for errors
    SourceLocation location = clauseLocation;

    this->nextClause();                  /* get the next clause               */
    /* have a next clause?               */
    if (!(this->flags&no_clause))
    {
        RexxToken *token = nextReal();     /* get the first token               */
                                           /* not a directive start?            */
        if (token->classId != TOKEN_DCOLON)
        {
            /* this is an error                  */
            syntaxError(errorCode);
        }
        firstToken();                      /* reset to the first token          */
        this->reclaimClause();             /* give back to the source object    */
    }
    // this resets the current clause location so that any errors on the current
    // clause detected after the clause check reports this on the correct line
    // number
    clauseLocation = location;
}


/**
 * Test if a directive is followed by a body of Rexx code
 * instead of another directive or the end of the source.
 *
 * @return True if there is a non-directive clause following the current
 *         clause.
 */
bool LanguageParser::hasBody()
{
    // assume there's no body here
    bool result = false;

    // if we have anything to look at, see if it is a directive or not.
    this->nextClause();
    if (!(this->flags&no_clause))
    {
        // we have a clause, now check if this is a directive or not
        RexxToken *token = nextReal();
        // not a "::", not a directive, which means we have real code to deal with
        result = token->classId != TOKEN_DCOLON;
        // reset this clause entirely so we can start parsing for real.
        firstToken();
        this->reclaimClause();
    }
    return result;
}


#define DEFAULT_GUARD    0             /* using defualt guarding            */
#define GUARDED_METHOD   1             /* method is a guarded one           */
#define UNGUARDED_METHOD 2             /* method is unguarded               */

#define DEFAULT_PROTECTION 0           /* using defualt protection          */
#define PROTECTED_METHOD   1           /* security manager permission needed*/
#define UNPROTECTED_METHOD 2           /* no protection.                    */

#define DEFAULT_ACCESS_SCOPE      0    /* using defualt scope               */
#define PUBLIC_SCOPE       1           /* publicly accessible               */
#define PRIVATE_SCOPE      2           /* private scope                     */

/**
 * Process a ::CLASS directive for a source file.
 */
void LanguageParser::classDirective()
{
    RexxToken *token = nextReal();       /* get the next token                */
    /* not a symbol or a string          */
    if (!token->isSymbolOrLiteral())
    {
        /* report an error                   */
        syntaxError(Error_Symbol_or_string_class);
    }
    RexxString *name = token->value;             /* get the routine name              */
                                     /* get the exposed name version      */
    RexxString *public_name = this->commonString(name->upper());
    /* does this already exist?          */
    if (this->class_dependencies->entry(public_name) != OREF_NULL)
    {
        /* have an error here                */
        syntaxError(Error_Translation_duplicate_class);
    }
    /* create a dependencies list        */
    this->flags |= _install;         /* have information to install       */

    // create a class directive and add this to the dependency list
    OrefSet(this, this->active_class, new ClassDirective(name, public_name, this->clause));
    this->class_dependencies->put((RexxObject *)active_class, public_name);
    // and also add to the classes list
    this->classes->append((RexxObject *)this->active_class);

    int  Public = DEFAULT_ACCESS_SCOPE;   /* haven't seen the keyword yet      */
    for (;;)
    {                       /* now loop on the option keywords   */
        token = nextReal();            /* get the next token                */
                                       /* reached the end?                  */
        if (token->isEndOfClause())
        {
            break;                       /* get out of here                   */
        }
                                         /* not a symbol token?               */
        else if (!token->isSymbol())
        {
            /* report an error                   */
            syntaxError(Error_Invalid_subkeyword_class, token);
        }
        else
        {                         /* have some sort of option keyword  */
                                  /* get the keyword type              */
            int type = this->subDirective(token);
            switch (type)
            {              /* process each sub keyword          */
                    /* ::CLASS name METACLASS metaclass  */
                case SUBDIRECTIVE_METACLASS:
                    /* already had a METACLASS?          */
                    if (active_class->getMetaClass() != OREF_NULL)
                    {
                        syntaxError(Error_Invalid_subkeyword_class, token);
                    }
                    token = nextReal();      /* get the next token                */
                                             /* not a symbol or a string          */
                    if (!token->isSymbolOrLiteral())
                    {
                        /* report an error                   */
                        syntaxError(Error_Symbol_or_string_metaclass, token);
                    }
                                             /* tag the active class              */
                    this->active_class->setMetaClass(token->value);
                    break;


                case SUBDIRECTIVE_PUBLIC:  /* ::CLASS name PUBLIC               */
                    if (Public != DEFAULT_ACCESS_SCOPE)  /* already had one of these?         */
                    {
                                             /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_class, token);
                    }
                    Public = PUBLIC_SCOPE;   /* turn on the seen flag             */
                                             /* just set this as a public object  */
                    this->active_class->setPublic();
                    break;

                case SUBDIRECTIVE_PRIVATE: /* ::CLASS name PUBLIC               */
                    if (Public != DEFAULT_ACCESS_SCOPE)  /* already had one of these?         */
                    {
                                             /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_class, token);
                    }
                    Public = PRIVATE_SCOPE;  /* turn on the seen flag             */
                    break;
                    /* ::CLASS name SUBCLASS sclass      */
                case SUBDIRECTIVE_SUBCLASS:
                    // If we have a subclass set already, this is an error
                    if (active_class->getSubClass() != OREF_NULL)
                    {
                                             /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_class, token);
                    }
                    token = nextReal();      /* get the next token                */
                                             /* not a symbol or a string          */
                    if (!token->isSymbolOrLiteral())
                    {
                        /* report an error                   */
                        syntaxError(Error_Symbol_or_string_subclass);
                    }
                    /* set the subclass information      */
                    this->active_class->setSubClass(token->value);
                    break;
                    /* ::CLASS name MIXINCLASS mclass    */
                case SUBDIRECTIVE_MIXINCLASS:
                    // If we have a subclass set already, this is an error
                    if (active_class->getSubClass() != OREF_NULL)
                    {
                                             /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_class, token);
                    }
                    token = nextReal();      /* get the next token                */
                                             /* not a symbol or a string          */
                    if (!token->isSymbolOrLiteral())
                    {
                        /* report an error                   */
                        syntaxError(Error_Symbol_or_string_mixinclass);
                    }
                    /* set the subclass information      */
                    this->active_class->setMixinClass(token->value);
                    break;
                    /* ::CLASS name INHERIT iclasses     */
                case SUBDIRECTIVE_INHERIT:
                    token = nextReal();      /* get the next token                */
                                             /* nothing after the keyword?        */
                    if (token->isEndOfClause())
                    {
                        /* report an error                   */
                        syntaxError(Error_Symbol_or_string_inherit, token);
                    }
                    while (!token->isEndOfClause())
                    {
                        /* not a symbol or a string          */
                        if (!token->isSymbolOrLiteral())
                        {
                            /* report an error                   */
                            syntaxError(Error_Symbol_or_string_inherit, token);
                        }
                        /* add to the inherit list           */
                        this->active_class->addInherits(token->value);
                        token = nextReal();    /* step to the next token            */
                    }
                    previousToken();         /* step back a token                 */
                    break;

                default:                   /* invalid keyword                   */
                    /* this is an error                  */
                    syntaxError(Error_Invalid_subkeyword_class, token);
                    break;
            }
        }
    }
}


/**
 * check for a duplicate method.
 *
 * @param name   The name to check.
 * @param classMethod
 *               Indicates whether this is a check for a CLASS or INSTANCE method.
 * @param errorMsg
 *               The error code to use if there is a duplicate.
 */
void LanguageParser::checkDuplicateMethod(RexxString *name, bool classMethod, int errorMsg)
{
    /* no previous ::CLASS directive?    */
    if (this->active_class == OREF_NULL)
    {
        if (classMethod)             /* supposed to be a class method?    */
        {
                                     /* this is an error                  */
            syntaxError(Error_Translation_missing_class);
        }
        /* duplicate method name?            */
        if (this->methods->entry(name) != OREF_NULL)
        {
            /* this is an error                  */
            syntaxError(errorMsg);
        }
    }
    else
    {                                /* add the method to the active class*/
        if (active_class->checkDuplicateMethod(name, classMethod))
        {
            /* this is an error                  */
            syntaxError(errorMsg);
        }
    }
}


/**
 * Add a new method to this compilation.
 *
 * @param name   The directory name of the method.
 * @param method The method object.
 * @param classMethod
 *               The class/instance method indicator.
 */
void LanguageParser::addMethod(RexxString *name, RexxMethod *method, bool classMethod)
{
    if (this->active_class == OREF_NULL)
    {
        this->methods->setEntry(name, method);
    }
    else
    {
        active_class->addMethod(name, method, classMethod);
    }
}



/**
 * Process a ::METHOD directive in a source file.
 */
void LanguageParser::methodDirective()
{
    int  Private = DEFAULT_ACCESS_SCOPE;    /* this is a public method           */
    int  Protected = DEFAULT_PROTECTION;  /* and is not protected yet          */
    int guard = DEFAULT_GUARD;       /* default is guarding               */
    bool Class = false;              /* default is an instance method     */
    bool Attribute = false;          /* init Attribute flag               */
    bool abstractMethod = false;     // this is an abstract method
    RexxToken *token = nextReal();   /* get the next token                */
    RexxString *externalname = OREF_NULL;       /* not an external method yet        */

                                     /* not a symbol or a string          */
    if (!token->isSymbolOrLiteral())
    {
        /* report an error                   */
        syntaxError(Error_Symbol_or_string_method, token);
    }
    RexxString *name = token->value; /* get the string name               */
                                     /* and the name form also            */
    RexxString *internalname = this->commonString(name->upper());
    for (;;)
    {                       /* now loop on the option keywords   */
        token = nextReal();            /* get the next token                */
                                       /* reached the end?                  */
        if (token->isEndOfClause())
        {
            break;                       /* get out of here                   */
        }
                                         /* not a symbol token?               */
        else if (!token->isSymbol())
        {
            /* report an error                   */
            syntaxError(Error_Invalid_subkeyword_method, token);
        }
        else
        {                         /* have some sort of option keyword  */
                                  /* process each sub keyword          */
            switch (this->subDirective(token))
            {
                /* ::METHOD name CLASS               */
                case SUBDIRECTIVE_CLASS:
                    if (Class)               /* had one of these already?         */
                    {
                                             /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }
                    Class = true;            /* flag this for later processing    */
                    break;
                    /* ::METHOD name EXTERNAL extname    */
                case SUBDIRECTIVE_EXTERNAL:
                    /* already had an external?          */
                    if (externalname != OREF_NULL || abstractMethod)
                    {
                        /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }
                    token = nextReal();      /* get the next token                */
                                             /* not a string?                     */
                    if (!token->isLiteral())
                    {
                        /* report an error                   */
                        syntaxError(Error_Symbol_or_string_external, token);
                    }
                    externalname = token->value;
                    break;
                    /* ::METHOD name PRIVATE             */
                case SUBDIRECTIVE_PRIVATE:
                    if (Private != DEFAULT_ACCESS_SCOPE)   /* already seen one of these?        */
                    {
                                             /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }
                    Private = PRIVATE_SCOPE;           /* flag for later processing         */
                    break;
                    /* ::METHOD name PUBLIC             */
                case SUBDIRECTIVE_PUBLIC:
                    if (Private != DEFAULT_ACCESS_SCOPE)   /* already seen one of these?        */
                    {
                                             /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }
                    Private = PUBLIC_SCOPE;        /* flag for later processing         */
                    break;
                    /* ::METHOD name PROTECTED           */
                case SUBDIRECTIVE_PROTECTED:
                    if (Protected != DEFAULT_PROTECTION)           /* already seen one of these?        */
                    {
                                             /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }
                    Protected = PROTECTED_METHOD;        /* flag for later processing         */
                    break;
                    /* ::METHOD name UNPROTECTED           */
                case SUBDIRECTIVE_UNPROTECTED:
                    if (Protected != DEFAULT_PROTECTION)           /* already seen one of these?        */
                    {
                                             /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }
                    Protected = UNPROTECTED_METHOD;      /* flag for later processing         */
                    break;
                    /* ::METHOD name UNGUARDED           */
                case SUBDIRECTIVE_UNGUARDED:
                    /* already seen one of these?        */
                    if (guard != DEFAULT_GUARD)
                    {
                        /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }
                    guard = UNGUARDED_METHOD;/* flag for later processing         */
                    break;
                    /* ::METHOD name GUARDED             */
                case SUBDIRECTIVE_GUARDED:
                    /* already seen one of these?        */
                    if (guard != DEFAULT_GUARD)
                    {
                        /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }
                    guard = GUARDED_METHOD;  /* flag for later processing         */
                    break;
                    /* ::METHOD name ATTRIBUTE           */
                case SUBDIRECTIVE_ATTRIBUTE:

                    if (Attribute)           /* already seen one of these?        */
                    {
                                             /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }
                    // cannot have an abstract attribute
                    if (abstractMethod)
                    {
                        /* EXTERNAL and ATTRIBUTE are        */
                        /* mutually exclusive                */
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }
                    Attribute = true;        /* flag for later processing         */
                    break;

                                           /* ::METHOD name ABSTRACT            */
                case SUBDIRECTIVE_ABSTRACT:

                    if (abstractMethod || externalname != OREF_NULL)
                    {
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }
                    // not compatible with ATTRIBUTE or EXTERNAL
                    if (externalname != OREF_NULL || Attribute)
                    {
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }
                    abstractMethod = true;   /* flag for later processing         */
                    break;


                default:                   /* invalid keyword                   */
                    /* this is an error                  */
                    syntaxError(Error_Invalid_subkeyword_method, token);
                    break;
            }
        }
    }

    // go check for a duplicate and validate the use of the CLASS modifier
    checkDuplicateMethod(internalname, Class, Error_Translation_duplicate_method);


    RexxMethod *_method = OREF_NULL;
    // is this an attribute method?
    if (Attribute)
    {
        // now get this as the setter method.
        RexxString *setterName = commonString(internalname->concatWithCstring("="));
        // need to check for duplicates on that too
        checkDuplicateMethod(setterName, Class, Error_Translation_duplicate_method);

                                       /* Go check the next clause to make  */
        this->checkDirective(Error_Translation_attribute_method);        /* sure that no code follows         */
        // this might be externally defined setters and getters.
        if (externalname != OREF_NULL)
        {
            RexxString *library = OREF_NULL;
            RexxString *procedure = OREF_NULL;
            decodeExternalMethod(internalname, externalname, library, procedure);
            // now create both getter and setting methods from the information.
            _method = createNativeMethod(internalname, library, procedure->concatToCstring("GET"));
            _method->setAttributes(Private == PRIVATE_SCOPE, Protected == PROTECTED_METHOD, guard != UNGUARDED_METHOD);
            // add to the compilation
            addMethod(internalname, _method, Class);

            _method = createNativeMethod(setterName, library, procedure->concatToCstring("SET"));
            _method->setAttributes(Private == PRIVATE_SCOPE, Protected == PROTECTED_METHOD, guard != UNGUARDED_METHOD);
            // add to the compilation
            addMethod(setterName, _method, Class);
        }
        else
        {
            // now get a variable retriever to get the property
            RexxVariableBase *retriever = this->getRetriever(name);

            // create the method pair and quit.
            createAttributeGetterMethod(internalname, retriever, Class, Private == PRIVATE_SCOPE,
                Protected == PROTECTED_METHOD, guard != UNGUARDED_METHOD);
            createAttributeSetterMethod(setterName, retriever, Class, Private == PRIVATE_SCOPE,
                Protected == PROTECTED_METHOD, guard != UNGUARDED_METHOD);
        }
        return;
    }
    // abstract method?
    else if (abstractMethod)
    {
                                       /* Go check the next clause to make  */
        this->checkDirective(Error_Translation_abstract_method);        /* sure that no code follows         */
        // this uses a special code block
        BaseCode *code = new AbstractCode();
        _method = new RexxMethod(name, code);
    }
    /* not an external method?           */
    else if (externalname == OREF_NULL)
    {
        // NOTE:  It is necessary to translate the block and protect the code
        // before allocating the RexxMethod object.  The new operator allocates the
        // the object first, then evaluates the constructor arguments after the allocation.
        // Since the translateBlock() call will allocate a lot of new objects before returning,
        // there's a high probability that the method object can get garbage collected before
        // there is any opportunity to protect the object.
        RexxCode *code = this->translateBlock(OREF_NULL);
        this->saveObject((RexxObject *)code);

        /* go do the next block of code      */
        _method = new RexxMethod(name, code);
    }
    else
    {
        RexxString *library = OREF_NULL;
        RexxString *procedure = OREF_NULL;
        decodeExternalMethod(internalname, externalname, library, procedure);

        /* go check the next clause to make  */
        this->checkDirective(Error_Translation_external_method);
        // and make this into a method object.
        _method = createNativeMethod(name, library, procedure);
    }
    _method->setAttributes(Private == PRIVATE_SCOPE, Protected == PROTECTED_METHOD, guard != UNGUARDED_METHOD);
    // add to the compilation
    addMethod(internalname, _method, Class);
}



/**
 * Process a ::OPTIONS directive in a source file.
 */
void LanguageParser::optionsDirective()
{
    // all options are of a keyword/value pattern
    for (;;)
    {
        RexxToken *token = nextReal(); /* get the next token                */
                                       /* reached the end?                  */
        if (token->isEndOfClause())
        {
            break;                       /* get out of here                   */
        }
                                         /* not a symbol token?               */
        else if (!token->isSymbol())
        {
            /* report an error                   */
            syntaxError(Error_Invalid_subkeyword_options, token);
        }
        else
        {                         /* have some sort of option keyword  */
                                  /* process each sub keyword          */
            switch (this->subDirective(token))
            {
                // ::OPTIONS DIGITS nnnn
                case SUBDIRECTIVE_DIGITS:
                {
                    token = nextReal();      /* get the next token                */
                                             /* not a string?                     */
                    if (!token->isSymbolOrLiteral())
                    {
                        /* report an error                   */
                        syntaxError(Error_Symbol_or_string_digits_value, token);
                    }
                    RexxString *value = token->value;          /* get the string value              */

                    if (!value->requestUnsignedNumber(digits, number_digits()) || digits < 1)
                    {
                        /* report an exception               */
                        syntaxError(Error_Invalid_whole_number_digits, value);
                    }
                    /* problem with the fuzz setting?    */
                    if (digits <= fuzz)
                    {
                        /* this is an error                  */
                        reportException(Error_Expression_result_digits, digits, fuzz);
                    }
                    break;
                }
                // ::OPTIONS FORM ENGINEERING/SCIENTIFIC
                case SUBDIRECTIVE_FORM:
                    token = nextReal();      /* get the next token                */
                                             /* not a string?                     */
                    if (!token->isSymbol())
                    {
                        /* report an error                   */
                        syntaxError(Error_Invalid_subkeyword_form, token);
                    }
                    /* resolve the subkeyword            */
                    /* and process                       */
                    switch (this->subKeyword(token))
                    {

                        case SUBKEY_SCIENTIFIC:        /* NUMERIC FORM SCIENTIFIC           */
                            form = Numerics::FORM_SCIENTIFIC;
                            break;

                        case SUBKEY_ENGINEERING:     /* NUMERIC FORM ENGINEERING          */
                            form = Numerics::FORM_ENGINEERING;
                            break;

                        default:                     /* invalid subkeyword                */
                            /* raise an error                    */
                            syntaxError(Error_Invalid_subkeyword_form, token);
                            break;

                    }
                    break;
                // ::OPTIONS FUZZ nnnn
                case SUBDIRECTIVE_FUZZ:
                {
                    token = nextReal();      /* get the next token                */
                                             /* not a string?                     */
                    if (!token->isSymbolOrLiteral())
                    {
                        /* report an error                   */
                        syntaxError(Error_Symbol_or_string_fuzz_value, token);
                    }
                    RexxString *value = token->value;          /* get the string value              */

                    if (!value->requestUnsignedNumber(fuzz, number_digits()))
                    {
                        /* report an exception               */
                        syntaxError(Error_Invalid_whole_number_fuzz, value);
                    }
                    /* problem with the digits setting?  */
                    if (fuzz >= digits)
                    {
                        /* and issue the error               */
                        reportException(Error_Expression_result_digits, digits, fuzz);
                    }
                    break;
                }
                // ::OPTIONS TRACE setting
                case SUBDIRECTIVE_TRACE:
                {
                    token = nextReal();      /* get the next token                */
                                             /* not a string?                     */
                    if (!token->isSymbolOrLiteral())
                    {
                        /* report an error                   */
                        syntaxError(Error_Symbol_or_string_trace_value, token);
                    }
                    RexxString *value = token->value;          /* get the string value              */
                    char badOption = 0;
                                                 /* process the setting               */
                    if (!parseTraceSetting(value, traceSetting, traceFlags, badOption))
                    {
                        syntaxError(Error_Invalid_trace_trace, new_string(&badOption, 1));
                    }
                    break;
                }

                default:                   /* invalid keyword                   */
                    /* this is an error                  */
                    syntaxError(Error_Invalid_subkeyword_options, token);
                    break;
            }
        }
    }
}

/**
 * Create a native method from a specification.
 *
 * @param name      The method name.
 * @param library   The library containing the method.
 * @param procedure The name of the procedure within the package.
 *
 * @return A method object representing this method.
 */
RexxMethod *LanguageParser::createNativeMethod(RexxString *name, RexxString *library, RexxString *procedure)
{
                                 /* create a new native method        */
    RexxNativeCode *nmethod = PackageManager::resolveMethod(library, procedure);
    // raise an exception if this entry point is not found.
    if (nmethod == OREF_NULL)
    {
         syntaxError(Error_External_name_not_found_method, procedure);
    }
    // this might return a different object if this has been used already
    nmethod = (RexxNativeCode *)nmethod->setSourceObject(this);
    /* turn into a real method object    */
    return new RexxMethod(name, nmethod);
}

/**
 * Decode an external library method specification.
 *
 * @param methodName The internal name of the method (uppercased).
 * @param externalSpec
 *                   The external specification string.
 * @param library    The returned library name.
 * @param procedure  The returned package name.
 */
void LanguageParser::decodeExternalMethod(RexxString *methodName, RexxString *externalSpec, RexxString *&library, RexxString *&procedure)
{
    // this is the default
    procedure = methodName;
    library = OREF_NULL;

                            /* convert external into words       */
    RexxArray *_words = this->words(externalSpec);
    /* not 'LIBRARY library [entry]' form? */
    if (((RexxString *)(_words->get(1)))->strCompare(CHAR_LIBRARY))
    {
        // full library with entry name version?
        if (_words->size() == 3)
        {
            library = (RexxString *)_words->get(2);
            procedure = (RexxString *)_words->get(3);
        }
        else if (_words->size() == 2)
        {
            library = (RexxString *)_words->get(2);
        }
        else  // wrong number of tokens
        {
                                     /* this is an error                  */
            syntaxError(Error_Translation_bad_external, externalSpec);
        }
    }
    else
    {
        /* unknown external type             */
        syntaxError(Error_Translation_bad_external, externalSpec);
    }
}

#define ATTRIBUTE_BOTH 0
#define ATTRIBUTE_GET  1
#define ATTRIBUTE_SET  2


/**
 * Process a ::ATTRIBUTE directive in a source file.
 */
void LanguageParser::attributeDirective()
{
    int  Private = DEFAULT_ACCESS_SCOPE;    /* this is a public method           */
    int  Protected = DEFAULT_PROTECTION;  /* and is not protected yet          */
    int  guard = DEFAULT_GUARD;       /* default is guarding               */
    int  style = ATTRIBUTE_BOTH;      // by default, we create both methods for the attribute.
    bool Class = false;              /* default is an instance method     */
    bool abstractMethod = false;     // by default, creating a concrete method
    RexxToken *token = nextReal();   /* get the next token                */

                                     /* not a symbol or a string          */
    if (!token->isSymbolOrLiteral())
    {
        /* report an error                   */
        syntaxError(Error_Symbol_or_string_attribute, token);
    }
    RexxString *name = token->value; /* get the string name               */
                                     /* and the name form also            */
    RexxString *internalname = this->commonString(name->upper());
    RexxString *externalname = OREF_NULL;

    for (;;)
    {                       /* now loop on the option keywords   */
        token = nextReal();            /* get the next token                */
                                       /* reached the end?                  */
        if (token->isEndOfClause())
        {
            break;                       /* get out of here                   */
        }
                                         /* not a symbol token?               */
        else if (!token->isSymbol())
        {
            /* report an error                   */
            syntaxError(Error_Invalid_subkeyword_attribute, token);
        }
        else
        {                         /* have some sort of option keyword  */
                                  /* process each sub keyword          */
            switch (this->subDirective(token))
            {
                case SUBDIRECTIVE_GET:
                    // only one of GET/SET allowed
                    if (style != ATTRIBUTE_BOTH)
                    {
                        syntaxError(Error_Invalid_subkeyword_attribute, token);
                    }
                    style = ATTRIBUTE_GET;
                    break;

                case SUBDIRECTIVE_SET:
                    // only one of GET/SET allowed
                    if (style != ATTRIBUTE_BOTH)
                    {
                        syntaxError(Error_Invalid_subkeyword_attribute, token);
                    }
                    style = ATTRIBUTE_SET;
                    break;


                /* ::ATTRIBUTE name CLASS               */
                case SUBDIRECTIVE_CLASS:
                    if (Class)               /* had one of these already?         */
                    {
                                             /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_attribute, token);
                    }
                    Class = true;            /* flag this for later processing    */
                    break;
                case SUBDIRECTIVE_PRIVATE:
                    if (Private != DEFAULT_ACCESS_SCOPE)   /* already seen one of these?        */
                    {
                                             /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_attribute, token);
                    }
                    Private = PRIVATE_SCOPE;           /* flag for later processing         */
                    break;
                    /* ::METHOD name PUBLIC             */
                case SUBDIRECTIVE_PUBLIC:
                    if (Private != DEFAULT_ACCESS_SCOPE)   /* already seen one of these?        */
                    {
                                             /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_attribute, token);
                    }
                    Private = PUBLIC_SCOPE;        /* flag for later processing         */
                    break;
                    /* ::METHOD name PROTECTED           */
                case SUBDIRECTIVE_PROTECTED:
                    if (Protected != DEFAULT_PROTECTION)           /* already seen one of these?        */
                    {
                                             /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_attribute, token);
                    }
                    Protected = PROTECTED_METHOD;        /* flag for later processing         */
                    break;
                case SUBDIRECTIVE_UNPROTECTED:
                    if (Protected != DEFAULT_PROTECTION)           /* already seen one of these?        */
                    {
                                             /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_attribute, token);
                    }
                    Protected = UNPROTECTED_METHOD;      /* flag for later processing         */
                    break;
                    /* ::METHOD name UNGUARDED           */
                case SUBDIRECTIVE_UNGUARDED:
                    /* already seen one of these?        */
                    if (guard != DEFAULT_GUARD)
                    {
                        /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_attribute, token);
                    }
                    guard = UNGUARDED_METHOD;/* flag for later processing         */
                    break;
                    /* ::METHOD name GUARDED             */
                case SUBDIRECTIVE_GUARDED:
                    /* already seen one of these?        */
                    if (guard != DEFAULT_GUARD)
                    {
                        /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_attribute, token);
                    }
                    guard = GUARDED_METHOD;  /* flag for later processing         */
                    break;
                    /* ::METHOD name ATTRIBUTE           */
                case SUBDIRECTIVE_EXTERNAL:
                    /* already had an external?          */
                    if (externalname != OREF_NULL || abstractMethod)
                    {
                        /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_attribute, token);
                    }
                    token = nextReal();      /* get the next token                */
                                             /* not a string?                     */
                    if (!token->isLiteral())
                    {
                        /* report an error                   */
                        syntaxError(Error_Symbol_or_string_external, token);
                    }
                    externalname = token->value;
                    break;
                                           /* ::METHOD name ABSTRACT            */
                case SUBDIRECTIVE_ABSTRACT:

                    if (abstractMethod || externalname != OREF_NULL)
                    {
                        syntaxError(Error_Invalid_subkeyword_attribute, token);
                    }
                    abstractMethod = true;   /* flag for later processing         */
                    break;


                default:                   /* invalid keyword                   */
                    /* this is an error                  */
                    syntaxError(Error_Invalid_subkeyword_attribute, token);
                    break;
            }
        }
    }

    // both attributes same default properties?

    // now get a variable retriever to get the property (do this before checking the body
    // so errors get diagnosed on the correct line),
    RexxVariableBase *retriever = this->getRetriever(name);

    switch (style)
    {
        case ATTRIBUTE_BOTH:
        {
            checkDuplicateMethod(internalname, Class, Error_Translation_duplicate_attribute);
            // now get this as the setter method.
            RexxString *setterName = commonString(internalname->concatWithCstring("="));
            checkDuplicateMethod(setterName, Class, Error_Translation_duplicate_attribute);

            // no code can follow the automatically generated methods
            this->checkDirective(Error_Translation_body_error);
            if (externalname != OREF_NULL)
            {
                RexxString *library = OREF_NULL;
                RexxString *procedure = OREF_NULL;
                decodeExternalMethod(internalname, externalname, library, procedure);
                // now create both getter and setting methods from the information.
                RexxMethod *_method = createNativeMethod(internalname, library, procedure->concatToCstring("GET"));
                _method->setAttributes(Private == PRIVATE_SCOPE, Protected == PROTECTED_METHOD, guard != UNGUARDED_METHOD);
                // add to the compilation
                addMethod(internalname, _method, Class);

                _method = createNativeMethod(setterName, library, procedure->concatToCstring("SET"));
                _method->setAttributes(Private == PRIVATE_SCOPE, Protected == PROTECTED_METHOD, guard != UNGUARDED_METHOD);
                // add to the compilation
                addMethod(setterName, _method, Class);
            }
            // abstract method?
            else if (abstractMethod)
            {
                // create the method pair and quit.
                createAbstractMethod(internalname, Class, Private == PRIVATE_SCOPE,
                    Protected == PROTECTED_METHOD, guard != UNGUARDED_METHOD);
                createAbstractMethod(setterName, Class, Private == PRIVATE_SCOPE,
                    Protected == PROTECTED_METHOD, guard != UNGUARDED_METHOD);
            }
            else
            {
                // create the method pair and quit.
                createAttributeGetterMethod(internalname, retriever, Class, Private == PRIVATE_SCOPE,
                    Protected == PROTECTED_METHOD, guard != UNGUARDED_METHOD);
                createAttributeSetterMethod(setterName, retriever, Class, Private == PRIVATE_SCOPE,
                    Protected == PROTECTED_METHOD, guard != UNGUARDED_METHOD);
            }
            break;

        }

        case ATTRIBUTE_GET:       // just the getter method
        {
            checkDuplicateMethod(internalname, Class, Error_Translation_duplicate_attribute);
            // external?  resolve the method
            if (externalname != OREF_NULL)
            {
                // no code can follow external methods
                this->checkDirective(Error_Translation_external_attribute);
                RexxString *library = OREF_NULL;
                RexxString *procedure = OREF_NULL;
                decodeExternalMethod(internalname, externalname, library, procedure);
                // if there was no procedure explicitly given, create one using the GET/SET convention
                if (internalname == procedure)
                {
                    procedure = procedure->concatToCstring("GET");
                }
                // now create both getter and setting methods from the information.
                RexxMethod *_method = createNativeMethod(internalname, library, procedure);
                _method->setAttributes(Private == PRIVATE_SCOPE, Protected == PROTECTED_METHOD, guard != UNGUARDED_METHOD);
                // add to the compilation
                addMethod(internalname, _method, Class);
            }
            // abstract method?
            else if (abstractMethod)
            {
                // no code can follow abstract methods
                this->checkDirective(Error_Translation_abstract_attribute);
                // create the method pair and quit.
                createAbstractMethod(internalname, Class, Private == PRIVATE_SCOPE,
                    Protected == PROTECTED_METHOD, guard != UNGUARDED_METHOD);
            }
            // either written in ooRexx or is automatically generated.
            else {
                if (hasBody())
                {
                    createMethod(internalname, Class, Private == PRIVATE_SCOPE,
                        Protected == PROTECTED_METHOD, guard != UNGUARDED_METHOD);
                }
                else
                {
                    createAttributeGetterMethod(internalname, retriever, Class, Private == PRIVATE_SCOPE,
                        Protected == PROTECTED_METHOD, guard != UNGUARDED_METHOD);
                }
            }
            break;
        }

        case ATTRIBUTE_SET:
        {
            // now get this as the setter method.
            RexxString *setterName = commonString(internalname->concatWithCstring("="));
            checkDuplicateMethod(setterName, Class, Error_Translation_duplicate_attribute);
            // external?  resolve the method
            if (externalname != OREF_NULL)
            {
                // no code can follow external methods
                this->checkDirective(Error_Translation_external_attribute);
                RexxString *library = OREF_NULL;
                RexxString *procedure = OREF_NULL;
                decodeExternalMethod(internalname, externalname, library, procedure);
                // if there was no procedure explicitly given, create one using the GET/SET convention
                if (internalname == procedure)
                {
                    procedure = procedure->concatToCstring("SET");
                }
                // now create both getter and setting methods from the information.
                RexxMethod *_method = createNativeMethod(setterName, library, procedure);
                _method->setAttributes(Private == PRIVATE_SCOPE, Protected == PROTECTED_METHOD, guard != UNGUARDED_METHOD);
                // add to the compilation
                addMethod(setterName, _method, Class);
            }
            // abstract method?
            else if (abstractMethod)
            {
                // no code can follow abstract methods
                this->checkDirective(Error_Translation_abstract_attribute);
                // create the method pair and quit.
                createAbstractMethod(setterName, Class, Private == PRIVATE_SCOPE,
                    Protected == PROTECTED_METHOD, guard != UNGUARDED_METHOD);
            }
            else
            {
                if (hasBody())        // just the getter method
                {
                    createMethod(setterName, Class, Private == PRIVATE_SCOPE,
                        Protected == PROTECTED_METHOD, guard != UNGUARDED_METHOD);
                }
                else
                {
                    createAttributeSetterMethod(setterName, retriever, Class, Private == PRIVATE_SCOPE,
                        Protected == PROTECTED_METHOD, guard != UNGUARDED_METHOD);
                }
            }
            break;
        }
    }
}


/**
 * Process a ::CONSTANT directive in a source file.
 */
void LanguageParser::constantDirective()
{
    RexxToken *token = nextReal();   /* get the next token                */
                                     /* not a symbol or a string          */
    if (!token->isSymbolOrLiteral())
    {
        /* report an error                   */
        syntaxError(Error_Symbol_or_string_constant, token);
    }
    RexxString *name = token->value; /* get the string name               */
                                     /* and the name form also            */
    RexxString *internalname = this->commonString(name->upper());

    // we only expect just a single value token here
    token = nextReal();                /* get the next token                */
    RexxObject *value;
                                       /* not a symbol or a string          */
    if (!token->isSymbolOrLiteral())
    {
        // if not a "+" or "-" operator, this is an error
        if (!token->isOperator() || (token->subclass != OPERATOR_SUBTRACT && token->subclass != OPERATOR_PLUS))
        {
            /* report an error                   */
            syntaxError(Error_Symbol_or_string_constant_value, token);
        }
        RexxToken *second = nextReal();
        // this needs to be a constant symbol...we check for
        // numeric below
        if (!second->isSymbol() || second->subclass != SYMBOL_CONSTANT)
        {
            /* report an error                   */
            syntaxError(Error_Symbol_or_string_constant_value, token);
        }
        // concat with the sign operator
        value = token->value->concat(second->value);
        // and validate that this a valid number
        if (value->numberString() == OREF_NULL)
        {
            /* report an error                   */
            syntaxError(Error_Symbol_or_string_constant_value, token);
        }
    }
    else
    {
        // this will be some sort of literal value
        value = this->commonString(token->value);
    }

    token = nextReal();                /* get the next token                */
    // No other options on this instruction
    if (!token->isEndOfClause())
    {
        /* report an error                   */
        syntaxError(Error_Invalid_data_constant_dir, token);
    }
    // this directive does not allow a body
    this->checkDirective(Error_Translation_constant_body);

    // check for duplicates.  We only do the class duplicate check if there
    // is an active class, otherwise we'll get a syntax error
    checkDuplicateMethod(internalname, false, Error_Translation_duplicate_constant);
    if (this->active_class != OREF_NULL)
    {
        checkDuplicateMethod(internalname, true, Error_Translation_duplicate_constant);
    }

    // create the method pair and quit.
    createConstantGetterMethod(internalname, value);
}


/**
 * Create a Rexx method body.
 *
 * @param name   The name of the attribute.
 * @param classMethod
 *               Indicates whether we are creating a class or instance method.
 * @param privateMethod
 *               The method's private attribute.
 * @param protectedMethod
 *               The method's protected attribute.
 * @param guardedMethod
 *               The method's guarded attribute.
 */
void LanguageParser::createMethod(RexxString *name, bool classMethod,
    bool privateMethod, bool protectedMethod, bool guardedMethod)
{
    // NOTE:  It is necessary to translate the block and protect the code
    // before allocating the RexxMethod object.  The new operator allocates the
    // the object first, then evaluates the constructor arguments after the allocation.
    // Since the translateBlock() call will allocate a lot of new objects before returning,
    // there's a high probability that the method object can get garbage collected before
    // there is any opportunity to protect the object.
    RexxCode *code = this->translateBlock(OREF_NULL);
    this->saveObject((RexxObject *)code);

    /* go do the next block of code      */
    RexxMethod *_method = new RexxMethod(name, code);
    _method->setAttributes(privateMethod, protectedMethod, guardedMethod);
    // go add the method to the accumulator
    addMethod(name, _method, classMethod);
}


/**
 * Create an ATTRIBUTE "get" method.
 *
 * @param name      The name of the attribute.
 * @param retriever
 * @param classMethod
 *                  Indicates we're adding a class or instance method.
 * @param privateMethod
 *                  The method's private attribute.
 * @param protectedMethod
 *                  The method's protected attribute.
 * @param guardedMethod
 *                  The method's guarded attribute.
 */
void LanguageParser::createAttributeGetterMethod(RexxString *name, RexxVariableBase *retriever,
    bool classMethod, bool privateMethod, bool protectedMethod, bool guardedMethod)
{
    // create the kernel method for the accessor
    BaseCode *code = new AttributeGetterCode(retriever);
    RexxMethod *_method = new RexxMethod(name, code);
    _method->setAttributes(privateMethod, protectedMethod, guardedMethod);
    // add this to the target
    addMethod(name, _method, classMethod);
}


/**
 * Create an ATTRIBUTE "set" method.
 *
 * @param name   The name of the attribute.
 * @param classMethod
 *                  Indicates we're adding a class or instance method.
 * @param privateMethod
 *               The method's private attribute.
 * @param protectedMethod
 *               The method's protected attribute.
 * @param guardedMethod
 *               The method's guarded attribute.
 */
void LanguageParser::createAttributeSetterMethod(RexxString *name, RexxVariableBase *retriever,
    bool classMethod, bool privateMethod, bool protectedMethod, bool guardedMethod)
{
    // create the kernel method for the accessor
    BaseCode *code = new AttributeSetterCode(retriever);
    RexxMethod *_method = new RexxMethod(name, code);
    _method->setAttributes(privateMethod, protectedMethod, guardedMethod);
    // add this to the target
    addMethod(name, _method, classMethod);
}


/**
 * Create an abstract method.
 *
 * @param name   The name of the method.
 * @param classMethod
 *                  Indicates we're adding a class or instance method.
 * @param privateMethod
 *               The method's private attribute.
 * @param protectedMethod
 *               The method's protected attribute.
 * @param guardedMethod
 *               The method's guarded attribute.
 */
void LanguageParser::createAbstractMethod(RexxString *name,
    bool classMethod, bool privateMethod, bool protectedMethod, bool guardedMethod)
{
    // create the kernel method for the accessor
    // this uses a special code block
    BaseCode *code = new AbstractCode();
    RexxMethod * _method = new RexxMethod(name, code);
    _method->setAttributes(privateMethod, protectedMethod, guardedMethod);
    // add this to the target
    addMethod(name, _method, classMethod);
}


/**
 * Create a CONSTANT "get" method.
 *
 * @param target The target method directory.
 * @param name   The name of the attribute.
 */
void LanguageParser::createConstantGetterMethod(RexxString *name, RexxObject *value)
{
    ConstantGetterCode *code = new ConstantGetterCode(value);
    // add this as an unguarded method
    RexxMethod *method = new RexxMethod(name, code);
    method->setUnguarded();
    if (active_class == OREF_NULL)
    {
        addMethod(name, method, false);
    }
    else
    {
        active_class->addConstantMethod(name, method);
    }
}


/**
 * Process a ::routine directive in a source file.
 */
void LanguageParser::routineDirective()
{
    RexxToken *token = nextReal();   /* get the next token                */
                                     /* not a symbol or a string          */
    if (!token->isSymbolOrLiteral())
    {
        /* report an error                   */
        syntaxError(Error_Symbol_or_string_routine, token);
    }
    RexxString *name = token->value; /* get the routine name              */
                                     /* does this already exist?          */
    if (this->routines->entry(name) != OREF_NULL)
    {
        /* have an error here                */
        syntaxError(Error_Translation_duplicate_routine);
    }
    this->flags |= _install;         /* have information to install       */
    RexxString *externalname = OREF_NULL;        /* no external name yet              */
    int Public = DEFAULT_ACCESS_SCOPE;      /* not a public routine yet          */
    for (;;)                         /* now loop on the option keywords   */
    {
        token = nextReal();            /* get the next token                */
                                       /* reached the end?                  */
        if (token->isEndOfClause())
        {
            break;                       /* get out of here                   */
        }
        /* not a symbol token?               */
        else if (!token->isSymbol())
        {
            /* report an error                   */
            syntaxError(Error_Invalid_subkeyword_routine, token);
        }
        /* process each sub keyword          */
        switch (this->subDirective(token))
        {
            /* ::ROUTINE name EXTERNAL []*/
            case SUBDIRECTIVE_EXTERNAL:
                /* already had an external?          */
                if (externalname != OREF_NULL)
                {
                    /* duplicates are invalid            */
                    syntaxError(Error_Invalid_subkeyword_class, token);
                }
                token = nextReal();        /* get the next token                */
                /* not a string?                     */
                if (!token->isLiteral())
                {
                    /* report an error                   */
                    syntaxError(Error_Symbol_or_string_requires, token);
                }
                /* external name is token value      */
                externalname = token->value;
                break;
                /* ::ROUTINE name PUBLIC             */
            case SUBDIRECTIVE_PUBLIC:
                if (Public != DEFAULT_ACCESS_SCOPE)   /* already had one of these?         */
                {
                    /* duplicates are invalid            */
                    syntaxError(Error_Invalid_subkeyword_routine, token);

                }
                Public = PUBLIC_SCOPE;     /* turn on the seen flag             */
                break;
                /* ::ROUTINE name PUBLIC             */
            case SUBDIRECTIVE_PRIVATE:
                if (Public != DEFAULT_ACCESS_SCOPE)   /* already had one of these?         */
                {
                    /* duplicates are invalid            */
                    syntaxError(Error_Invalid_subkeyword_routine, token);

                }
                Public = PRIVATE_SCOPE;    /* turn on the seen flag             */
                break;

            default:                     /* invalid keyword                   */
                /* this is an error                  */
                syntaxError(Error_Invalid_subkeyword_routine, token);
                break;
        }
    }
    {
        this->saveObject(name);          /* protect the name                  */

        if (externalname != OREF_NULL)   /* have an external routine?         */
        {
            /* convert external into words       */
            RexxArray *_words = this->words(externalname);
            // ::ROUTINE foo EXTERNAL "LIBRARY libbar [foo]"
            if (((RexxString *)(_words->get(1)))->strCompare(CHAR_LIBRARY))
            {
                RexxString *library = OREF_NULL;
                // the default entry point name is the internal name
                RexxString *entry = name;

                // full library with entry name version?
                if (_words->size() == 3)
                {
                    library = (RexxString *)_words->get(2);
                    entry = (RexxString *)_words->get(3);
                }
                else if (_words->size() == 2)
                {
                    library = (RexxString *)_words->get(2);
                }
                else  // wrong number of tokens
                {
                    /* this is an error                  */
                    syntaxError(Error_Translation_bad_external, externalname);
                }

                /* go check the next clause to make  */
                this->checkDirective(Error_Translation_external_routine);      /* sure no code follows              */
                                             /* create a new native method        */
                RoutineClass *routine = PackageManager::resolveRoutine(library, entry);
                // raise an exception if this entry point is not found.
                if (routine == OREF_NULL)
                {
                     syntaxError(Error_External_name_not_found_routine, entry);
                }
                // make sure this is attached to the source object for context information
                routine->setSourceObject(this);
                /* add to the routine directory      */
                this->routines->setEntry(name, routine);
                if (Public == PUBLIC_SCOPE)    /* a public routine?                 */
                {
                    /* add to the public directory too   */
                    this->public_routines->setEntry(name, routine);
                }
            }

            // ::ROUTINE foo EXTERNAL "REGISTERED libbar [foo]"
            else if (((RexxString *)(_words->get(1)))->strCompare(CHAR_REGISTERED))
            {
                RexxString *library = OREF_NULL;
                // the default entry point name is the internal name
                RexxString *entry = name;

                // full library with entry name version?
                if (_words->size() == 3)
                {
                    library = (RexxString *)_words->get(2);
                    entry = (RexxString *)_words->get(3);
                }
                else if (_words->size() == 2)
                {
                    library = (RexxString *)_words->get(2);
                }
                else  // wrong number of tokens
                {
                    /* this is an error                  */
                    syntaxError(Error_Translation_bad_external, externalname);
                }

                /* go check the next clause to make  */
                this->checkDirective(Error_Translation_external_routine);      /* sure no code follows              */
                                             /* create a new native method        */
                RoutineClass *routine = PackageManager::resolveRoutine(name, library, entry);
                // raise an exception if this entry point is not found.
                if (routine == OREF_NULL)
                {
                     syntaxError(Error_External_name_not_found_routine, entry);
                }
                // make sure this is attached to the source object for context information
                routine->setSourceObject(this);
                /* add to the routine directory      */
                this->routines->setEntry(name, routine);
                if (Public == PUBLIC_SCOPE)    /* a public routine?                 */
                {
                    /* add to the public directory too   */
                    this->public_routines->setEntry(name, routine);
                }
            }
            else
            {
                /* unknown external type             */
                syntaxError(Error_Translation_bad_external, externalname);
            }
        }
        else
        {
            // NOTE:  It is necessary to translate the block and protect the code
            // before allocating the RexxMethod object.  The new operator allocates the
            // the object first, then evaluates the constructor arguments after the allocation.
            // Since the translateBlock() call will allocate a lot of new objects before returning,
            // there's a high probability that the method object can get garbage collected before
            // there is any opportunity to protect the object.
            RexxCode *code = this->translateBlock(OREF_NULL);
            this->saveObject((RexxObject *)code);
            RoutineClass *routine = new RoutineClass(name, code);
            /* add to the routine directory      */
            this->routines->setEntry(name, routine);
            if (Public == PUBLIC_SCOPE)    /* a public routine?                 */
            {
                /* add to the public directory too   */
                this->public_routines->setEntry(name, routine);

            }
        }
        this->toss(name);                /* release the "Gary Cole" (GC) lock */
    }
}

/**
 * Process a ::REQUIRES directive.
 */
void LanguageParser::requiresDirective()
{
    RexxToken *token = nextReal();   /* get the next token                */
                                     /* not a symbol or a string          */
    if (!token->isSymbolOrLiteral())
    {
        /* report an error                   */
        syntaxError(Error_Symbol_or_string_requires, token);
    }
    RexxString *name = token->value; /* get the requires name             */
    token = nextReal();              /* get the next token                */
    if (!token->isEndOfClause()) /* something appear after this?      */
    {
        // this is potentially a library directive
        libraryDirective(name, token);
        return;
    }
    this->flags |= _install;         /* have information to install       */
    /* save the ::requires location      */
    this->requires->append((RexxObject *)new RequiresDirective(name, this->clause));
}


/**
 * Process a ::REQUIRES name LIBRARY directive.
 */
void LanguageParser::libraryDirective(RexxString *name, RexxToken *token)
{
    // we have an extra token on a ::REQUIRES directive.  The only thing accepted here
    // is the token LIBRARY.
    if (!token->isSymbol())
    {
        syntaxError(Error_Invalid_subkeyword_requires, token);
    }
                                   /* process each sub keyword          */
    if (subDirective(token) != SUBDIRECTIVE_LIBRARY)
    {
        syntaxError(Error_Invalid_subkeyword_requires, token);
    }
    token = nextReal();              /* get the next token                */
    if (!token->isEndOfClause()) /* something appear after this?      */
    {
        // nothing else allowed after this
        syntaxError(Error_Invalid_subkeyword_requires, token);
    }
    this->flags |= _install;         /* have information to install       */
    // add this to the library list
    this->libraries->append((RexxObject *)new LibraryDirective(name, this->clause));
}


void LanguageParser::directive()
/********************************************************************/
/* Function:  parse a directive statement                           */
/********************************************************************/
{
    RexxToken    *token;                 /* current token under processing    */

    this->nextClause();                  /* get the directive clause          */
    if (this->flags&no_clause)           /* reached the end?                  */
        return;                          /* all finished                      */
    token = nextReal();                  /* skip the leading ::               */
    if (token->classId != TOKEN_DCOLON)  /* reached the end of a block?       */
                                         /* have an error here                */
        syntaxError(Error_Translation_bad_directive);
    token = nextReal();                  /* get the keyword token             */
    if (!token->isSymbol())  /* not a symbol?                     */
                                         /* have an error here                */
        syntaxError(Error_Symbol_expected_directive);

    switch (this->keyDirective(token))
    { /* match against the directive list  */

        case DIRECTIVE_CLASS:              /* ::CLASS directive                 */
            classDirective();
            break;

        case DIRECTIVE_METHOD:             /* ::METHOD directive                */
            methodDirective();
            break;

        case DIRECTIVE_ROUTINE:            /* ::ROUTINE directive               */
            routineDirective();
            break;

        case DIRECTIVE_REQUIRES:           /* ::REQUIRES directive              */
            requiresDirective();
            break;

        case DIRECTIVE_ATTRIBUTE:          /* ::ATTRIBUTE directive             */
            attributeDirective();
            break;

        case DIRECTIVE_CONSTANT:           /* ::CONSTANT directive              */
            constantDirective();
            break;

        case DIRECTIVE_OPTIONS:            /* ::OPTIONS directive               */
            optionsDirective();
            break;

        default:                           /* unknown directive                 */
            syntaxError(Error_Translation_bad_directive);
            break;
    }
}
