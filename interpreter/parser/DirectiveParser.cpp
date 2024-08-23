/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2024 Rexx Language Association. All rights reserved.    */
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
/*                                                                            */
/* Parsing methods for processing directive instructions.                     */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "RexxActivation.hpp"
#include "LanguageParser.hpp"
#include "DirectoryClass.hpp"
#include "ClassDirective.hpp"
#include "RequiresDirective.hpp"
#include "LibraryDirective.hpp"
#include "ConstantDirective.hpp"
#include "MethodClass.hpp"
#include "RoutineClass.hpp"
#include "CPPCode.hpp"
#include "BaseCode.hpp"
#include "PackageManager.hpp"
#include "TraceSetting.hpp"


/**
 * Parse a directive instruction.
 */
void LanguageParser::nextDirective()
{
    // we are in a context where everthing we check is expected to be a
    // directive.

    // step to the next clause
    nextClause();

    // if we've hit the end, we're done.
    if (noClauseAvailable())
    {
        return;
    }

    RexxToken *token = nextReal();
    // we expect anything found here to be the start of a directive
    if (!token->isType(TOKEN_DCOLON))
    {
        syntaxError(Error_Translation_bad_directive);
    }

    // and this needs to be followed by a symbol keyword
    token = nextReal();
    if (!token->isSymbol())
    {
        syntaxError(Error_Symbol_expected_directive);
    }

    // and process each of the named directives
    switch (token->keyDirective())
    {
        // define a class
        case DIRECTIVE_CLASS:
            classDirective();
            break;

        // create a method
        case DIRECTIVE_METHOD:
            methodDirective();
            break;

        // create a routine
        case DIRECTIVE_ROUTINE:
            routineDirective();
            break;

        // require a package file or library
        case DIRECTIVE_REQUIRES:
            requiresDirective();
            break;

        // define an attribute
        case DIRECTIVE_ATTRIBUTE:
            attributeDirective();
            break;

        // define a constant
        case DIRECTIVE_CONSTANT:
            constantDirective();
            break;

        // define package-wide options
        case DIRECTIVE_OPTIONS:
            optionsDirective();
            break;

        // create package named resources
        case DIRECTIVE_RESOURCE:
            resourceDirective();
            break;

        // create package annotations
        case DIRECTIVE_ANNOTATE:
            annotateDirective();
            break;

        // an unknown directive
        default:
            syntaxError(Error_Translation_bad_directive);
            break;
    }
}


/**
 * Verify that no code follows a directive except for more
 * directive instructions.
 *
 * @param errorCode The error code to issue if this condition was violated.
 */
void LanguageParser::checkDirective(RexxErrorCodes errorCode)
{
    // save the clause location so we can reset for errors
    SourceLocation location = clauseLocation;

    // step to the next clause...if there is one, it must
    // be a directive.
    nextClause();
    if (clauseAvailable())
    {
        // The first token of the next instruction must be a ::
        RexxToken *token = nextReal();

        if (!token->isType(TOKEN_DCOLON))
        {
            syntaxError(errorCode);
        }
        // back up and push the clause back
        firstToken();
        reclaimClause();
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
    nextClause();
    if (clauseAvailable())
    {
        // we have a clause, now check if this is a directive or not
        RexxToken *token = nextReal();
        // not a "::", not a directive, which means we have real code to deal with
        result = !token->isType(TOKEN_DCOLON);
        // reset this clause entirely so we can start parsing for real.
        firstToken();
        reclaimClause();
    }
    return result;
}


/**
 * Test if a class directive is defining a duplicate class.
 *
 * @param name   The name from the ::class directive.
 *
 * @return true if a class with this name has already been encounterd.
 */
bool LanguageParser::isDuplicateClass(RexxString *name)
{
    return classDependencies->hasEntry(name);
}


/**
 * Retrieve a currently processed class directive.
 *
 * @param name   The name from the ::class directive.
 *
 * @return The class directive object or OREF_NULL if the named class is not found.
 */
ClassDirective *LanguageParser::findClassDirective(RexxString *name)
{
    return (ClassDirective *)classDependencies->entry(name);
}


/**
 * Test if a routine directive is defining a duplicate routine.
 *
 * @param name   The name from the ::routine directive.
 *
 * @return true if routine with this name has already been
 *         encounterd.
 */
bool LanguageParser::isDuplicateRoutine(RexxString *name)
{
    return routines->hasEntry(name);
}


/**
 * Test if a routine directive is defining a duplicate routine.
 *
 * @param name   The name from the ::routine directive.
 *
 * @return true if routine with this name has already been
 *         encounterd.
 */
RoutineClass *LanguageParser::findRoutine(RexxString *name)
{
    return (RoutineClass *)routines->entry(name);
}


/**
 * Add a class directive to the definition set.
 *
 * @param name      The new class directive.
 * @param directive
 */
void LanguageParser::addClassDirective(RexxString *name, ClassDirective *directive)
{
    classDependencies->put(directive, name);
    // also add to the array list
    classes->append(directive);
}


/**
 * Parse a class reference for a ::CLASS directive.  This
 * could be a subclass, mixinclass, metaclass, or inherits
 * class name.
 *
 * @param error  The error to raise for nothing there.
 *
 * @return A ClassResolver to look up the class reference.
 */
ClassResolver *LanguageParser::parseClassReference(RexxErrorCodes error)
{
    // this is a required string or symbol value
    RexxToken *token = nextReal();
    if (!token->isSymbolOrLiteral())
    {
        syntaxError(error, token);
    }

    // the class specification can be a literal, a symbol, or a
    // symbol:symbol namespace specification.  If this is a string,
    // we're done
    if (token->isLiteral())
    {
        return new ClassResolver(OREF_NULL, commonString(token->upperValue()));
    }
    else
    {
        // this is a symbol form.  Get the name, which might be either
        // a namespace name or the class name.  We need to parse a little
        // further to determine.
        RexxString *name = token->value();
        token = nextReal();

        // if this is not a colon, put it back and return the single
        // name form of resolver
        if (!token->isType(TOKEN_COLON))
        {
            previousToken();
            return new ClassResolver(OREF_NULL, name);
        }

        // ok, this should be of the form symbol:symbol.
        token = nextReal();
        if (!token->isSymbol())
        {
            syntaxError(Error_Symbol_expected_namespace_class);
        }
        // this is a qualified class reference
        return new ClassResolver(name, token->value());
    }
}


/**
 * Process a ::CLASS directive for a source file.
 */
void LanguageParser::classDirective()
{
    // first token is the name, which must be a symbol or string name
    RexxToken *token = nextReal();
    if (!token->isSymbolOrLiteral())
    {
        syntaxError(Error_Symbol_or_string_class);
    }

    // get the class name
    RexxString *name = token->value();
    // and we export this name in uppercase

    RexxString *public_name = commonString(name->upper());
    // check for a duplicate class
    if (isDuplicateClass(public_name))
    {
        syntaxError(Error_Translation_duplicate_class);
    }

    // create a class directive and add this to the dependency list
    activeClass = new ClassDirective(name, public_name, clause);

    // if we have any computed constants associated with this class, we need to
    // keep some special information about the constant expressions.

    // a table of variables...starting with the special variables we allocated space for.
    // we use the initial "clean" version, then save it after each parse constant associated
    // with the class.
    constantVariables = OREF_NULL;
    // also start with the default index counter
    constantVariableIndex = RexxLocalVariables::FIRST_VARIABLE_INDEX;
    // and also the largest stack needed to evaluate these.
    constantMaxStack = 0;
    // add this to our directives list.
    addClassDirective(public_name, activeClass);

    // we're using default scope.
    AccessFlag accessFlag = DEFAULT_ACCESS_SCOPE;

    // now we have a bunch of option keywords to handle, which can
    // only be specified once each.
    for (;;)
    {
        // real simple class definition.
        token = nextReal();
        if (token->isEndOfClause())
        {
            break;
        }
        // all options are symbols
        else if (!token->isSymbol())
        {
            syntaxError(Error_Invalid_subkeyword_class, token);
        }
        else
        {
            // directive sub keywords are also table based
            DirectiveSubKeyword type = token->subDirective();
            switch (type)
            {
                // ::CLASS name METACLASS metaclass
                case SUBDIRECTIVE_METACLASS:
                    // can't be a duplicate keyword
                    if (activeClass->getMetaClass() != OREF_NULL)
                    {
                        syntaxError(Error_Invalid_subkeyword_class, token);
                    }

                    // parse off the class name and set in the directive
                    activeClass->setMetaClass(parseClassReference(Error_Symbol_or_string_metaclass));
                    break;

                // ::CLASS name PUBLIC
                case SUBDIRECTIVE_PUBLIC:
                    // have we already seen an ACCESS flag?  This is an error
                    if (accessFlag != DEFAULT_ACCESS_SCOPE)
                    {
                        syntaxError(Error_Invalid_subkeyword_class, token);
                    }
                    accessFlag = PUBLIC_SCOPE;
                    // set the access in the active class.
                    activeClass->setPublic();
                    break;

                // ::CLASS name PRIVATE
                case SUBDIRECTIVE_PRIVATE:
                    if (accessFlag != DEFAULT_ACCESS_SCOPE)
                    {
                        syntaxError(Error_Invalid_subkeyword_class, token);
                    }
                    accessFlag = PRIVATE_SCOPE;

                    // don't need to set anything in the directive...this is the default
                    break;

                // ::CLASS name SUBCLASS sub
                case SUBDIRECTIVE_SUBCLASS:
                    // If we have a subclass set already, this is an error
                    if (activeClass->getSubClass() != OREF_NULL)
                    {
                        syntaxError(Error_Invalid_subkeyword_class, token);
                    }

                    // parse off the class name and set in the directive
                    activeClass->setSubClass(parseClassReference(Error_Symbol_or_string_subclass));
                    break;

                // ::CLASS name MIXINCLASS mclass
                case SUBDIRECTIVE_MIXINCLASS:
                    // If we have a subclass set already, this is an error
                    // NOTE:  setting a mixin class also defines the subclass...
                    if (activeClass->getSubClass() != OREF_NULL)
                    {
                        syntaxError(Error_Invalid_subkeyword_class, token);
                    }

                    // parse off the class name and set in the directive
                    activeClass->setMixinClass(parseClassReference(Error_Symbol_or_string_mixinclass));
                    break;

                // ::CLASS name INHERIT classes
                case SUBDIRECTIVE_INHERIT:
                    // all tokens after the keyword will be consumed by the INHERIT keyword.
                    token = nextReal();
                    if (token->isEndOfClause())
                    {
                        syntaxError(Error_Symbol_or_string_inherit, token);
                    }

                    while (!token->isEndOfClause())
                    {
                        // backup for the parsing
                        previousToken();

                        activeClass->addInherits(parseClassReference(Error_Symbol_or_string_inherit));
                        token = nextReal();
                    }
                    // step back a token for final completion checks
                    previousToken();
                    break;

                // ::CLASS name ABSTRACT
                case SUBDIRECTIVE_ABSTRACT:
                    // already been specified?  this is an error
                    if (activeClass->isAbstract())
                    {
                        syntaxError(Error_Invalid_subkeyword_class, token);
                    }

                    // mark this as abstract
                    activeClass->setAbstract();
                    break;

                // invalid keyword
                default:
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
void LanguageParser::checkDuplicateMethod(RexxString *name, bool classMethod, RexxErrorCodes errorMsg)
{
    // no previous ::CLASS directive?
    if (activeClass == OREF_NULL)
    {
        // cannot create unattached class methods.
        if (classMethod)
        {
            syntaxError(Error_Translation_missing_class);
        }
        // duplicate method name?
        if (unattachedMethods->entry(name) != OREF_NULL)
        {
            syntaxError(errorMsg);
        }
    }
    else
    {                                /* add the method to the active class*/
        // adding the method to the active class
        if (activeClass->checkDuplicateMethod(name, classMethod))
        {
            syntaxError(errorMsg);
        }
    }
}


/**
 * Locate the most recently created method with a given name in
 * this context.
 *
 * @param name The name to check.
 */
MethodClass *LanguageParser::findMethod(RexxString *name)
{
    // no previous ::CLASS directive?  Check the unattached methods
    if (activeClass == OREF_NULL)
    {
        return (MethodClass *)unattachedMethods->entry(name);
    }
    // we have an active class...get the method from there.  This will
    // check first for instance methods, then class
    else
    {
        // adding the method to the active class
        return (MethodClass *)activeClass->findMethod(name);
    }
}


/**
 * Locate the most recently created instance method with a given
 * name in this context.
 *
 * @param name The name to check.
 */
MethodClass *LanguageParser::findInstanceMethod(RexxString *name)
{
    // no previous ::CLASS directive?  Check the unattached methods
    if (activeClass == OREF_NULL)
    {
        return (MethodClass *)unattachedMethods->entry(name);
    }
    // we have an active class...get the method from there.  This will
    // check first for instance methods, then class
    else
    {
        // adding the method to the active class
        return (MethodClass *)activeClass->findInstanceMethod(name);
    }
}


/**
 * Locate the most recently created class method with a given
 * name in this context.
 *
 * @param name The name to check.
 */
MethodClass *LanguageParser::findClassMethod(RexxString *name)
{
    // no previous ::CLASS directive?  can't have a class method
    if (activeClass == OREF_NULL)
    {
        return OREF_NULL;
    }
    // we have an active class...get the method from there.
    else
    {
        // adding the method to the active class
        return (MethodClass *)activeClass->findClassMethod(name);
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
void LanguageParser::addMethod(RexxString *name, MethodClass *method, bool classMethod)
{
    // make sure this is attached to the source object for context information
    method->setPackageObject(package);
    // if no active class yet, these are unattached methods.
    if (activeClass == OREF_NULL)
    {
        unattachedMethods->setEntry(name, method);
    }
    else
    {
        activeClass->addMethod(name, method, classMethod);
    }
}


/**
 * Process a ::METHOD directive in a source file.
 */
void LanguageParser::methodDirective()
{
    // set default modifiers
    AccessFlag accessFlag = DEFAULT_ACCESS_SCOPE;
    ProtectedFlag protectedFlag = DEFAULT_PROTECTION;
    GuardFlag guardFlag = DEFAULT_GUARD;
    // other attributes of methods
    bool isClass = false;
    bool isAttribute = false;
    bool isAbstract = false;
    RexxString *externalname = OREF_NULL;       // not an external method yet
    RexxString *delegateName = OREF_NULL;       // no delegate target yet

    // method name must be a symbol or string
    RexxToken *token = nextReal();
    if (!token->isSymbolOrLiteral())
    {
        syntaxError(Error_Symbol_or_string_method, token);
    }

    // this is the method internal name
    RexxString *name = token->value();
    // this is the look up name
    RexxString *internalname = commonString(name->upper());

    // now process any additional option keywords
    for (;;)
    {
        // finished on EOC
        token = nextReal();
        if (token->isEndOfClause())
        {
            break;
        }
        // option keywords must be symbols
        else if (!token->isSymbol())
        {
            syntaxError(Error_Invalid_subkeyword_method, token);
        }
        else
        {
            // potential option keyword
            switch (token->subDirective())
            {
                // ::METHOD name CLASS
                case SUBDIRECTIVE_CLASS:
                    // no dups
                    if (isClass)
                    {
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }
                    isClass = true;
                    break;

                // ::METHOD name EXTERNAL extname
                case SUBDIRECTIVE_EXTERNAL:
                    // no dup on external and abstract is mutually exclusive
                    if (externalname != OREF_NULL || delegateName != OREF_NULL || isAbstract)
                    {
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }
                    token = nextReal();
                    // external name must be a string value
                    if (!token->isLiteral())
                    {
                        syntaxError(Error_Symbol_or_string_external, token);
                    }
                    // this will be parsed at install time
                    externalname = token->value();
                    break;

                // ::METHOD name PRIVATE
                case SUBDIRECTIVE_PRIVATE:
                    // has an access flag already been specified?
                    if (accessFlag != DEFAULT_ACCESS_SCOPE)
                    {
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }
                    accessFlag = PRIVATE_SCOPE;
                    break;

                // ::METHOD name PACKAGE
                case SUBDIRECTIVE_PACKAGE:
                    // has an access flag already been specified?
                    if (accessFlag != DEFAULT_ACCESS_SCOPE)
                    {
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }
                    accessFlag = PACKAGE_SCOPE;
                    break;

                // ::METHOD name PUBLIC
                case SUBDIRECTIVE_PUBLIC:
                    // has an access flag already been specified?
                    if (accessFlag != DEFAULT_ACCESS_SCOPE)
                    {
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }

                    accessFlag = PUBLIC_SCOPE;
                    break;

                // ::METHOD name PROTECTED
                case SUBDIRECTIVE_PROTECTED:
                    // had a protection flag specified already?
                    if (protectedFlag != DEFAULT_PROTECTION)
                    {
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }
                    protectedFlag = PROTECTED_METHOD;
                    break;

                // ::METHOD name UNPROTECTED
                case SUBDIRECTIVE_UNPROTECTED:
                    // had a protection flag specified already?
                    if (protectedFlag != DEFAULT_PROTECTION)
                    {
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }
                    protectedFlag = UNPROTECTED_METHOD;
                    break;

                // ::METHOD name UNGUARDED
                case SUBDIRECTIVE_UNGUARDED:
                    // already had a guard specification?
                    if (guardFlag != DEFAULT_GUARD)
                    {
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }

                    guardFlag = UNGUARDED_METHOD;
                    break;

                // ::METHOD name GUARDED
                case SUBDIRECTIVE_GUARDED:
                    // already had a guard specification?
                    if (guardFlag != DEFAULT_GUARD)
                    {
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }

                    guardFlag = GUARDED_METHOD;
                    break;

                // ::METHOD name ATTRIBUTE
                case SUBDIRECTIVE_ATTRIBUTE:
                    // check for duplicates
                    if (isAttribute)
                    {
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }

                    isAttribute = true;
                    break;

                // ::METHOD name ABSTRACT
                case SUBDIRECTIVE_ABSTRACT:
                    // can't have dups or external name or be a delegate
                    if (isAbstract || externalname != OREF_NULL || delegateName != OREF_NULL)
                    {
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }

                    isAbstract = true;
                    break;

                // ::METHOD name DELEGATE property
                case SUBDIRECTIVE_DELEGATE:
                    // no dup on external and abstract is mutually exclusive
                    if (externalname != OREF_NULL || delegateName != OREF_NULL || isAbstract)
                    {
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }

                    token = nextReal();
                    // delegate name must be a symbol
                    if (!token->isSymbol())
                    {
                        syntaxError(Error_Symbol_expected_delegate, token);
                    }
                    // get the name of the attribute used for the forwarding
                    delegateName = token->value();
                    break;

                // something invalid
                default:
                    syntaxError(Error_Invalid_subkeyword_method, token);
                    break;
            }
        }
    }

    // go check for a duplicate and validate the use of the CLASS modifier
    checkDuplicateMethod(internalname, isClass, Error_Translation_duplicate_method);

    Protected<MethodClass> _method;

    // handle delegates first.  A delegate method can also be an attribute, which really
    // just means we produce two delegate methods
    if (delegateName != OREF_NULL)
    {
        // now get a variable retriever to get the property
        RexxVariableBase *retriever = getRetriever(delegateName);

        // cannot have code following an method with the delegate keyword
        checkDirective(Error_Translation_delegate_method);

        if (isAttribute)
        {
            // now get this as the setter method.
            RexxString *setterName = commonString(internalname->concatWithCstring("="));
            // need to check for duplicates on that too
            checkDuplicateMethod(setterName, isClass, Error_Translation_duplicate_method);
            // create the method pair and quit.
            createDelegateMethod(setterName, retriever, isClass, accessFlag, protectedFlag, guardFlag, true);
        }
        // create the method pair and quit.
        createDelegateMethod(internalname, retriever, isClass, accessFlag, protectedFlag, guardFlag, isAttribute);
        return;
    }
    // is this an attribute method?
    else if (isAttribute)
    {
        // now get this as the setter method.
        RexxString *setterName = commonString(internalname->concatWithCstring("="));
        // need to check for duplicates on that too
        checkDuplicateMethod(setterName, isClass, Error_Translation_duplicate_method);

        // cannot have code following an method with the attribute keyword
        checkDirective(Error_Translation_attribute_method);
        // this might be externally defined setters and getters.
        if (externalname != OREF_NULL)
        {
            RexxString *l = OREF_NULL;
            RexxString *p = OREF_NULL;
            decodeExternalMethod(internalname, externalname, l, p);
            Protected<RexxString> library = l;
            Protected<RexxString> procedure = p;
            Protected<RexxString> getName = procedure->concatToCstring("GET");
            Protected<RexxString> setName = procedure->concatToCstring("SET");
            // now create both getter and setting methods from the information.
            _method = createNativeMethod(internalname, library, getName);
            _method->setAttributes(accessFlag, protectedFlag, guardFlag);
            // mark this as an attribute method
            _method->setAttribute();
            // add to the compilation
            addMethod(internalname, _method, isClass);

            _method = createNativeMethod(setterName, library, setName);
            _method->setAttributes(accessFlag, protectedFlag, guardFlag);
            // add to the compilation
            addMethod(setterName, _method, isClass);
        }
        // an abstract method as an attribute
        else if (isAbstract)
        {
            // create the method pair and quit.
            createAbstractMethod(internalname, isClass, accessFlag, protectedFlag, guardFlag, true);
            createAbstractMethod(setterName, isClass, accessFlag, protectedFlag, guardFlag, true);
        }
        else
        {
            // now get a variable retriever to get the property
            RexxVariableBase *retriever = getRetriever(name);

            // create the method pair and quit.
            createAttributeGetterMethod(internalname, retriever, isClass, accessFlag, protectedFlag, guardFlag);
            createAttributeSetterMethod(setterName, retriever, isClass, accessFlag, protectedFlag, guardFlag);
        }
        return;
    }
    // abstract method?
    else if (isAbstract)
    {
        // check that there is no code following this method.
        checkDirective(Error_Translation_abstract_method);
        // this uses a special code block
        Protected<BaseCode> code = new AbstractCode();
        _method = new MethodClass(name, code);
        // make sure the method is marked abstract
        _method->setAbstract();
    }
    // regular Rexx code method?
    else if (externalname == OREF_NULL)
    {
        // NOTE:  It is necessary to translate the block and protect the code
        // before allocating the MethodClass object.  The new operator allocates the
        // the object first, then evaluates the constructor arguments after the allocation.
        // Since the translateBlock() call will allocate a lot of new objects before returning,
        // there's a high probability that the method object can get garbage collected before
        // there is any opportunity to protect the object.
        Protected<RexxCode> code = translateBlock();

        // go do the next block of code
        _method = new MethodClass(name, code);
    }
    // external method
    else
    {
        RexxString *l = OREF_NULL;
        RexxString *p = OREF_NULL;
        decodeExternalMethod(internalname, externalname, l, p);
        Protected<RexxString> library = l;
        Protected<RexxString> procedure = p;

        // check that there this is only followed by other directives.
        checkDirective(Error_Translation_external_method);
        // and make this into a method object.
        _method = createNativeMethod(name, library, procedure);
    }
    _method->setAttributes(accessFlag, protectedFlag, guardFlag);
    // add to the compilation
    addMethod(internalname, _method, isClass);
}


/**
 * Process a ::OPTIONS directive in a source file.
 */
void LanguageParser::optionsDirective()
{
    // except for (NO)PROLOG all options are of a keyword/value pattern
    for (;;)
    {

        RexxToken *token = nextReal();
        // finish up if we've hit EOC
        if (token->isEndOfClause())
        {
            break;
        }
        // non-symbol is an error
        else if (!token->isSymbol())
        {
            syntaxError(Error_Invalid_subkeyword_options, token);
        }
        else
        {
            // potential options keyword
            switch (token->subDirective())
            {
                // ::OPTIONS DIGITS nnnn
                case SUBDIRECTIVE_DIGITS:
                {
                    token = nextReal();
                    // we'll accept this as a symbol or a string...as long as it's a number
                    if (!token->isSymbolOrLiteral())
                    {
                        syntaxError(Error_Symbol_or_string_digits_value, token);
                    }

                    size_t digits;

                    // convert to a binary number using either 9 or 18 digits
                    if (!token->value()->requestUnsignedNumber(digits, Numerics::ARGUMENT_DIGITS) || digits < 1)
                    {
                        syntaxError(Error_Invalid_whole_number_digits, token->value());
                    }
                    // problem with the fuzz setting?
                    if ((wholenumber_t)digits <= package->getFuzz())
                    {
                        reportException(Error_Expression_result_digits, digits, package->getFuzz());
                    }
                    // set this in the package object
                    package->setDigits(digits);
                    break;
                }

                // ::OPTIONS FORM ENGINEERING/SCIENTIFIC
                case SUBDIRECTIVE_FORM:
                {
                    token = nextReal();
                    if (!token->isSymbol())
                    {
                        syntaxError(Error_Symbol_expected_form, token);
                    }

                    switch (token->subKeyword())
                    {
                        // FORM SCIENTIFIC
                        case SUBKEY_SCIENTIFIC:
                            package->setForm(Numerics::FORM_SCIENTIFIC);
                            break;

                        // FORM ENGINEERING
                        case SUBKEY_ENGINEERING:     /* NUMERIC FORM ENGINEERING          */
                            package->setForm(Numerics::FORM_ENGINEERING);
                            break;

                        // bad keyword
                        default:
                            syntaxError(Error_Invalid_subkeyword_form, token);
                            break;
                    }
                    break;
                }

                // ::OPTIONS FUZZ nnnn
                case SUBDIRECTIVE_FUZZ:
                {
                    token = nextReal();
                    if (!token->isSymbolOrLiteral())
                    {
                        syntaxError(Error_Symbol_or_string_fuzz_value, token);
                    }

                    RexxString *value = token->value();          /* get the string value              */

                    size_t fuzz;

                    // convert to a binary number using either 9 or 18 digits
                    if (!value->requestUnsignedNumber(fuzz, Numerics::ARGUMENT_DIGITS))
                    {
                        syntaxError(Error_Invalid_whole_number_fuzz, value);
                    }
                    // validate with the digits setting
                    if ((wholenumber_t)fuzz >= package->getDigits())
                    {
                        reportException(Error_Expression_result_digits, package->getDigits(), fuzz);
                    }
                    package->setFuzz(fuzz);
                    break;
                }

                // ::OPTIONS TRACE setting
                case SUBDIRECTIVE_TRACE:
                {
                    token = nextReal();
                    if (!token->isSymbolOrLiteral())
                    {
                        syntaxError(Error_Symbol_or_string_trace_value, token);
                    }

                    RexxString *value = token->value();
                    char badOption = 0;
                    TraceSetting settings;

                    // validate the setting
                    if (!settings.parseTraceSetting(value, badOption))
                    {
                        syntaxError(Error_Invalid_trace_trace, new_string(&badOption, 1));
                    }
                    // poke into the package
                    package->setTraceSetting(settings);
                    break;
                }

                // ::OPTIONS NOVALUE
                case SUBDIRECTIVE_NOVALUE:
                {
                    token = nextReal();
                    if (!token->isSymbol())
                    {
                        syntaxError(Error_Symbol_expected_after_keyword, GlobalNames::NOVALUE);
                    }

                    switch (token->subDirective())
                    {
                        case SUBDIRECTIVE_ERROR: // backwards compatibility for NOVALUE
                        case SUBDIRECTIVE_SYNTAX:
                        {
                            package->enableNovalueSyntax();
                            break;
                        }

                        case SUBDIRECTIVE_CONDITION:
                        {
                            package->disableNovalueSyntax();
                            break;
                        }

                        default:
                            syntaxError(Error_Invalid_subkeyword_following, GlobalNames::NOVALUE, token->value());
                    }
                    break;
                }

                // ::OPTIONS ERROR
                case SUBDIRECTIVE_ERROR:
                {
                    token = nextReal();
                    if (!token->isSymbol())
                    {
                        syntaxError(Error_Symbol_expected_after_keyword, GlobalNames::ERRORNAME);
                    }

                    switch (token->subDirective())
                    {
                        case SUBDIRECTIVE_SYNTAX:
                        {
                            package->enableErrorSyntax();
                            break;
                        }

                        case SUBDIRECTIVE_CONDITION:
                        {
                            package->disableErrorSyntax();
                            break;
                        }

                        default:
                            syntaxError(Error_Invalid_subkeyword_following, GlobalNames::ERRORNAME, token->value());
                    }
                    break;
                }

                // ::OPTIONS FAILURE
                case SUBDIRECTIVE_FAILURE:
                {
                    token = nextReal();
                    if (!token->isSymbol())
                    {
                        syntaxError(Error_Symbol_expected_after_keyword, GlobalNames::FAILURE);
                    }

                    switch (token->subDirective())
                    {
                        case SUBDIRECTIVE_SYNTAX:
                        {

                            package->enableFailureSyntax();
                            break;
                        }

                        case SUBDIRECTIVE_CONDITION:
                        {
                            package->disableFailureSyntax();
                            break;
                        }

                        default:
                            syntaxError(Error_Invalid_subkeyword_following, GlobalNames::FAILURE, token->value());
                    }
                    break;
                }

                // ::OPTIONS LOSTDIGITS
                case SUBDIRECTIVE_LOSTDIGITS:
                {
                    token = nextReal();
                    if (!token->isSymbol())
                    {
                        syntaxError(Error_Symbol_expected_after_keyword, GlobalNames::LOSTDIGITS);
                    }

                    switch (token->subDirective())
                    {
                        case SUBDIRECTIVE_SYNTAX:
                        {

                            package->enableLostdigitsSyntax();
                            break;
                        }

                        case SUBDIRECTIVE_CONDITION:
                        {
                            package->disableLostdigitsSyntax();
                            break;
                        }

                        default:
                            syntaxError(Error_Invalid_subkeyword_following, GlobalNames::LOSTDIGITS, token->value());
                    }
                    break;
                }

                // ::OPTIONS NOSTRING
                case SUBDIRECTIVE_NOSTRING:
                {
                    token = nextReal();
                    if (!token->isSymbol())
                    {
                        syntaxError(Error_Symbol_expected_after_keyword, GlobalNames::NOSTRING);
                    }

                    switch (token->subDirective())
                    {
                        case SUBDIRECTIVE_SYNTAX:
                        {

                            package->enableNostringSyntax();
                            break;
                        }

                        case SUBDIRECTIVE_CONDITION:
                        {
                            package->disableNostringSyntax();
                            break;
                        }

                        default:
                            syntaxError(Error_Invalid_subkeyword_following, GlobalNames::NOSTRING, token->value());
                    }
                    break;
                }

                // ::OPTIONS NOTREADY
                case SUBDIRECTIVE_NOTREADY:
                {
                    token = nextReal();
                    if (!token->isSymbol())
                    {
                        syntaxError(Error_Symbol_expected_after_keyword, GlobalNames::NOTREADY);
                    }

                    switch (token->subDirective())
                    {
                        case SUBDIRECTIVE_SYNTAX:
                        {

                            package->enableNotreadySyntax();
                            break;
                        }

                        case SUBDIRECTIVE_CONDITION:
                        {
                            package->disableNotreadySyntax();
                            break;
                        }

                        default:
                            syntaxError(Error_Invalid_subkeyword_following, GlobalNames::NOTREADY, token->value());
                    }
                    break;
                }

                // ::OPTIONS ALL
                case SUBDIRECTIVE_ALL:
                {
                    token = nextReal();
                    if (!token->isSymbol())
                    {
                        syntaxError(Error_Symbol_expected_after_keyword, GlobalNames::ALL);
                    }

                    switch (token->subDirective())
                    {
                        case SUBDIRECTIVE_SYNTAX:
                        {

                            package->enableErrorSyntax();
                            package->enableFailureSyntax();
                            package->enableLostdigitsSyntax();
                            package->enableNostringSyntax();
                            package->enableNotreadySyntax();
                            package->enableNovalueSyntax();
                            break;
                        }

                        case SUBDIRECTIVE_CONDITION:
                        {
                            package->disableErrorSyntax();
                            package->disableFailureSyntax();
                            package->disableLostdigitsSyntax();
                            package->disableNostringSyntax();
                            package->disableNotreadySyntax();
                            package->disableNovalueSyntax();
                            break;
                        }

                        default:
                            syntaxError(Error_Invalid_subkeyword_following, GlobalNames::ALL, token->value());
                    }
                    break;
                }

                // ::OPTIONS NOPROLOG
                case SUBDIRECTIVE_NOPROLOG:
                {
                    // this option is just the keyword...flip off the prolog in the package
                    package->disableProlog();
                    break;
                }

                // ::OPTIONS PROLOG
                case SUBDIRECTIVE_PROLOG:
                {
                    // this option is just the keyword...flip on the prolog in the package
                    package->enableProlog();
                    break;
                }

                // invalid keyword
                default:
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
MethodClass *LanguageParser::createNativeMethod(RexxString *name, RexxString *library, RexxString *procedure)
{
    Protected<NativeCode> nmethod = PackageManager::resolveMethod(library, procedure);
    // raise an exception if this entry point is not found.
    if (nmethod == OREF_NULL)
    {
         syntaxError(Error_External_name_not_found_method, procedure);
    }
    // this might return a different object if this has been used already
    nmethod = (NativeCode *)nmethod->setPackageObject(package);
    // turn into a real method object
    return new MethodClass(name, nmethod);
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

    // convert external into array of words (this also adds all words
    // to the common string pool and uppercases the first word)
    Protected<ArrayClass> _words = words(externalSpec);
    if (_words->size() == 0)
    {
        syntaxError(Error_Translation_bad_external, externalSpec);
    }

    // not 'LIBRARY library [entry]' form?
    if (((RexxString *)(_words->get(1)))->strCompare("LIBRARY"))
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
            syntaxError(Error_Translation_bad_external, externalSpec);
        }
    }
    else
    {
        syntaxError(Error_Translation_bad_external, externalSpec);
    }
}


/**
 * Indicator of what type of attribute to generate.
 */
typedef enum
{
    ATTRIBUTE_BOTH,
    ATTRIBUTE_GET,
    ATTRIBUTE_SET,
} AttributeType;


/**
 * Process a ::ATTRIBUTE directive in a source file.
 */
void LanguageParser::attributeDirective()
{
    // set the default attributes
    AccessFlag accessFlag = DEFAULT_ACCESS_SCOPE;
    ProtectedFlag  protectedFlag = DEFAULT_PROTECTION;
    GuardFlag guardFlag = DEFAULT_GUARD;
    // by default, we create both methods for the attribute.
    AttributeType style = ATTRIBUTE_BOTH;
    bool isClass = false;            // default is an instance method
    bool isAbstract = false;         // by default, creating a concrete method
    RexxString *delegateName = OREF_NULL;

    RexxToken *token = nextReal();
    // the name must be a string or a symbol
    if (!token->isSymbolOrLiteral())
    {
        syntaxError(Error_Symbol_or_string_attribute, token);
    }

    // get the attribute name and the internal value that we create the method names with
    RexxString *name = token->value();
    RexxString *internalname = commonString(name->upper());
    RexxString *externalname = OREF_NULL;

    // process options
    for (;;)
    {
        // if last token we're out of here.
        token = nextReal();
        if (token->isEndOfClause())
        {
            break;
        }
        // all options must be symbols
        else if (!token->isSymbol())
        {
            syntaxError(Error_Invalid_subkeyword_attribute, token);
        }
        else
        {
            switch (token->subDirective())
            {
                // GET define this as a attribute get method
                case SUBDIRECTIVE_GET:
                    // only one per customer
                    if (style != ATTRIBUTE_BOTH)
                    {
                        syntaxError(Error_Invalid_subkeyword_attribute, token);
                    }
                    style = ATTRIBUTE_GET;
                    break;

                // SET create an attribute assignment method
                case SUBDIRECTIVE_SET:
                    // only one of GET/SET allowed
                    if (style != ATTRIBUTE_BOTH)
                    {
                        syntaxError(Error_Invalid_subkeyword_attribute, token);
                    }
                    style = ATTRIBUTE_SET;
                    break;


                // ::ATTRIBUTE name CLASS  creating class methods
                case SUBDIRECTIVE_CLASS:
                    // no dups allowed
                    if (isClass)
                    {
                        syntaxError(Error_Invalid_subkeyword_attribute, token);
                    }
                    isClass = true;
                    break;

                // private access?
                case SUBDIRECTIVE_PRIVATE:
                    // can have just one of PUBLIC or PRIVATE
                    if (accessFlag != DEFAULT_ACCESS_SCOPE)
                    {
                        syntaxError(Error_Invalid_subkeyword_attribute, token);
                    }
                    accessFlag = PRIVATE_SCOPE;
                    break;

                // define with public access (the default)
                case SUBDIRECTIVE_PUBLIC:
                    // must be first access specifier
                    if (accessFlag != DEFAULT_ACCESS_SCOPE)
                    {
                        syntaxError(Error_Invalid_subkeyword_attribute, token);
                    }
                    accessFlag = PUBLIC_SCOPE;
                    break;

                // define with package access
                case SUBDIRECTIVE_PACKAGE:
                    // must be first access specifier
                    if (accessFlag != DEFAULT_ACCESS_SCOPE)
                    {
                        syntaxError(Error_Invalid_subkeyword_attribute, token);
                    }
                    accessFlag = PACKAGE_SCOPE;
                    break;

                // ::METHOD name PROTECTED
                case SUBDIRECTIVE_PROTECTED:
                    // only one of PROTECTED UNPROTECTED
                    if (protectedFlag != DEFAULT_PROTECTION)
                    {
                        syntaxError(Error_Invalid_subkeyword_attribute, token);
                    }
                    protectedFlag = PROTECTED_METHOD;
                    break;

                // unprotected method (the default)
                case SUBDIRECTIVE_UNPROTECTED:
                    // only one of PROTECTED UNPROTECTED
                    if (protectedFlag != DEFAULT_PROTECTION)
                    {
                        syntaxError(Error_Invalid_subkeyword_attribute, token);
                    }
                    protectedFlag = UNPROTECTED_METHOD;
                    break;

                // unguarded access to an object
                case SUBDIRECTIVE_UNGUARDED:
                    if (guardFlag != DEFAULT_GUARD)
                    {
                        syntaxError(Error_Invalid_subkeyword_attribute, token);
                    }
                    guardFlag = UNGUARDED_METHOD;
                    break;

                // guarded access to a method (the default)
                case SUBDIRECTIVE_GUARDED:
                    if (guardFlag != DEFAULT_GUARD)
                    {
                        syntaxError(Error_Invalid_subkeyword_attribute, token);
                    }
                    guardFlag = GUARDED_METHOD;
                    break;

                // external attributes?
                case SUBDIRECTIVE_EXTERNAL:
                    // can't be abstract and external
                    if (externalname != OREF_NULL || delegateName != OREF_NULL || isAbstract)
                    {
                        syntaxError(Error_Invalid_subkeyword_attribute, token);
                    }

                    // the external specifier must be a string
                    token = nextReal();
                    if (!token->isLiteral())
                    {
                        syntaxError(Error_Symbol_or_string_external, token);
                    }

                    externalname = token->value();
                    break;

                // abstract method
                case SUBDIRECTIVE_ABSTRACT:
                    // abstract and external conflict
                    if (isAbstract || externalname != OREF_NULL || delegateName != OREF_NULL)
                    {
                        syntaxError(Error_Invalid_subkeyword_attribute, token);
                    }
                    isAbstract = true;
                    break;

                // ::ATTRIBUTE name DELEGATE target property
                case SUBDIRECTIVE_DELEGATE:
                    // no dup on external and abstract is mutually exclusive
                    if (externalname != OREF_NULL || delegateName != OREF_NULL || isAbstract)
                    {
                        syntaxError(Error_Invalid_subkeyword_attribute, token);
                    }

                    token = nextReal();
                    // delegate name must be a symbol
                    if (!token->isSymbol())
                    {
                        syntaxError(Error_Symbol_expected_delegate, token);
                    }
                    // get the name of the attribute used for the forwarding
                    delegateName = token->value();
                    break;

                // some invalid keyword
                default:
                    syntaxError(Error_Invalid_subkeyword_attribute, token);
                    break;
            }
        }
    }

    // both attributes same default properties?

    // now get a variable retriever to get the property (do this before checking the body
    // so errors get diagnosed on the correct line),
    RexxVariableBase *retriever = getRetriever(name);

    switch (style)
    {
        // creating both a setter and getter.  These cannot have
        // following bodies
        case ATTRIBUTE_BOTH:
        {
            checkDuplicateMethod(internalname, isClass, Error_Translation_duplicate_attribute);
            // now get this as the setter method.
            RexxString *setterName = commonString(internalname->concatWithCstring("="));
            checkDuplicateMethod(setterName, isClass, Error_Translation_duplicate_attribute);

            // no code can follow the automatically generated methods
            checkDirective(Error_Translation_body_error);
            if (externalname != OREF_NULL)
            {
                RexxString *l = OREF_NULL;
                RexxString *p = OREF_NULL;
                decodeExternalMethod(internalname, externalname, l, p);
                Protected<RexxString> library = l;
                Protected<RexxString> procedure = p;
                Protected<RexxString> getName = procedure->concatToCstring("GET");
                Protected<RexxString> setName = procedure->concatToCstring("SET");

                // now create both getter and setting methods from the information.
                Protected<MethodClass> _method = createNativeMethod(internalname, library, getName);
                _method->setAttributes(accessFlag, protectedFlag, guardFlag);
                // mark this as an attribute method
                _method->setAttribute();
                // add to the compilation
                addMethod(internalname, _method, isClass);

                _method = createNativeMethod(setterName, library, setName);
                _method->setAttributes(accessFlag, protectedFlag, guardFlag);
                // mark this as an attribute method
                _method->setAttribute();
                // add to the compilation
                addMethod(setterName, _method, isClass);
            }
            // abstract method?
            else if (isAbstract)
            {
                // create the method pair and quit.
                createAbstractMethod(internalname, isClass, accessFlag, protectedFlag, guardFlag, true);
                createAbstractMethod(setterName, isClass, accessFlag, protectedFlag, guardFlag, true);
            }
            // delegating these to another object
            else if (delegateName != OREF_NULL)
            {
                // the retriever is the delegate name, not the attribute name
                retriever = getRetriever(delegateName);
                // create the method pair and quit.
                createDelegateMethod(internalname, retriever, isClass, accessFlag, protectedFlag, guardFlag, true);
                // create the method pair and quit.
                createDelegateMethod(setterName, retriever, isClass, accessFlag, protectedFlag, guardFlag, true);
            }
            else
            {
                // create the method pair and quit.
                createAttributeGetterMethod(internalname, retriever, isClass, accessFlag, protectedFlag, guardFlag);
                createAttributeSetterMethod(setterName, retriever, isClass, accessFlag, protectedFlag, guardFlag);
            }
            break;

        }
        // just need a getter method
        case ATTRIBUTE_GET:
        {
            checkDuplicateMethod(internalname, isClass, Error_Translation_duplicate_attribute);
            // external?  resolve the method
            if (externalname != OREF_NULL)
            {
                // no code can follow external methods
                checkDirective(Error_Translation_external_attribute);
                RexxString *l = OREF_NULL;
                RexxString *p = OREF_NULL;
                decodeExternalMethod(internalname, externalname, l, p);
                Protected<RexxString> library = l;
                Protected<RexxString> procedure = p;
                // if there was no procedure explicitly given, create one using the GET/SET convention
                if (internalname == procedure)
                {
                    procedure = procedure->concatToCstring("GET");
                }
                // now create both getter and setting methods from the information.
                Protected<MethodClass> _method = createNativeMethod(internalname, library, procedure);
                _method->setAttributes(accessFlag, protectedFlag, guardFlag);
                // mark this as an attribute method
                _method->setAttribute();
                // add to the compilation
                addMethod(internalname, _method, isClass);
            }
            // abstract method?
            else if (isAbstract)
            {
                // no code can follow abstract methods
                checkDirective(Error_Translation_abstract_attribute);
                // create the method pair and quit.
                createAbstractMethod(internalname, isClass, accessFlag, protectedFlag, guardFlag, true);
            }
            // delegating these to another object
            else if (delegateName != OREF_NULL)
            {
                // the retriever is the delegate name, not the attribute name
                retriever = getRetriever(delegateName);

                // no code can follow delegate methods
                checkDirective(Error_Translation_delegate_attribute);

                // create the method pair and quit.
                createDelegateMethod(internalname, retriever, isClass, accessFlag, protectedFlag, guardFlag, true);
            }
            // either written in ooRexx or is automatically generated.
            else
            {
                // written in Rexx?  go create
                if (hasBody())
                {
                    createMethod(internalname, isClass, accessFlag, protectedFlag, guardFlag, true);
                }
                else
                {
                    createAttributeGetterMethod(internalname, retriever, isClass, accessFlag, protectedFlag, guardFlag);
                }
            }
            break;
        }

        // just a setter method
        case ATTRIBUTE_SET:
        {
            // now get this as the setter method.
            RexxString *setterName = commonString(internalname->concatWithCstring("="));
            checkDuplicateMethod(setterName, isClass, Error_Translation_duplicate_attribute);
            // external?  resolve the method
            if (externalname != OREF_NULL)
            {
                // no code can follow external methods
                checkDirective(Error_Translation_external_attribute);
                RexxString *l = OREF_NULL;
                RexxString *p = OREF_NULL;
                decodeExternalMethod(internalname, externalname, l, p);
                Protected<RexxString> library = l;
                Protected<RexxString> procedure = p;
                // if there was no procedure explicitly given, create one using the GET/SET convention
                if (internalname == procedure)
                {
                    procedure = procedure->concatToCstring("SET");
                }
                // now create both getter and setting methods from the information.
                Protected<MethodClass> _method = createNativeMethod(setterName, library, procedure);
                _method->setAttributes(accessFlag, protectedFlag, guardFlag);
                // mark this as an attribute method
                _method->setAttribute();
                // add to the compilation
                addMethod(setterName, _method, isClass);
            }
            // abstract method?
            else if (isAbstract)
            {
                // no code can follow abstract methods
                checkDirective(Error_Translation_abstract_attribute);
                // create the method pair and quit.
                createAbstractMethod(setterName, isClass, accessFlag, protectedFlag, guardFlag, true);
            }
            // delegating these to another object
            else if (delegateName != OREF_NULL)
            {
                // the retriever is the delegate name, not the attribute name
                retriever = getRetriever(delegateName);

                // no code can follow delegate methods
                checkDirective(Error_Translation_delegate_attribute);

                // create the method pair and quit.
                createDelegateMethod(setterName, retriever, isClass, accessFlag, protectedFlag, guardFlag, true);
            }
            else
            {
                if (hasBody())        // just the getter method
                {
                    createMethod(setterName, isClass, accessFlag, protectedFlag, guardFlag, true);
                }
                else
                {
                    createAttributeSetterMethod(setterName, retriever, isClass, accessFlag, protectedFlag, guardFlag);
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
    RexxToken *token = nextReal();

    if (!token->isSymbolOrLiteral())
    {
        syntaxError(Error_Symbol_or_string_constant, token);
    }

    // get the expressed name and the name we use for the methods
    RexxString *name = token->value();
    RexxString *internalname = commonString(name->upper());

    // we only expect just a single value token here
    token = nextReal();
    RexxObject *value = OREF_NULL;
    RexxInternalObject *expression = OREF_NULL;

    // the value omitted?  Just use the literal name of the constant.
    if (token->isEndOfClause())
    {
        value = name;
        // push the EOC token back
        previousToken();
    }
    // this could be an expression that needs to
    else if (token->isLeftParen())
    {
        // we have an expression in parens, parse it as a subexpression.
        expression = translateConstantExpression(token, Error_Invalid_expression_missing_constant);
    }
    // no a symbol or literal...we have special checks for signed numbers
    else if (!token->isSymbolOrLiteral())
    {
        // if not a "+" or "-" operator, this is an error
        if (!token->isOperator() || (!token->isSubtype(OPERATOR_SUBTRACT, OPERATOR_PLUS)))
        {
            syntaxError(Error_Symbol_or_string_constant_value, token);
        }
        RexxToken *second = nextReal();
        // this needs to be a constant symbol...we check for
        // numeric below
        if (!second->isSymbol() || !second->isSubtype(SYMBOL_CONSTANT))
        {
            syntaxError(Error_Symbol_or_string_constant_value, token);
        }
        // concat with the sign operator
        value = token->value()->concat(second->value());
        // and validate that this a valid number
        if (value->numberString() == OREF_NULL)
        {
            syntaxError(Error_Symbol_or_string_constant_value, token);
        }
    }
    else
    {
        // this will be some sort of literal value
        value = token->value();
    }

    // nothing more permitted after this
    requiredEndOfClause(Error_Invalid_data_constant_dir);

    // save the current location before checking for a body, since that
    // gets the next clause, which will update the location.
    SourceLocation directiveLocation = clause->getLocation();

    // this directive does not allow a body
    checkDirective(Error_Translation_constant_body);

    // check for duplicates.  We only do the class duplicate check if there
    // is an active class, otherwise we'll get a syntax error
    checkDuplicateMethod(internalname, false, Error_Translation_duplicate_constant);
    if (activeClass != OREF_NULL)
    {
        checkDuplicateMethod(internalname, true, Error_Translation_duplicate_constant);
    }

    // create the method pair and quit.
    createConstantGetterMethod(internalname, value, expression, directiveLocation);
}


/**
 * Process an ::ANNOTATE directive in a source file.
 */
void LanguageParser::annotateDirective()
{
    RexxToken *token = nextReal();

    // the annotation type must be a symbol
    if (!token->isSymbol())
    {
        syntaxError(Error_Symbol_expected_annotation_type, token);
    }

    // the type determines what table we add this to.
    StringTable *annotations;

    // now handle the different annotation types
    switch (token->subDirective())
    {
        // annotating the package
        case SUBDIRECTIVE_PACKAGE:
        {
            // get this directly from the package.  Note, annotations are accumulative.
            annotations = package->getAnnotations();
            break;
        }

        // annotating the current a previously defined class.
        case SUBDIRECTIVE_CLASS:
        {
            // we need a class name on this token
            token = nextReal();
            // this must be a literal string
            if (!token->isSymbolOrLiteral())
            {
                syntaxError(Error_Symbol_or_string_directive_option, GlobalNames::ANNOTATE_DIRECTIVE, GlobalNames::CLASS);
            }

            // get the target class name
            RexxString *name = commonString(token->upperValue());
            ClassDirective *directive = findClassDirective(name);

            // if the directive does not exist (or has not yet been defined)
            // raise the error now.
            if (directive == OREF_NULL)
            {
                syntaxError(Error_Translation_missing_annotation_target, "class", name);
            }

            // get this directly from the directive.  Note, annotations are accumulative.
            annotations = directive->getAnnotations();
            break;
        }

        // annotating a previously defined routine.
        case SUBDIRECTIVE_ROUTINE:
        {
            // we need a routine name on this token
            token = nextReal();
            // this must be a literal string
            if (!token->isSymbolOrLiteral())
            {
                syntaxError(Error_Symbol_or_string_directive_option, GlobalNames::ANNOTATE_DIRECTIVE, GlobalNames::ROUTINE);
            }

            // get the target name
            RexxString *name = commonString(token->upperValue());
            RoutineClass *routine = findRoutine(name);

            // if the directive does not exist (or has not yet been defined)
            // raise the error now.
            if (routine == OREF_NULL)
            {
                syntaxError(Error_Translation_missing_annotation_target, "routine", name);
            }

            // get this directly from the directive.  Note, annotations are accumulative.
            annotations = routine->getAnnotations();
            break;
        }

        // annotating a previously defined method.
        case SUBDIRECTIVE_METHOD:
        {
            // we need a method name on this token
            token = nextReal();
            // this must be a literal string
            if (!token->isSymbolOrLiteral())
            {
                syntaxError(Error_Symbol_or_string_directive_option, GlobalNames::ANNOTATE_DIRECTIVE, GlobalNames::METHOD);
            }

            // get the target method
            RexxString *name = commonString(token->upperValue());
            MethodClass *method = findMethod(name);

            // if the directive does not exist (or has not yet been defined)
            // raise the error now.
            if (method == OREF_NULL)
            {
                syntaxError(Error_Translation_missing_annotation_target, "method", name);
            }

            // get this directly from the directive.  Note, annotations are accumulative.
            annotations = method->getAnnotations();
            break;
        }

        // annotating previously defined attribute method(s).
        case SUBDIRECTIVE_ATTRIBUTE:
        {
            // we need a attribute name on this token
            token = nextReal();
            // this must be a literal string
            if (!token->isSymbolOrLiteral())
            {
                syntaxError(Error_Symbol_or_string_directive_option, GlobalNames::ANNOTATE_DIRECTIVE, GlobalNames::ATTRIBUTE);
            }

            // get the target attribute name
            RexxString *attributeName = commonString(token->upperValue());

            // process the annotations and return directly from here.
            processAttributeAnnotations(attributeName);
            return;
        }


        // annotating a previously constant method.
        case SUBDIRECTIVE_CONSTANT:
        {
            // we need a class name on this token
            token = nextReal();
            // this must be a literal string
            if (!token->isSymbolOrLiteral())
            {
                syntaxError(Error_Symbol_or_string_directive_option, GlobalNames::ANNOTATE_DIRECTIVE, GlobalNames::CONSTANT);
            }

            // get the target class name
            RexxString *name = commonString(token->upperValue());
            MethodClass *method = findMethod(name);

            // we need to enforce that this is a constant method
            if (method != OREF_NULL && !method->isConstant())
            {
                method = OREF_NULL;
            }

            // if the directive does not exist (or has not yet been defined)
            // raise the error now.
            if (method == OREF_NULL)
            {
                syntaxError(Error_Translation_missing_annotation_target, "constant", name);
            }

            // get this directly from the directive.  Note, annotations are accumulative.
            // for constant methods, we create a single method and attach it to both the
            // instance and class method dictionaries, so we only need to update the one method.
            annotations = method->getAnnotations();
            break;
        }

        // an unknown keyword
        default:
        {
            syntaxError(Error_Invalid_subkeyword_annotation, token);
            break;
        }
    }

    // now start on the attribute tokens
    token = nextReal();

    // an ::ANNOTATE directive need not specify anything, but if it
    // does, everything is in the form of "symbol constant" pairs.
    while (!token->isEndOfClause())
    {
        processAnnotation(token, annotations);
        token = nextReal();
    }
}


/**
 * Process potential attribute annotations.  This will
 * annotate both methods of an annotate pair (or the ones
 * that actually exist).  At least one of the annotation
 * methods must exist.  Both methods must be at the same
 * scope (i.e., class or instance).  Instance methods
 * are checked first, then class methods.
 *
 * @param getterName   The name of the attribute.
 */
void LanguageParser::processAttributeAnnotations(RexxString *getterName)
{
    // we need both a setter and getter method, set getter the setter name
    RexxString *setterName = commonString(getterName->concatWithCstring("="));

    // we check first for instance methods.  Any methods we find
    // must have been created using ::ATTRIBUTE or ::METHOD ATTRIBUTE.
    // we ignore any name matches that are not attribute methods
    MethodClass *getterMethod = findInstanceMethod(getterName);
    if (getterMethod != OREF_NULL && !getterMethod->isAttribute())
    {
        getterMethod = OREF_NULL;
    }

    MethodClass *setterMethod = findInstanceMethod(setterName);
    if (setterMethod != OREF_NULL && !setterMethod->isAttribute())
    {
        setterMethod = OREF_NULL;
    }

    // if we did not find any instance methods with the attribute name,
    // check for class methods for the current class
    if (getterMethod == OREF_NULL && setterMethod == OREF_NULL)
    {
        getterMethod = findClassMethod(getterName);
        if (getterMethod != OREF_NULL && !getterMethod->isAttribute())
        {
            getterMethod = OREF_NULL;
        }

        setterMethod = findClassMethod(setterName);
        if (setterMethod != OREF_NULL && !setterMethod->isAttribute())
        {
            setterMethod = OREF_NULL;
        }
    }

    // if we did not find any attribute methods at all, then there are no
    // annotations to process.
    if (getterMethod == OREF_NULL && setterMethod == OREF_NULL)
    {
        syntaxError(Error_Translation_missing_annotation_target, "attribute", getterName);
    }

    // parse this one, then merge into each of the methods
    Protected<StringTable> annotations = new_string_table();

    // now start on the attribute tokens
    RexxToken *token = nextReal();

    // an ::ANNOTATE directive need not specify anything, but if it
    // does, everything is in the form of "symbol constant" pairs.
    while (!token->isEndOfClause())
    {
        processAnnotation(token, annotations);
        token = nextReal();
    }

    // now merge these annotations into the methods that exist
    if (getterMethod != OREF_NULL)
    {
        annotations->putAll(getterMethod->getAnnotations());
    }

    // now merge these annotations into the methods that exist
    if (setterMethod != OREF_NULL)
    {
        annotations->putAll(setterMethod->getAnnotations());
    }
}


/**
 * Parse off a single name/value pair on a package attribute.
 *
 * @param token  The current token, which should be the name of an annotation.
 * @param table  The target annotation table.
 */
void LanguageParser::processAnnotation(RexxToken *token, StringTable *table)
{
    // the names must be a symbol
    if (!token->isSymbol())
    {
        syntaxError(Error_Symbol_expected_annotation_attribute, token);
    }

    // get the expressed name and the name we use for the methods
    RexxString *name = token->value();

    // we only expect just a single value token here
    token = nextReal();
    Protected<RexxObject> value;

    // the value omitted?  This is an error
    if (token->isEndOfClause())
    {
        syntaxError(Error_Symbol_or_string_package_attribute_missing);
    }
    // not a symbol or literal...we have special checks for signed numbers
    else if (!token->isSymbolOrLiteral())
    {
        // if not a "+" or "-" operator, this is an error
        if (!token->isOperator() || (!token->isSubtype(OPERATOR_SUBTRACT, OPERATOR_PLUS)))
        {
            syntaxError(Error_Symbol_or_string_package_attribute_bad_value, token);
        }
        RexxToken *second = nextReal();
        // this needs to be a constant symbol...we check for
        // numeric below
        if (!second->isSymbol() || !second->isSubtype(SYMBOL_CONSTANT))
        {
            syntaxError(Error_Symbol_or_string_package_attribute_bad_value, token);
        }
        // concat with the sign operator
        value = token->value()->concat(second->value());
        // and validate that this a valid number
        if (value->numberString() == OREF_NULL)
        {
            syntaxError(Error_Symbol_or_string_package_attribute_bad_value, value);
        }
    }
    else
    {
        // this will be some sort of literal value
        value = token->value();
    }

    // add this to the annotation table.
    table->put(value, name);
}


/**
 * Process a ::RESOURCE directive in a source file.
 */
void LanguageParser::resourceDirective()
{
    // the first token needs to be a resource name.
    RexxToken *token = nextReal();
    if (!token->isSymbolOrLiteral())
    {
        syntaxError(Error_Symbol_or_string_resource, token);
    }

    // get the expressed name and the name we use for the publishing the resource
    RexxString *name = token->value();
    RexxString *internalname = commonString(name->upper());

    RexxString *endMarker = GlobalNames::DEFAULT_RESOURCE_END;

    // ok, we can have an END keyword here indicating the end of data marker.
    token = nextReal();

    // if we have something else here, check for an END marker definition (the only
    // option currently allowed)
    if (!token->isEndOfClause())
    {
        // the keyword must be a symbol
        if (!token->isSymbol())
        {
            syntaxError(Error_Invalid_subkeyword_resource, token);
        }

        // the only option currently supported is END
        if (token->subDirective() != SUBDIRECTIVE_END)
        {
            syntaxError(Error_Invalid_subkeyword_resource, token);
        }

        // this is a required string or symbol value
        token = nextReal();
        if (!token->isSymbolOrLiteral())
        {
            syntaxError(Error_Symbol_or_string_resource_end, token);
        }

        // get the new end marker
        endMarker = token->value();

        // nothing more permitted after this
        requiredEndOfClause(Error_Invalid_data_resource_dir);
    }


    // check for a duplicate resource name
    if (resources->hasIndex(internalname))
    {
        syntaxError(Error_Translation_duplicate_resource);
    }

    // ok, we need to scan the lines following this directive for
    // the marker.
    Protected<ArrayClass> resource = new_array();

    // step to the next line if we're not already there by virtue
    // of hitting the end of line when scanning the clause
    conditionalNextLine();

    // now loop, looking for
    while (true)
    {
        // the end marker is required.  If we hit the end without finding this,
        // raise an error.  This keeps us from inadvertantly swallowing the rest of
        // the program
        if (!moreLines())
        {
            syntaxError(Error_Translation_missing_resource_end, endMarker, name);
        }

        // see if the next line is our marker
        if (checkMarker(endMarker))
        {
            // add this resource to our list
            resources->put(resource, internalname);
            // step to the next line to continue parsing...and we're done
            nextLine();
            return;
        }

        // extract this line as a string and add to the resource array
        resource->append(getStringLine());

        // and step to the next line
        nextLine();
    }
}


/**
 * Create a Rexx method body.
 *
 * @param name      The name of the attribute.
 * @param classMethod
 *                  Indicates whether we are creating a class or instance method.
 * @param privateMethod
 *                  The method's private attribute.
 * @param protectedMethod
 *                  The method's protected attribute.
 * @param guardedMethod
 *                  The method's guarded attribute.
 * @param isAttribute Indicates if this is an attribute method.
 */
void LanguageParser::createMethod(RexxString *name, bool classMethod,
    AccessFlag privateMethod, ProtectedFlag protectedMethod, GuardFlag guardedMethod, bool isAttribute)
{
    // NOTE:  It is necessary to translate the block and protect the code
    // before allocating the MethodClass object.  The new operator allocates the
    // the object first, then evaluates the constructor arguments after the allocation.
    // Since the translateBlock() call will allocate a lot of new objects before returning,
    // there's a high probability that the method object can get garbage collected before
    // there is any opportunity to protect the object.
    Protected<RexxCode> code = translateBlock();

    // convert into a method object
    Protected<MethodClass> _method = new MethodClass(name, code);
    _method->setAttributes(privateMethod, protectedMethod, guardedMethod);
    // mark with the attribute state
    _method->setAttribute(isAttribute);
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
    bool classMethod, AccessFlag privateMethod, ProtectedFlag protectedMethod, GuardFlag guardedMethod)
{
    // create the kernel method for the accessor
    Protected<BaseCode> code = new AttributeGetterCode(retriever);
    Protected<MethodClass> _method = new MethodClass(name, code);
    _method->setAttributes(privateMethod, protectedMethod, guardedMethod);
    // mark as an attribute method
    _method->setAttribute();
    // add this to the target
    addMethod(name, _method, classMethod);
}


/**
 * Create a DELEGATE method.
 *
 * @param name      The name of the method.
 * @param retriever The retriever for the target variable.
 * @param classMethod
 *                  Indicates we're adding a class or instance method.
 * @param privateMethod
 *                  The method's private attribute.
 * @param protectedMethod
 *                  The method's protected attribute.
 * @param guardedMethod
 *                  The method's guarded attribute.
 * @param isAttribute
 *                  Indicates whether this is an attribute method.
 */
void LanguageParser::createDelegateMethod(RexxString *name, RexxVariableBase *retriever,
    bool classMethod, AccessFlag privateMethod, ProtectedFlag protectedMethod, GuardFlag guardedMethod, bool isAttribute)
{
    // create the kernel method for the accessor
    Protected<BaseCode> code = new DelegateCode(retriever);
    Protected<MethodClass> _method = new MethodClass(name, code);
    _method->setAttributes(privateMethod, protectedMethod, guardedMethod);
    // mark with the attribute state
    _method->setAttribute(isAttribute);
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
    bool classMethod, AccessFlag privateMethod, ProtectedFlag protectedMethod, GuardFlag guardedMethod)
{
    // create the kernel method for the accessor
    Protected<BaseCode> code = new AttributeSetterCode(retriever);
    Protected<MethodClass> _method = new MethodClass(name, code);
    _method->setAttributes(privateMethod, protectedMethod, guardedMethod);
    // mark as an attribute method
    _method->setAttribute();
    // add this to the target
    addMethod(name, _method, classMethod);
}


/**
 * Create an abstract method.
 *
 * @param name   The name of the method.
 * @param classMethod
 *               Indicates we're adding a class or instance method.
 * @param privateMethod
 *               The method's private attribute.
 * @param protectedMethod
 *               The method's protected attribute.
 * @param guardedMethod
 *               The method's guarded attribute.
 * @param isAttribute
 *               The method attribute status.
 */
void LanguageParser::createAbstractMethod(RexxString *name,
    bool classMethod, AccessFlag privateMethod, ProtectedFlag protectedMethod, GuardFlag guardedMethod, bool isAttribute)
{
    // create the kernel method for the accessor
    // this uses a special code block
    Protected<BaseCode> code = new AbstractCode();
    Protected<MethodClass> _method = new MethodClass(name, code);
    _method->setAttributes(privateMethod, protectedMethod, guardedMethod);
    // mark with the attribute state
    _method->setAttribute(isAttribute);
    // and also mark as abstract
    _method->setAbstract();
    // add this to the target
    addMethod(name, _method, classMethod);
}


/**
 * Create a CONSTANT "get" method.
 *
 * @param name       The name of the attribute.
 * @param value      A potential expression that needs to be evaluated at run time.
 * @param expression An expression to evaluate if this is the dynamic form.
 * @param location   The clause location information.
 */
void LanguageParser::createConstantGetterMethod(RexxString *name, RexxObject *value, RexxInternalObject *expression, SourceLocation &location)
{
    Protected<ConstantGetterCode> code = new ConstantGetterCode(name, value);
    // add this as an unguarded method
    Protected<MethodClass> method = new MethodClass(name, code);
    method->setUnguarded();
    // mark as a constant method
    method->setConstant();

    // if we do not have an active class, only real constants are allowed.
    if (activeClass == OREF_NULL)
    {
        // if this is the evaluated form, we can't process this outside of the
        // context of a class object.
        if (expression != OREF_NULL)
        {
            syntaxError(Error_Translation_constant_no_class, name);
        }
        addMethod(name, method, false);
    }
    else
    {
        // connect this to the source package before adding to the class.
        method->setPackageObject(package);
        // if we have an expression, then value was null and the method completed
        // above is incomplete. We need to create a new ConstantDirective instruction
        // that will evaluate this expression when the package is installed.
        if (expression != OREF_NULL)
        {
            // this gets added to the active class for evaluation later
            Protected<ConstantDirective> directive = new ConstantDirective(code, expression, clause);
            // the clause we provided is the wrong location, so set the correct position.
            directive->setLocation(location);
            activeClass->addConstantMethod(name, method, directive, constantMaxStack, constantVariableIndex);
        }
        else
        {
            // just a normal "constant" constant
            activeClass->addConstantMethod(name, method);
        }
    }
}


/**
 * Process a ::routine directive in a source file.
 */
void LanguageParser::routineDirective()
{
    // must start with a name token
    RexxToken *token = nextReal();

    if (!token->isSymbolOrLiteral())
    {
        syntaxError(Error_Symbol_or_string_routine, token);
    }

    // NOTE:  routine lookups are case sensitive if the name
    // is quoted, so we don't uppercase this name.
    RexxString *name = token->value();
    if (isDuplicateRoutine(name))
    {
        syntaxError(Error_Translation_duplicate_routine);
    }

    // setup option defaults and then process any remaining options.
    RexxString *externalname = OREF_NULL;
    AccessFlag accessFlag = DEFAULT_ACCESS_SCOPE;
    for (;;)
    {
        token = nextReal();
        // finished?
        if (token->isEndOfClause())
        {
            break;
        }
        // all options must be symbols
        else if (!token->isSymbol())
        {
            syntaxError(Error_Invalid_subkeyword_routine, token);
        }

        switch (token->subDirective())
        {
            // written in native code backed by an external library
            case SUBDIRECTIVE_EXTERNAL:
                if (externalname != OREF_NULL)
                {
                    syntaxError(Error_Invalid_subkeyword_class, token);
                }
                token = nextReal();
                // this is a compound string descriptor, so it must be a literal
                if (!token->isLiteral())
                {
                    syntaxError(Error_Symbol_or_string_external, token);
                }

                externalname = token->value();
                break;

            // publicly available to programs that require this
            case SUBDIRECTIVE_PUBLIC:
                if (accessFlag != DEFAULT_ACCESS_SCOPE)
                {
                    syntaxError(Error_Invalid_subkeyword_routine, token);

                }
                accessFlag = PUBLIC_SCOPE;
                break;

            // this has private scope (the default)
            case SUBDIRECTIVE_PRIVATE:
                if (accessFlag != DEFAULT_ACCESS_SCOPE)
                {
                    syntaxError(Error_Invalid_subkeyword_routine, token);
                }
                accessFlag = PRIVATE_SCOPE;
                break;

            // something invalid
            default:
                syntaxError(Error_Invalid_subkeyword_routine, token);
                break;
        }
    }
    {
        // is this mapped to an external library?
        if (externalname != OREF_NULL)
        {
            // convert external into array of words (this also adds all words
            // to the common string pool and uppercases the first word)
            Protected<ArrayClass> _words = words(externalname);
            if (_words->size() == 0)
            {
                syntaxError(Error_Translation_bad_external, externalname);
            }

            // ::ROUTINE foo EXTERNAL "LIBRARY libbar [foo]"
            // NOTE: decodeExternalMethod doesn't really work for routines
            // because we have a second form.  Not really worth writing
            // a second version just for one use.
            if (((RexxString *)(_words->get(1)))->strCompare("LIBRARY"))
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
                    syntaxError(Error_Translation_bad_external, externalname);
                }

                // make sure this is followed by a directive or end of file
                checkDirective(Error_Translation_external_routine);
                // create a new native method.
                RoutineClass *routine = PackageManager::resolveRoutine(library, entry);
                // raise an exception if this entry point is not found.
                if (routine == OREF_NULL)
                {
                     syntaxError(Error_External_name_not_found_routine, entry);
                }
                // make sure this is attached to the source object for context information
                routine->setPackageObject(package);
                // add to the routine directory
                routines->setEntry(name, routine);
                // if this is a public routine, add to the public directory as well.
                if (accessFlag == PUBLIC_SCOPE)
                {
                    // add to the public directory too
                    publicRoutines->setEntry(name, routine);
                }
            }

            // ::ROUTINE foo EXTERNAL "REGISTERED libbar [foo]"
            // this is an old-style external function.
            else if (((RexxString *)(_words->get(1)))->strCompare("REGISTERED"))
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
                    syntaxError(Error_Translation_bad_external, externalname);
                }

                // go check the next clause to make
                checkDirective(Error_Translation_external_routine);
                // resolve the native routine
                RoutineClass *routine = PackageManager::resolveRoutine(name, library, entry);
                // raise an exception if this entry point is not found.
                if (routine == OREF_NULL)
                {
                     syntaxError(Error_External_name_not_found_routine, entry);
                }
                // make sure this is attached to the source object for context information
                routine->setPackageObject(package);
                // add to the routine directory
                routines->setEntry(name, routine);
                // if this is a public routine, add to the public directory as well.
                if (accessFlag == PUBLIC_SCOPE)
                {
                    // add to the public directory too
                    publicRoutines->setEntry(name, routine);
                }
            }
            else
            {
                // unknown external type
                syntaxError(Error_Translation_bad_external, externalname);
            }
        }
        else
        {
            // NOTE:  It is necessary to translate the block and protect the code
            // before allocating the RoutineClass object.  The new operator allocates the
            // the object first, then evaluates the constructor arguments after the allocation.
            // Since the translateBlock() call will allocate a lot of new objects before returning,
            // there's a high probability that the method object can get garbage collected before
            // there is any opportunity to protect the object.
            Protected<RexxCode> code = translateBlock();
            Protected<RoutineClass> routine = new RoutineClass(name, code);
            // make sure this is attached to the source object for context information
            routine->setPackageObject(package);
            // add to the routine directory
            routines->setEntry(name, routine);
            // if this is a public routine, add to the public directory as well.
            if (accessFlag == PUBLIC_SCOPE)
            {
                // add to the public directory too
                publicRoutines->setEntry(name, routine);
            }
        }
    }
}


/**
 * Process a ::REQUIRES directive.
 */
void LanguageParser::requiresDirective()
{
    bool isLibrary = false;
    RexxString *namespaceName = OREF_NULL;

    // the required entity name is a string or symbol
    RexxToken *token = nextReal();
    if (!token->isSymbolOrLiteral())
    {
        syntaxError(Error_Symbol_or_string_requires, token);
    }
    // we have the name, and we have a couple options we can process.
    RexxString *name = token->value();
    for (;;)
    {
        token = nextReal();
        // finished?
        if (token->isEndOfClause())
        {
            break;
        }
        // all options must be symbols
        else if (!token->isSymbol())
        {
            syntaxError(Error_Invalid_subkeyword_requires, token);
        }

        switch (token->subDirective())
        {
            // we have a label on the requires
            case SUBDIRECTIVE_NAMESPACE:
            {
                // can only have one of these and cannot have this with the LIBRARY option
                if (isLibrary || namespaceName != OREF_NULL)
                {
                    syntaxError(Error_Invalid_subkeyword_requires, token);
                }
                // get the label token
                token = nextReal();
                if (!token->isSymbol())
                {
                    syntaxError(Error_Symbol_expected_namespace);
                }
                // NOTE:  since this is a symbol, the label will be an
                // uppercase name.
                namespaceName = token->value();

                // REXX is a reserved namespace name.
                if (namespaceName->strCompare(GlobalNames::REXX))
                {
                    syntaxError(Error_Translation_reserved_namespace);
                }

                break;
            }

            // this identifies a library
            case SUBDIRECTIVE_LIBRARY:
            {
                // can only specify library once and for now, at least,
                // the LABEL keyword is not allowed on a LIBRARY requires.
                // this might have some meaning eventually for resolving
                // external routines, but for now, this is a restriction.
                if (isLibrary || namespaceName != OREF_NULL)
                {
                    syntaxError(Error_Invalid_subkeyword_requires, token);
                }
                isLibrary = true;
                break;
            }

            default:
                syntaxError(Error_Invalid_subkeyword_requires, token);
                break;
        }
    }

    // this is either a library directive or a requires directive
    if (isLibrary)
    {
        libraries->append(new LibraryDirective(name, clause));
    }
    else
    {
        requires->append(new RequiresDirective(name, namespaceName, clause));
    }
}


