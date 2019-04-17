/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2019 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                                                */
/*                                                                            */
/* Language translator defintions.                                            */
/*                                                                            */
/******************************************************************************/
#ifndef Included_LanguageParser
#define Included_LanguageParser

#include "FlagSet.hpp"
#include "SourceLocation.hpp"
#include "ListClass.hpp"
#include "QueueClass.hpp"
#include "MemoryStack.hpp"
#include "Token.hpp"
#include "Clause.hpp"
#include "RexxInstruction.hpp"
#include "DoBlockComponents.hpp"
#include "LanguageLevel.hpp"
#include "RexxErrorCodes.h"
#include "CommandIOConfiguration.hpp"
#include "MethodClass.hpp"


class RexxInstruction;
class RexxInstructionIf;
class RexxExpressionMessage;
class RexxCompoundVariable;
class RoutineClass;
class RexxCode;
class PackageClass;
class ClassDirective;
class RexxActivation;
class ExpressionStack;
class StackFrameClass;
class ProgramSource;
class RexxVariableBase;
class RexxStemVariable;
class TraceSetting;
class ClassResolver;


// handy defines for simplifying creation of instruction types.
#define new_instruction(name, type) sourceNewObject(sizeof(RexxInstruction##type), The##type##InstructionBehaviour, KEYWORD_##name)
#define new_variable_instruction(name, type, count, itemType) sourceNewObject(sizeof(RexxInstruction##type), count, sizeof(itemType), The##type##InstructionBehaviour, KEYWORD_##name)

// context flag values.
typedef enum
{
    interpret,            // interpreting code...
    reclaimed,            // we have a reclaimed clause
    noClause,             // no additional clauses are available
    noDirectives          // parsing cannot include directives.
} ParsingFlags;

// builtin function code prototype
typedef RexxObject *builtin_func(RexxActivation *, size_t, ExpressionStack *);
// pointer to a builtin function
typedef builtin_func *pbuiltin;

/**
 * Main class for parsing Rexx source into an executable
 * entity.
 */
class LanguageParser: public RexxInternalObject
{
 public:
    void        *operator new(size_t);
    inline void  operator delete(void *) { ; }

    LanguageParser(RexxString *name, ProgramSource *s);
    inline LanguageParser(RESTORETYPE restoreType) { ; };

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;

    // main execution methods
    void        translate();
    void        compileSource();
    void        initializeForParsing();
    void        initializeForDirectives();
    void        initializeForTranslation();
    void        resolveDependencies();
    void        flushControl(RexxInstruction *);
    RexxCode   *translateBlock();
    RexxCode   *translateInterpret(PackageClass *sourceContext, StringTable *contextLabels);
    RexxInternalObject *translateConstantExpression(RexxToken *token, RexxErrorCodes error);
    RoutineClass *generateProgram(PackageClass *sourceContext = OREF_NULL);
    RoutineClass *generateRoutine(PackageClass *sourceContext = OREF_NULL);
    MethodClass *generateMethod(PackageClass *sourceContext = OREF_NULL);
    PackageClass *generatePackage(PackageClass *sourceContext = OREF_NULL);

    PackageClass *getPackage() { return package; }

    void         installPackage();
    StackFrameClass *createStackFrame();
    void        holdObject(RexxInternalObject *object) { holdStack->push(object);};

    // token scanning methods
    void        scanComment();
    RexxToken  *scanSymbol();
    RexxToken  *scanLiteral();
    void        nextLine();
    void        conditionalNextLine();
    bool        checkMarker(RexxString *marker);
    RexxString *getStringLine();
    void        position(size_t, size_t);
    bool        nextClause();
    RexxToken  *sourceNextToken(RexxToken *);
    bool        nextSpecial(unsigned int, SourceLocation &);
    CharacterClass locateToken(unsigned int &, bool);
    RexxString *packHexLiteral(size_t, size_t);
    RexxString *packBinaryLiteral(size_t, size_t);
    RexxToken  *getToken(int term, RexxErrorCodes error = Error_None);
    inline unsigned int getChar() { return (unsigned char)(current[lineOffset]); }
    inline unsigned int getChar(size_t o) { return (unsigned char)(current[o]); }
    inline unsigned int nextChar() { return (unsigned char)(current[lineOffset++]); }
    inline unsigned int getNextChar() { return (unsigned char)(current[lineOffset + 1]); }
    inline unsigned int followingChar() { return haveNextChar() ? getNextChar() : INVALID_CHARACTER; }
    inline void        stepPosition() { lineOffset++; }
    inline void        stepPosition(size_t o) { lineOffset += o; }
    inline bool        moreChars() { return lineOffset < currentLength; }
    inline bool        haveNextChar() {  return lineOffset < currentLength; }
    inline bool        moreLines() { return (lineNumber <= lineCount); }
    inline void        truncateLine() { lineOffset = currentLength; }

    // general parsing methods
    void        needVariable(RexxToken *);
    void        needVariableOrDotSymbol(RexxToken *);
    const SourceLocation& getLocation() { return clauseLocation; }
    void        startLocation(SourceLocation &);
    void        endLocation(SourceLocation &);
    void        setLocation(SourceLocation &);
    inline void        previousToken() { clause->previous(); }
    inline void        firstToken() { clause->firstToken(); }
    inline void        trimClause() { clause->trim(); }
    inline size_t      markPosition() { return clause->mark(); }
    inline void        resetPosition(size_t p) { clause->reset(p); }

    RexxVariableBase *addSimpleVariable(RexxString *);
    RexxStemVariable  *addStem(RexxString *);
    RexxCompoundVariable *addCompound(RexxString *);
    void        expose(RexxString *);
    void        localVariable(RexxString *);
    void        autoExpose();
    RexxString *commonString(RexxString *);
    RexxInternalObject *addText(RexxToken *);
    RexxVariableBase *addVariable(RexxToken *);
    RexxVariableBase *requiredVariable(RexxToken *, const char *);
    void        addClause(RexxInstruction *);
    void        resolveCalls();
    void        addLabel(RexxInstruction *, RexxString *);
    RexxInstruction *findLabel(RexxString *);
    void        setGuard();
    ArrayClass  *getGuard();
    void        addBlock();
    RexxVariableBase *getRetriever(RexxString *);
    ArrayClass  *words(RexxString *);
    inline void  reclaimClause()  { flags.set(reclaimed); };
    inline bool  atEnd() { return !flags.test(reclaimed) && !moreLines(); };

           void setInterpret() { flags.set(interpret); flags.set(noDirectives); }
    inline bool isInterpret() { return flags.test(interpret); }
    inline bool noClauseAvailable() { return flags.test(noClause); }
    inline bool clauseAvailable() { return !flags.test(noClause); }
    inline RexxToken  *nextToken() { return clause->next(); }
    inline RexxToken  *nextReal() { return clause->nextRealToken(); }
    inline void        requiredEndOfClause(RexxErrorCodes error)
    {
        RexxToken *token = nextReal();
        if (!token->isEndOfClause())
        {
            syntaxError(error, token);
        }
        // NOTE:  Some contexts where this is tested are
        // in loops that still process the next token.  Back up
        // to the clause terminator so it is still there.
        previousToken();
    }

    inline RexxInternalObject *requiredLogicalExpression(int terminators, RexxErrorCodes error)
    {
        RexxInternalObject *conditional = parseLogical(terminators);
        if (conditional == OREF_NULL)
        {
            syntaxError(error);
        }
        return conditional;
    }

    inline RexxInternalObject *requiredExpression(int terminators, RexxErrorCodes error)
    {
        RexxInternalObject *expression = parseExpression(terminators);
        if (expression == OREF_NULL)
        {
            syntaxError(error);
        }
        return expression;
    }

    inline bool capturingGuardVariables() { return guardVariables != OREF_NULL; }
           bool isExposed(RexxString *varName);
    void captureGuardVariable(RexxString *varname, RexxVariableBase *retriever);
    void requireLanguageLevel(LanguageLevel l);
    static bool canExecute(LanguageLevel l);

    // instruction parsing methods
    RexxInstruction *nextInstruction();
    size_t      processVariableList(InstructionKeyword);

    RexxInstruction *addressNew();
    RexxInstruction *assignmentNew(RexxToken *);
    RexxInstruction *assignmentOpNew(RexxToken *, RexxToken *);
    RexxInstruction *callOnNew(InstructionSubKeyword type);
    RexxInstruction *dynamicCallNew(RexxToken *token);
    RexxInstruction *qualifiedCallNew(RexxToken *token);
    RexxInstruction *callNew();
    RexxInstruction *commandNew();
    RexxInstruction *doNew();
    RexxInstruction *newControlledLoop(RexxString *label, RexxVariableBase *countVariable, RexxToken *nameToken);
    RexxInstruction *newDoOverLoop(RexxString *label, RexxVariableBase *countVariable, RexxToken *nameToken);
    RexxInstruction *newDoWithLoop(RexxString *label, RexxVariableBase *countVariable);
    RexxInstruction *newSimpleDo(RexxString *label);
    RexxInstruction *newLoopForever(RexxString *label, RexxVariableBase *countVariable);
    RexxInstruction *newLoopWhile(RexxString *label, RexxVariableBase *countVariable, WhileUntilLoop &conditional);
    RexxInstruction *newLoopUntil(RexxString *label, RexxVariableBase *countVariable, WhileUntilLoop &conditional);
    RexxInstruction *parseForeverLoop(RexxString *label, RexxVariableBase *countVariable);
    RexxInstruction *parseCountLoop(RexxString *label, RexxVariableBase *countVariable);
    RexxInstruction *createLoop(bool isLoop);
    RexxInstruction *dropNew();
    RexxInstruction *elseNew(RexxToken *);
    RexxInstruction *endNew();
    RexxInstruction *endIfNew(RexxInstructionIf *);
    RexxInstruction *exitNew();
    RexxInstruction *exposeNew();
    RexxInstruction *forwardNew();
    RexxInstruction *guardNew();
    RexxInstruction *ifNew();
    RexxInstruction *whenNew();
    RexxInstruction *interpretNew();
    RexxInstruction *labelNew(RexxToken *name, RexxToken *colon);
    RexxInstruction *leaveNew(InstructionKeyword type);
    RexxInstruction *messageNew(RexxExpressionMessage *);
    RexxInstruction *doubleMessageNew(RexxExpressionMessage *msg);
    RexxInstruction *messageAssignmentNew(RexxExpressionMessage *, RexxInternalObject *);
    RexxInstruction *messageAssignmentOpNew(RexxExpressionMessage *, RexxToken *, RexxInternalObject *);
    RexxInstruction *nopNew();
    RexxInstruction *numericNew();
    RexxInstruction *optionsNew();
    RexxInstruction *otherwiseNew(RexxToken *);
    RexxInstruction *parseNew(InstructionSubKeyword);
    RexxInstruction *procedureNew();
    RexxInstruction *pushNew();
    RexxInstruction *queueNew();
    RexxInstruction *raiseNew();
    RexxInstruction *replyNew();
    RexxInstruction *returnNew();
    RexxInstruction *sayNew();
    RexxInstruction *selectNew();
    RexxInstruction *dynamicSignalNew();
    RexxInstruction *signalOnNew(InstructionSubKeyword type);
    RexxInstruction *signalNew();
    RexxInstruction *thenNew(RexxToken *, RexxInstructionIf *);
    RexxInstruction *traceNew();
    RexxInstruction *useNew();
    RexxInstruction *useLocalNew();

    inline void        addReference(RexxInternalObject *reference) { calls->addLast(reference); }
    inline void        pushDo(RexxInstruction *i) { control->push(i); }
    inline RexxInstruction *popDo() { return (RexxInstruction *)(control->pull()); }
    inline RexxInstruction *topDo() { return (RexxInstruction *)(control->peek()); }
           RexxInstruction *topBlockInstruction();
    inline InstructionKeyword topDoType() { return ((RexxInstruction *)(control->peek()))->getType(); }
    inline bool topDoIsType(InstructionKeyword t) { return ((RexxInstruction *)(control->peek()))->isType(t); }
    inline bool topDoIsType(InstructionKeyword t1, InstructionKeyword t2) { return topDoIsType(t1) || topDoIsType(t2); }
    RexxInstruction *sourceNewObject(size_t size, RexxBehaviour *_behaviour, InstructionKeyword type);
    RexxInstruction *sourceNewObject(size_t size, size_t count, size_t itemSize, RexxBehaviour *_behaviour, InstructionKeyword type);

    // directive parsing methods
    void        nextDirective();
    void        routineDirective();
    void        requiresDirective();
    void        libraryDirective(RexxString *name, RexxToken *token);
    void        methodDirective();
    void        classDirective();
    void        attributeDirective();
    void        constantDirective();
    void        annotateDirective();
    void        optionsDirective();
    void        resourceDirective();
    void        addInstalledClass(RexxString *name, RexxClass *classObject, bool publicClass);
    void        addInstalledRoutine(RexxString *name, RoutineClass *routineObject, bool publicRoutine);
    void        processAnnotation(RexxToken *token, StringTable *table);
    void        processAttributeAnnotations(RexxString *getterName);
    ClassResolver *parseClassReference(RexxErrorCodes error);

    // methods to support directive parsing
    void        checkDirective(RexxErrorCodes errorCode);
    bool        hasBody();
    void        decodeExternalMethod(RexxString *methodName, RexxString *externalSpec, RexxString *&library, RexxString *&procedure);
    MethodClass *createNativeMethod(RexxString *name, RexxString *library, RexxString *procedure);
    void        createMethod(RexxString *name, bool classMethod, AccessFlag privateMethod, ProtectedFlag protectedMethod, GuardFlag guardedMethod, bool isAttribute);
    void        createAttributeGetterMethod(RexxString *name, RexxVariableBase *retriever, bool classMethod, AccessFlag privateMethod, ProtectedFlag protectedMethod, GuardFlag guardedMethod);
    void        createAttributeSetterMethod(RexxString *name, RexxVariableBase *retriever, bool classMethod, AccessFlag privateMethod, ProtectedFlag protectedMethod, GuardFlag guardedMethod);
    void        createDelegateMethod(RexxString *name, RexxVariableBase *retriever, bool classMethod, AccessFlag privateMethod, ProtectedFlag protectedMethod, GuardFlag guardedMethod, bool isAttribute);
    void        createConstantGetterMethod(RexxString *name, RexxObject *value, RexxInternalObject *expression, SourceLocation &location);
    void        createAbstractMethod(RexxString *name, bool classMethod, AccessFlag privateMethod, ProtectedFlag protectedMethod, GuardFlag guardedMethod, bool isAttribute);
    void        checkDuplicateMethod(RexxString *name, bool classMethod, RexxErrorCodes errorMsg);
    void        addMethod(RexxString *name, MethodClass *method, bool classMethod);
    bool        isDuplicateClass(RexxString *name);
    bool        isDuplicateRoutine(RexxString *name);
    void        addClassDirective(RexxString *name, ClassDirective *directive);
    ClassDirective *findClassDirective(RexxString *name);
    RoutineClass *findRoutine(RexxString *name);
    MethodClass *findMethod(RexxString *name);
    MethodClass *findClassMethod(RexxString *name);
    MethodClass *findInstanceMethod(RexxString *name);

    // expression parsing methods
    RexxInternalObject *parseConstantExpression();
    RexxInternalObject *parenExpression(RexxToken *);
    RexxInternalObject *parseExpression(int);
    RexxInternalObject *parseSubExpression(int);
    RexxInternalObject *parseFullSubExpression(int);
    size_t       parseArgList(RexxToken *, int);
    ArrayClass  *parseArgArray(RexxToken *, int);
    size_t       parseCaseWhenList(int terminators );
    RexxInternalObject *parseFunction(RexxToken *, RexxToken *);
    RexxInternalObject *parseQualifiedSymbol(RexxString *namespaceName);
    RexxInternalObject *parseCollectionMessage(RexxToken *, RexxInternalObject *);
    RexxInternalObject *parseMessage(RexxInternalObject *, bool, int);
    RexxInternalObject *parseMessageTerm();
    RexxInternalObject *parseVariableOrMessageTerm();
    RexxInternalObject *parseMessageSubterm(int);
    RexxInternalObject *parseSubTerm(int);
    RexxInternalObject *parseLoopConditional(InstructionSubKeyword &, RexxErrorCodes);
    RexxInternalObject *parseLogical(int terminators);
    inline void        pushOperator(RexxToken *operatorToken) { operators->push(operatorToken); };
    inline RexxToken  *popOperator() { return (RexxToken *)(operators->pull()); };
    inline RexxToken  *topOperator() { return (RexxToken *)(operators->peek()); };
    void        pushTerm(RexxInternalObject *);
    void        pushSubTerm(RexxInternalObject *);
    RexxInternalObject *requiredTerm(RexxToken *token, RexxErrorCodes errorCode = Error_Invalid_expression_general);
    RexxInternalObject *popTerm();
    RexxInternalObject *popSubTerm();
    RexxInternalObject *popNTerms(size_t);
    CommandIOConfiguration *parseAddressWith();
    bool checkRedirectNormal(RexxToken *token);
    OutputOption::Enum parseRedirectOutputOptions();
    void parseRedirectOptions(RexxInternalObject *&source, RedirectionType::Enum &type);
    RexxInternalObject *parseVariableReferenceTerm();

    // various error processing methods
    void        error(RexxErrorCodes);
    void        error(RexxErrorCodes, RexxObject *);
    void        error(RexxErrorCodes, RexxObject *, RexxObject *);
    void        error(RexxErrorCodes, RexxObject *, RexxObject *, RexxObject *);
    void        error(RexxErrorCodes errorcode, const SourceLocation &location, ArrayClass *subs);
    void        errorLine(RexxErrorCodes, RexxInstruction *);
    void        errorPosition(RexxErrorCodes, RexxToken *);
    void        errorToken(RexxErrorCodes, RexxToken *);
    void        blockError(RexxInstruction *);

    inline void syntaxError(RexxErrorCodes errorcode, RexxInstruction *i) { errorLine(errorcode, i); }
    inline void blockSyntaxError(RexxInstruction *i) { blockError(i); }
    inline void syntaxErrorAt(RexxErrorCodes errorcode, RexxToken *token) { errorPosition(errorcode, token); }
    inline void syntaxError(RexxErrorCodes errorcode, RexxObject *a1) { error(errorcode, a1); }
    inline void syntaxError(RexxErrorCodes errorcode, RexxObject *a1, RexxObject *a2) { error(errorcode, a1, a2); }
    inline void syntaxError(RexxErrorCodes errorcode, RexxObject *a1, RexxObject *a2, RexxObject *a3) { error(errorcode, a1, a2, a3); }
    inline void syntaxError(RexxErrorCodes errorcode, RexxToken *token) { errorToken(errorcode, token); }
    inline void syntaxError(RexxErrorCodes errorcode) { error(errorcode); }


    // other useful static scanning routines
    static StringSymbolType scanSymbol(RexxString *string);

    static inline bool isSymbolCharacter(unsigned int ch)
    {
        // The anding is necessary to keep characters > 0x7F from being
        // treated as negative numbers and returning bogus values.
        // the test for less that 256 is necessary because we have
        // a magic "not-a-character" marker that's out of range.
        return ch < 256 && characterTable[ch & 0xff] != 0;
    }

    static inline int translateChar(unsigned int ch)
    {
        // The anding is necessary to keep characters > 0x7F from being
        // treated as negative numbers and returning bogus values.  This
        // assumes were're working with a good character already.
        return characterTable[ch & 0xff];
    }

    // static methods for creating/processing different Rexx executables.

    static MethodClass *createMethod(RexxString *name, ArrayClass *source, PackageClass *sourceContext);
    static MethodClass *createMethod(RexxString *name, BufferClass *source);
    static MethodClass *createMethod(RexxString *name, PackageClass *sourceContext);
    static RoutineClass *createRoutine(RexxString *name, ArrayClass *source, PackageClass *sourceContext);
    static RoutineClass *createRoutine(RexxString *name, BufferClass *source, PackageClass *sourceContext);
    static RoutineClass *createRoutine(RexxString *name, PackageClass *sourceContext);
    static RoutineClass *createProgram(RexxString *name, BufferClass *source);
    static RoutineClass *createProgram(RexxString *name, ArrayClass *source, PackageClass *sourceContext);
    static RoutineClass *createProgram(RexxString *name);
    static RoutineClass *restoreFromMacroSpace(RexxString *name);
    static RoutineClass *processInstore(PRXSTRING instore, RexxString * name);
    static RexxCode *translateInterpret(RexxString *interpretString, PackageClass *sourceContext, StringTable *labels, size_t lineNumber);
    static RoutineClass *createProgramFromFile(RexxString *filename);
    static PackageClass *createPackage(RexxString *filename);
    static PackageClass *createPackage(RexxString *name, ArrayClass *source, PackageClass *sourceContext = OREF_NULL);
    static PackageClass *createPackage(RexxString *name, BufferClass *source);

    // the table of builtin function stubs.
    static pbuiltin builtinTable[];

    // an invalid 8-bit character marker.
    static const unsigned int INVALID_CHARACTER = 0x100;

    // maximum length of a symbol
    static const size_t MAX_SYMBOL_LENGTH = 250;

protected:

    // size of our parsing holdstack used for short-term protection.
    static const size_t HOLDSIZE = 30;

    RexxString *name;                    // the name of the code we're translating (frequently a file name)
    ProgramSource *source;               // the source we're translating.
    PackageClass *package;               // the source package we're translating
    FlagSet<ParsingFlags, 32> flags;     // a set of flags with parse state
    const char *current;                 // current working line
    size_t currentLength;                // length of current line
    size_t lineCount;                    // count of lines in the source
    RexxClause *clause;                  // current clause being created
    SourceLocation clauseLocation;       // current clause location for errors
    size_t lineNumber;                   // current line position
    size_t lineOffset;                   // current offset with in the line

    RexxCode        *mainSection;        // the main section of code before any directives
    PushThroughStack *holdStack;         // stack for holding temporaries
    StringTable     *literals;           // root of associated literal list
    StringTable     *dotVariables;       // root of associated dot variable values
    StringTable     *strings;            // common pool of created strings
    QueueClass      *control;            // queue of control structures
    QueueClass      *terms;              // stack of expression terms
    QueueClass      *subTerms;           // stack for arguments lists, et al.
    QueueClass      *operators;          // stack of expression terms
    ClassDirective  *activeClass;        // currently active ::CLASS directive
    StringTable     *classDependencies;  // directory of named ::class directives
    StringTable     *unattachedMethods;  // methods not associated with any class
    StringTable     *routines;           // routines defined by ::routine directives.
    StringTable     *publicRoutines;     // routines defined by ::routine directives.
    StringTable     *resources;          // resources defined by ::resource directives
    ArrayClass      *requires;           // list of ::requires directories, in order of appearance.
    ArrayClass      *libraries;          // libraries identified on a ::requires directive.
    ArrayClass      *classes;            // list of installed ::class directives.

                                         // start of block parsing section

    RexxInstruction *firstInstruction;   // first instruction of parse tree
    RexxInstruction *lastInstruction;    // last instruction of parse tree
    RexxInstruction *currentInstruction; // current "protected" instruction
    StringTable     *variables;          // root of associated variable list
    StringTable     *labels;             // root of associated label list
    IdentityTable   *guardVariables;     // exposed variables in guard list
    StringTable     *exposedVariables;   // root of exposed variables list
    StringTable     *localVariables;     // list of explicitly specified local variables
    ArrayClass      *calls;              // root of call list

    size_t           currentStack;       // current expression stack depth
    size_t           maxStack;           // maximum stack depth
    size_t           variableIndex;      // current variable index slot
    size_t           constantMaxStack;           // maximum stack depth for the evaluated constants
    size_t           constantVariableIndex;      // current variable index slot for evaluated constants
    StringTable     *constantVariables;          // root of associated variable list for evaluated constants.

    // table of character values
    static int characterTable[];
};
#endif

