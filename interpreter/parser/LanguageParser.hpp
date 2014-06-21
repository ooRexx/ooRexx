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
/* REXX Kernel                                                                */
/*                                                                            */
/* Languate translator defintions.                                            */
/*                                                                            */
/******************************************************************************/
#ifndef Included_LanguageParser
#define Included_RexxSource

#include "SourceLocation.hpp"
#include "ListClass.hpp"
#include "QueueClass.hpp"
#include "StackClass.hpp"
#include "Token.hpp"
#include "Clause.hpp"

class RexxInstruction;
class RexxInstructionDo;
class RexxInstructionIf;
class RexxInstructionForward;
class RexxExpressionMessage;
class RexxCompoundVariable;
class RoutineClass;
class RexxCode;
class PackageClass;
class ClassDirective;
class RexxActivation;
class RexxExpressionStack;
class StackFrameClass;

// various keyword enumerations

// the keyword instruction and directive identifiers
enum
{
    KEYWORD_NONE = 0,
    KEYWORD_ADDRESS,
    KEYWORD_ARG,
    KEYWORD_CALL,
    KEYWORD_DO,
    KEYWORD_DROP,
    KEYWORD_EXIT,
    KEYWORD_IF,
    KEYWORD_INTERPRET,
    KEYWORD_ITERATE,
    KEYWORD_LEAVE,
    KEYWORD_NOP,
    KEYWORD_NUMERIC,
    KEYWORD_OPTIONS,
    KEYWORD_PARSE,
    KEYWORD_PROCEDURE,
    KEYWORD_PULL,
    KEYWORD_PUSH,
    KEYWORD_QUEUE,
    KEYWORD_REPLY,
    KEYWORD_RETURN,
    KEYWORD_SAY,
    KEYWORD_SELECT,
    KEYWORD_SIGNAL,
    KEYWORD_TRACE,
    KEYWORD_GUARD,
    KEYWORD_USE,
    KEYWORD_EXPOSE,
    KEYWORD_RAISE,
    KEYWORD_ELSE,
    KEYWORD_THEN,
    KEYWORD_END,
    KEYWORD_OTHERWISE,
    KEYWORD_IFTHEN,
    KEYWORD_WHENTHEN,
    KEYWORD_WHEN,
    KEYWORD_ASSIGNMENT,
    KEYWORD_COMMAND,
    KEYWORD_MESSAGE,
    KEYWORD_LABEL,
    KEYWORD_ENDIF,
    KEYWORD_BLOCK,
    KEYWORD_FIRST,
    KEYWORD_LAST,
    KEYWORD_ENDELSE,
    KEYWORD_ENDTHEN,
    KEYWORD_ENDWHEN,
    KEYWORD_INSTRUCTION,
    KEYWORD_FORWARD,
    KEYWORD_LOOP
} InstructionKeyword;

// instruction subkeyword types
enum {
    SUBKEY_NONE = 0,
    SUBKEY_ARG,
    SUBKEY_BY,
    SUBKEY_DIGITS,
    SUBKEY_END,
    SUBKEY_ELSE,
    SUBKEY_ENGINEERING,
    SUBKEY_EXPOSE,
    SUBKEY_FOR,
    SUBKEY_FOREVER,
    SUBKEY_FORM,
    SUBKEY_FUZZ,
    SUBKEY_LINEIN,
    SUBKEY_LOWER,
    SUBKEY_CASELESS,
    SUBKEY_NAME,
    SUBKEY_NOVALUE,
    SUBKEY_OFF,
    SUBKEY_ON,
    SUBKEY_OTHERWISE,
    SUBKEY_OVER,
    SUBKEY_PULL,
    SUBKEY_SCIENTIFIC,
    SUBKEY_SOURCE,
    SUBKEY_THEN,
    SUBKEY_TO,
    SUBKEY_UNTIL,
    SUBKEY_UPPER,
    SUBKEY_VALUE,
    SUBKEY_VAR,
    SUBKEY_VERSION,
    SUBKEY_WHEN,
    SUBKEY_WHILE,
    SUBKEY_WITH,
    SUBKEY_DESCRIPTION,
    SUBKEY_ADDITIONAL,
    SUBKEY_RESULT,
    SUBKEY_ARRAY,
    SUBKEY_RETURN,
    SUBKEY_EXIT,
    SUBKEY_CONTINUE,
    SUBKEY_CLASS,
    SUBKEY_MESSAGE,
    SUBKEY_ARGUMENTS,
    SUBKEY_LABEL,
    SUBKEY_STRICT,
    SUBKEY_TRUE,
    SUBKEY_FALSE
} InstructionSubKeyword;


enum {
    DIRECTIVE_NONE = 0,
    DIRECTIVE_METHOD,
    DIRECTIVE_OPTIONS,
    DIRECTIVE_REQUIRES,
    DIRECTIVE_CLASS,
    DIRECTIVE_ATTRIBUTE,
    DIRECTIVE_LIBRARY,
    DIRECTIVE_CONSTANT
} DirectiveKeyword;

// Identify different token types returned by
// locateToken()
enum {
    NORMAL_CHAR,
    SIGNIFICANT_BLANK,
    CLAUSE_EOF,
    CLAUSE_EOL,
} CharacterClass;

/* token extended types - end of clause */


enum {
    SUBDIRECTIVE_NONE = 0,
    SUBDIRECTIVE_PUBLIC,
    SUBDIRECTIVE_METACLASS,
    SUBDIRECTIVE_INHERIT,
    SUBDIRECTIVE_PRIVATE,
    SUBDIRECTIVE_GUARDED,
    SUBDIRECTIVE_CLASS,
    SUBDIRECTIVE_EXTERNAL,
    SUBDIRECTIVE_SUBCLASS,
    SUBDIRECTIVE_UNGUARDED,
    SUBDIRECTIVE_MIXINCLASS,
    SUBDIRECTIVE_ATTRIBUTE,
    SUBDIRECTIVE_PROTECTED,
    SUBDIRECTIVE_ABSTRACT,
    SUBDIRECTIVE_UNPROTECTED,
    SUBDIRECTIVE_GET,
    SUBDIRECTIVE_SET,
    SUBDIRECTIVE_LIBRARY,
    SUBDIRECTIVE_DIGITS,
    SUBDIRECTIVE_FORM,
    SUBDIRECTIVE_FUZZ,
    SUBDIRECTIVE_TRACE,
} DirectiveSubkeyword;


enum {
    CONDITION_NONE = 0,
    CONDITION_ANY,
    CONDITION_ERROR,
    CONDITION_FAILURE,
    CONDITION_HALT,
    CONDITION_NOMETHOD,
    CONDITION_NOSTRING,
    CONDITION_NOTREADY,
    CONDITION_NOVALUE,
    CONDITION_PROPAGATE,
    CONDITION_SYNTAX,
    CONDITION_USER,
    CONDITION_LOSTDIGITS
} ConditionKeywords;

// markers for the builtin function
enum {
    NO_BUILTIN  = 0,
    BUILTIN_ABBREV,
    BUILTIN_ABS,
    BUILTIN_ADDRESS,
    BUILTIN_ARG,
    BUILTIN_B2X,
    BUILTIN_BITAND,
    BUILTIN_BITOR,
    BUILTIN_BITXOR,
    BUILTIN_C2D,
    BUILTIN_C2X,
    BUILTIN_CENTER,
    BUILTIN_CENTRE,
    BUILTIN_CHANGESTR,
    BUILTIN_CHARIN,
    BUILTIN_CHAROUT,
    BUILTIN_CHARS,
    BUILTIN_COMPARE,
    BUILTIN_CONDITION,
    BUILTIN_COPIES,
    BUILTIN_COUNTSTR,
    BUILTIN_D2C,
    BUILTIN_D2X,
    BUILTIN_DATATYPE,
    BUILTIN_DATE,
    BUILTIN_DELSTR,
    BUILTIN_DELWORD,
    BUILTIN_DIGITS,
    BUILTIN_ERRORTEXT,
    BUILTIN_FORM,
    BUILTIN_FORMAT,
    BUILTIN_FUZZ,
    BUILTIN_INSERT,
    BUILTIN_LASTPOS,
    BUILTIN_LEFT,
    BUILTIN_LENGTH,
    BUILTIN_LINEIN,
    BUILTIN_LINEOUT,
    BUILTIN_LINES,
    BUILTIN_MAX,
    BUILTIN_MIN,
    BUILTIN_OVERLAY,
    BUILTIN_POS,
    BUILTIN_QUEUED,
    BUILTIN_RANDOM,
    BUILTIN_REVERSE,
    BUILTIN_RIGHT,
    BUILTIN_SIGN,
    BUILTIN_SOURCELINE,
    BUILTIN_SPACE,
    BUILTIN_STREAM,
    BUILTIN_STRIP,
    BUILTIN_SUBSTR,
    BUILTIN_SUBWORD,
    BUILTIN_SYMBOL,
    BUILTIN_TIME,
    BUILTIN_TRACE,
    BUILTIN_TRANSLATE,
    BUILTIN_TRUNC,
    BUILTIN_VALUE,
    BUILTIN_VAR,
    BUILTIN_VERIFY,
    BUILTIN_WORD,
    BUILTIN_WORDINDEX,
    BUILTIN_WORDLENGTH,
    BUILTIN_WORDPOS,
    BUILTIN_WORDS,
    BUILTIN_X2B,
    BUILTIN_X2C,
    BUILTIN_X2D,
    BUILTIN_XRANGE,
    BUILTIN_USERID,
    BUILTIN_LOWER,
    BUILTIN_UPPER,
    BUILTIN_RXFUNCADD,
    BUILTIN_RXFUNCDROP,
    BUILTIN_RXFUNCQUERY,
    BUILTIN_ENDLOCAL,
    BUILTIN_SETLOCAL,
    BUILTIN_QUEUEEXIT,
    BUILTIN_QUALIFY,
} BuiltinCodes;



/******************************************************************************/
/* various expression terminator sets                                         */
/******************************************************************************/
#define   TERM_EOC     0x00000001u     /* terminate on end of clause        */
#define   TERM_RIGHT   0x00000002u     /* terminate on left paren           */
#define   TERM_SQRIGHT 0x00000004u     /* terminate on left square bracket  */
#define   TERM_TO      0x00000008u     /* terminate on TO keyword           */
#define   TERM_BY      0x00000010u     /* terminate on BY keyword           */
#define   TERM_FOR     0x00000020u     /* terminate on FOR keyword          */
#define   TERM_WHILE   0x00000040u     /* terminate on WHILE/UNTIL keywords */
#define   TERM_COMMA   0x00000080u     /* terminate on comma                */
#define   TERM_WITH    0x00000100u     /* terminate on WITH keyword         */
#define   TERM_THEN    0x00000200u     /* terminate on THEN keyword         */
#define   TERM_KEYWORD 0x10000000u     /* perform keyword terminator checks */
                                       /* terminate on DO keywords          */
#define   TERM_CONTROL (TERM_KEYWORD | TERM_TO | TERM_BY | TERM_FOR | TERM_WHILE | TERM_EOC)
                                       /* terminate on DO conditionals      */
#define   TERM_COND    (TERM_KEYWORD | TERM_WHILE | TERM_EOC)

#define   TERM_IF      (TERM_KEYWORD | TERM_THEN | TERM_EOC)

const size_t TRACE_ALL           = 'A';
const size_t TRACE_COMMANDS      = 'C';
const size_t TRACE_LABELS        = 'L';
const size_t TRACE_NORMAL        = 'N';
const size_t TRACE_FAILURES      = 'F';
const size_t TRACE_ERRORS        = 'E';
const size_t TRACE_RESULTS       = 'R';
const size_t TRACE_INTERMEDIATES = 'I';
const size_t TRACE_OFF           = 'O';
const size_t TRACE_IGNORE        = '0';

// a mask for accessing just the setting information
const size_t TRACE_SETTING_MASK  = 0xff;

/******************************************************************************/
/* Constants used for setting trace interactive debug.  These get merged      */
/* in with the setting value, so they must be > 256                           */
/******************************************************************************/
const int DEBUG_IGNORE      =  0x0000;
const int DEBUG_ON          =  0x0100;
const int DEBUG_OFF         =  0x0200;
const int DEBUG_TOGGLE      =  0x0400;
const int DEBUG_NOTRACE     =  0x0800;

// the mask for accessing just the debug flags
const size_t TRACE_DEBUG_MASK  = 0xff00;

// an invalid 8-bit character marker.
const unsigned int INVALID_CHARACTER = '00000100';

                                       /* handy defines to easy coding      */
#define new_instruction(name, type) this->sourceNewObject(sizeof(RexxInstruction##type), The##type##InstructionBehaviour, KEYWORD_##name)
#define new_variable_instruction(name, type, size) this->sourceNewObject(size, The##type##InstructionBehaviour, KEYWORD_##name)

// context flag values.
typedef enum {interpret, reclaimed, noClase} ParsingFlags;

                                       /* builtin function prototype        */
typedef RexxObject *builtin_func(RexxActivation *, size_t, RexxExpressionStack *);
typedef builtin_func *pbuiltin;        /* pointer to a builtin function     */

/**
 * A class for intializing the keyword tables.
 */
class KeywordEntry {
public:
    inline KeywordEntry(const char *n, int code)
    {
        name = n;
        length = strlen(name);
        keyword_code = code;
    }

    const char *name;                  /* keyword name                      */
    size_t length;                     /* keyword name length               */
    int    keyword_code;               /* translated keyword code           */
};

class LanguageParser: public RexxInternalObject {
 friend class RexxActivation;
 public:
    void        *operator new(size_t);
    inline void *operator new(size_t size, void *ptr) {return ptr;}
    inline void  operator delete(void *) { ; }
    inline void  operator delete(void *, void *) { ; }
    void        initBuffered(RexxBuffer *);
    void        initFile();
    void        extractNameInformation();
    bool        reconnect();
    void        setReconnect();
    void        setBufferedSource(RexxBuffer *newSource) { this->initBuffered(newSource); }
    void        interpretLine(size_t);
    void        comment();
    void        needVariable(RexxToken *);
    void        needVariableOrDotSymbol(RexxToken *);
    bool        terminator(int, RexxObject *);
    static int  resolveKeyword(RexxString *token, KeywordEntry *Table, int Table_Size);
    static int  subKeyword(RexxToken *);
    static int  keyword(RexxToken *);
    static int  builtin(RexxToken *);
    static size_t resolveBuiltin(RexxString *);
    static int  condition(RexxToken *);
    static int  parseOption(RexxToken *);
    static int  keyDirective(RexxToken *);
    static int  subDirective(RexxToken *);
    void        nextLine();
    void        position(size_t, size_t);
    void        live(size_t);
    void        liveGeneral(int reason);
    size_t      sourceSize();
    RexxString *get(size_t);
    void        nextClause();
    RexxToken  *sourceNextToken(RexxToken *);
    RexxString *traceBack(RexxActivation *, SourceLocation &, size_t, bool);
    RexxString *extract(SourceLocation &);
    RexxArray  *extractSource(SourceLocation &);
    RexxArray  *extractSource();
    void        getLocation(SourceLocation &);
    void        startLocation(SourceLocation &);
    void        endLocation(SourceLocation &);
    bool        nextSpecial(unsigned int, SourceLocation &);
    unsigned int locateToken(bool);
    void        globalSetup();
    RexxString *packLiteral(size_t, size_t, int);
    RexxCode   *generateCode(bool isMethod);
    RexxCode   *interpretMethod(RexxDirectory *);
    RexxCode   *interpret(RexxString *, RexxDirectory *, size_t);
    void        checkDirective(int errorCode);
    bool        hasBody();
    RexxObject *toss(RexxObject *);
    void        cleanup();
    void        mergeRequired(RexxSource *);
    PackageClass *loadRequires(RexxActivity *activity, RexxString *target);
    PackageClass *loadRequires(RexxActivity *activity, RexxString *target, RexxArray *s);
    void        addPackage(PackageClass *package);
    PackageClass *getPackage();
    void        inheritSourceContext(RexxSource *source);
    RoutineClass *findRoutine(RexxString *);
    RoutineClass *findLocalRoutine(RexxString *);
    RoutineClass *findPublicRoutine(RexxString *);
    RexxClass  *findClass(RexxString *);
    RexxClass  *findInstalledClass(RexxString *name);
    RexxClass  *findPublicClass(RexxString *name);
    RexxString *resolveProgramName(RexxActivity *activity, RexxString *name);
    void        processInstall(RexxActivation *);
    void        install();
    RexxCode   *translate(RexxDirectory *);
    void        resolveDependencies();
    void        directive();
    void        routineDirective();
    void        requiresDirective();
    void        libraryDirective(RexxString *name, RexxToken *token);
    void        methodDirective();
    void        classDirective();
    void        attributeDirective();
    void        constantDirective();
    void        optionsDirective();
    void        decodeExternalMethod(RexxString *methodName, RexxString *externalSpec, RexxString *&library, RexxString *&procedure);
    RexxMethod *createNativeMethod(RexxString *name, RexxString *library, RexxString *procedure);
    void        createMethod(RexxString *name, bool classMethod, bool privateMethod, bool protectedMethod, bool guardedMethod);
    void        createAttributeGetterMethod(RexxString *name, RexxVariableBase *retriever, bool classMethod, bool privateMethod, bool protectedMethod, bool guardedMethod);
    void        createAttributeSetterMethod(RexxString *name, RexxVariableBase *retriever, bool classMethod, bool privateMethod, bool protectedMethod, bool guardedMethod);
    void        createConstantGetterMethod(RexxString *name, RexxObject *value);
    void        createAbstractMethod(RexxString *name, bool classMethod, bool privateMethod, bool protectedMethod, bool guardedMethod);
    void        checkDuplicateMethod(RexxString *name, bool classMethod, int errorMsg);
    void        addMethod(RexxString *name, RexxMethod *method, bool classMethod);
    void        flushControl(RexxInstruction *);
    RexxCode   *translateBlock(RexxDirectory *);
    RexxInstruction *instruction();
    RexxVariableBase *addVariable(RexxString *);
    RexxStemVariable  *addStem(RexxString *);
    RexxCompoundVariable *addCompound(RexxString *);
    void        expose(RexxString *);
    RexxString *commonString(RexxString *);
    RexxObject *addText(RexxToken *);
    RexxObject *addVariable(RexxToken *);
    void        addClause(RexxInstruction *);
    void        addLabel(RexxInstruction *, RexxString *);
    RexxInstruction *findLabel(RexxString *);
    void        setGuard();
    RexxArray  *getGuard();
    void        addBlock(void);
    RexxVariableBase *getRetriever(RexxString *);
    RexxObject *constantExpression();
    RexxObject *constantLogicalExpression();
    RexxObject *parenExpression(RexxToken *);
    RexxObject *expression(int);
    RexxObject *subExpression(int);
    size_t      argList(RexxToken *, int);
    RexxArray  *argArray(RexxToken *, int);
    RexxObject *function(RexxToken *, RexxToken *, int);
    RexxObject *collectionMessage(RexxToken *, RexxObject *, int);
    RexxToken  *getToken(int, int);
    RexxObject *message(RexxObject *, bool, int);
    RexxObject *messageTerm();
    RexxObject *variableOrMessageTerm();
    RexxObject *messageSubterm(int);
    RexxObject *subTerm(int);
    void        pushTerm(RexxObject *);
    RexxObject *popTerm();
    RexxObject *popNTerms(size_t);
    void        isExposeValid();
    RexxArray  *words(RexxString *);
    void        errorCleanup();
    void        error(int);
    void        error(int, RexxObject *);
    void        error(int, RexxObject *, RexxObject *);
    void        error(int, RexxObject *, RexxObject *, RexxObject *);
    void        error(int errorcode, SourceLocation &location, RexxArray *subs);
    void        errorLine(int, RexxInstruction *);
    void        errorPosition(int, RexxToken *);
    void        errorToken(int, RexxToken *);
    void        blockError(RexxInstruction *);
    RexxInstruction *sourceNewObject(size_t, RexxBehaviour *, int);
    static bool parseTraceSetting(RexxString *, size_t &, size_t &, char &);
    static RexxString *formatTraceSetting(size_t source);
    size_t      processVariableList(int);
    RexxObject *parseConditional(int *, int);
    RexxObject *parseLogical(RexxToken *first, int terminators);

    bool        terminator(int, RexxToken *);
    bool        isTraceable();
    inline bool isInterpret() { return (flags & _interpret) != 0; }

    inline bool        needsInstallation() { return (this->flags&_install) != 0; }
    inline void        install(RexxActivation *activation) { if (needsInstallation()) this->processInstall(activation); };
    inline void        addReference(RexxObject *reference) { calls->addLast(reference); }
    inline void        pushDo(RexxInstruction *i) { control->pushRexx((RexxObject *)i); }
    inline RexxInstruction *popDo() { return (RexxInstruction *)(control->pullRexx()); };
    inline RexxInstruction *topDo() { return (RexxInstruction *)(control->peek()); };
           void        setProgramName(RexxString *name);
    inline void        pushOperator(RexxToken *operatorToken) { operators->pushRexx((RexxObject *)operatorToken); };
    inline RexxToken  *popOperator() { return (RexxToken *)(operators->pullRexx()); };
    inline RexxToken  *topOperator() { return (RexxToken *)(operators->peek()); };
    inline void        reclaimClause()  { flags.set(reclaimed); };
    inline bool        atEnd(void) { return (!(flags.test(reclaimed)) && !moreLines); };

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

    inline RexxToken  *nextToken() { return clause->next(); }
    inline RexxToken  *nextReal() { return clause->nextRealToken(); }
    inline void        previousToken() { clause->previous(); }
    inline void        firstToken() { clause->firstToken(); }
    inline void        trimClause() { clause->trim(); }
    inline size_t      markPosition() { return clause->mark(); }
    inline void        resetPosition(size_t p) { clause->reset(p); }
    inline void        syntaxError(int errorcode, RexxInstruction *i) { this->errorLine(errorcode, i); }
    inline void        blockSyntaxError(RexxInstruction *i) { this->blockError(i); }
    inline void        syntaxErrorAt(int errorcode, RexxToken *token) { this->errorPosition(errorcode, token); }
    inline void        syntaxError(int errorcode, RexxObject *a1) { this->error(errorcode, a1); }
    inline void        syntaxError(int errorcode, RexxObject *a1, RexxObject *a2) { this->error(errorcode, a1, a2); }
    inline void        syntaxError(int errorcode, RexxObject *a1, RexxObject *a2, RexxObject *a3) { this->error(errorcode, a1, a2, a3); }
    inline void        syntaxError(int errorcode, RexxToken *token) { this->errorToken(errorcode, token); }
    inline void        syntaxError(int errorcode) { this->error(errorcode); }
    inline bool        isInternalCode() { return this->isOldSpace(); }
    StackFrameClass *createStackFrame();

    RexxInstruction *addressNew();
    RexxInstruction *assignmentNew(RexxToken *);
    RexxInstruction *assignmentOpNew(RexxToken *, RexxToken *);
    RexxInstruction *callNew();
    RexxInstruction *commandNew();
    RexxInstruction *doNew();
    RexxInstruction *loopNew();
    RexxInstruction *createLoop();
    RexxInstruction *createDoLoop();
    RexxInstruction *createDoLoop(RexxInstructionDo *, bool);
    RexxInstruction *dropNew();
    RexxInstruction *elseNew(RexxToken *);
    RexxInstruction *endNew();
    RexxInstruction *endIfNew(RexxInstructionIf *);
    RexxInstruction *exitNew();
    RexxInstruction *exposeNew();
    RexxInstruction *forwardNew();
    void        RexxInstructionForwardCreate(RexxInstructionForward *);
    RexxInstruction *guardNew();
    RexxInstruction *ifNew(int);
    RexxInstruction *instructionNew(int);
    RexxInstruction *interpretNew();
    RexxInstruction *labelNew();
    RexxInstruction *leaveNew(int);
    RexxInstruction *messageNew(RexxExpressionMessage *);
    RexxInstruction *messageAssignmentNew(RexxExpressionMessage *, RexxObject *);
    RexxInstruction *messageAssignmentOpNew(RexxExpressionMessage *, RexxToken *, RexxObject *);
    RexxInstruction *nopNew();
    RexxInstruction *numericNew();
    RexxInstruction *optionsNew();
    RexxInstruction *otherwiseNew(RexxToken *);
    RexxInstruction *parseNew(int);
    RexxInstruction *procedureNew();
    RexxInstruction *queueNew(int);
    RexxInstruction *raiseNew();
    RexxInstruction *replyNew();
    RexxInstruction *returnNew();
    RexxInstruction *sayNew();
    RexxInstruction *selectNew();
    RexxInstruction *signalNew();
    RexxInstruction *thenNew(RexxToken *, RexxInstructionIf *);
    RexxInstruction *traceNew();
    RexxInstruction *useNew();
    void        holdObject(RexxObject *object) { this->holdstack->push(object);};
    void        saveObject(RexxObject *object) { this->savelist->put(object, object); };
    void        removeObj(RexxObject *object) { if (object != OREF_NULL) this->savelist->remove(object); };
    void        setSecurityManager(RexxObject *manager) { OrefSet(this, this->securityManager, new SecurityManager(manager)); }
    SecurityManager *getSecurityManager() { return securityManager; }

    inline RexxDirectory *getLocalRoutines() { return routines; }
    inline RexxDirectory *getPublicRoutines() { return public_routines; }
    inline void setLocalRoutines(RexxDirectory *r) { routines = r; }
    inline void setPublicRoutines(RexxDirectory *r) { public_routines = r; }

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

    void addInstalledClass(RexxString *name, RexxClass *classObject, bool publicClass);
    void addInstalledRoutine(RexxString *name, RoutineClass *routineObject, bool publicRoutine);

    RexxDirectory *getInstalledClasses() { install(); return installed_classes; }
    RexxDirectory *getInstalledPublicClasses() { install(); return installed_public_classes; }
    RexxDirectory *getImportedClasses() { install(); return merged_public_classes; }
    RexxDirectory *getInstalledRoutines() { install(); return routines; }
    RexxDirectory *getInstalledPublicRoutines() { install(); return public_routines; }
    RexxDirectory *getImportedRoutines() { install(); return merged_public_routines; }
    RexxDirectory *getDefinedMethods() { install(); return methods; }
    RexxList      *getPackages() { install(); return loadedPackages; }
    size_t         getDigits() { return digits; }
    bool           getForm() { return form; }
    size_t         getFuzz() { return fuzz; }
    size_t         getTraceSetting() { return traceSetting; }
    size_t         getTraceFlags() { return traceFlags; }
    RexxString    *getTrace() { return formatTraceSetting(traceSetting); }

    static pbuiltin builtinTable[];      /* table of builtin function stubs   */

    static const size_t TRACE_ALL           = 'A';
    static const size_t TRACE_COMMANDS      = 'C';
    static const size_t TRACE_LABELS        = 'L';
    static const size_t TRACE_NORMAL        = 'N';
    static const size_t TRACE_FAILURES      = 'F';
    static const size_t TRACE_ERRORS        = 'E';
    static const size_t TRACE_RESULTS       = 'R';
    static const size_t TRACE_INTERMEDIATES = 'I';
    static const size_t TRACE_OFF           = 'O';
    static const size_t TRACE_IGNORE        = '0';

    static const size_t DEFAULT_TRACE_SETTING = TRACE_NORMAL;

// a mask for accessing just the setting information
    static const size_t TRACE_SETTING_MASK  = 0xff;

/******************************************************************************/
/*     static constants used for setting trace interactive debug.  These get merged      */
/* in with the setting value, so they must be > 256                           */
/******************************************************************************/
    static const size_t DEBUG_IGNORE      =  0x0000;
    static const size_t DEBUG_ON          =  0x0100;
    static const size_t DEBUG_OFF         =  0x0200;
    static const size_t DEBUG_TOGGLE      =  0x0400;
    static const size_t DEBUG_NOTRACE     =  0x0800;

// the mask for accessing just the debug flags
    static const size_t TRACE_DEBUG_MASK  = 0xff00;

protected:

    RexxSource *package;                 // the source package we're translating
    ProgramSource *source;               // the source we're translating.
    FlagSet<ParsingFlags, 32> flags;     // a set of flags with parse state
    const char *current;                 // current working line
    size_t currentLength;                // length of current line
    RexxClause *clause;                  // current clause being created
    SourceLocation clauseLocation;       // current clause location for errors
    size_t lineNumber;                   // current line position
    size_t lineOffset;                   // current offset with in the line
    size_t interpretAdjust;              // INTERPRET adjustment TODO:  might not need this in the parser.

    RexxIdentityTable *saveList;         /* saved objects                     */
    RexxStack       *holdStack;          /* stack for holding temporaries     */
    RexxDirectory   *literals;           /* root of associated literal list   */
    RexxDirectory   *strings;            /* common pool of created strings    */
    RexxQueue       *control;            /* queue of control structures       */
    RexxQueue       *terms;              /* stack of expression terms         */
    RexxQueue       *subTerms;           /* stack for arguments lists, et al. */
    RexxQueue       *operators;          /* stack of expression terms         */
    ClassDirective  *activeClass;        /* currently active ::CLASS directive*/

                                         /* start of block parsing section    */

    RexxInstruction *first;              /* first instruction of parse tree   */
    RexxInstruction *last;               /* last instruction of parse tree    */
    RexxInstruction *currentInstruction; /* current "protected" instruction   */
    RexxDirectory   *variables;          /* root of associated variable list  */
    RexxDirectory   *labels;             /* root of associated label list     */
    RexxIdentityTable *guardVariables;   /* exposed variables in guard list   */
    RexxDirectory   *exposedVariables;   /* root of exposed variables list    */
    RexxList        *calls;              /* root of call list                 */
    size_t           currentStack;       /* current expression stack depth    */
    size_t           maxStack;           /* maximum stack depth               */
    size_t           variableIndex;      /* current variable index slot       */



    // Tables of different keywords using in various contexts.
    static KeywordEntry directives[];
    static KeywordEntry keywordInstructions[];
    static KeywordEntry subKeywords[];
    static KeywordEntry builtinFunctions[];
    static KeywordEntry conditionKeywords[];
    static KeywordEntry parseOptions[];
    static KeywordEntry subDirectives[];

    // table of character values
    static int characterTable[];
};
#endif

