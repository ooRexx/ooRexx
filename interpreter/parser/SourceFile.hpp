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
/* REXX Kernel                                                SourceFile.hpp  */
/*                                                                            */
/* Translater Source Class Definitions                                        */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxSource
#define Included_RexxSource

#include "SourceLocation.hpp"
#include "ListClass.hpp"
#include "QueueClass.hpp"
#include "StackClass.hpp"
#include "Token.hpp"
#include "Clause.hpp"
#include "SecurityManager.hpp"

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

/******************************************************************************/
/* Constants used for setting trace                                           */
/******************************************************************************/

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

                                       /* handy defines to easy coding      */
#define new_instruction(name, type) this->sourceNewObject(sizeof(RexxInstruction##type), The##type##InstructionBehaviour, KEYWORD_##name)
#define new_variable_instruction(name, type, size) this->sourceNewObject(size, The##type##InstructionBehaviour, KEYWORD_##name)

#define _interpret       0x00000002    /* this is interpret translation     */
#define _install         0x00000004    /* installation stuff required       */
#define reclaimed        0x00000008    /* last clause only partially used   */
#define reclaim_possible 0x00000020    /* can re-establish source connect   */
#define no_clause        0x00000040    /* last clause of a block reached    */

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

class RexxSource : public RexxInternalObject {
 friend class RexxActivation;
 public:
    void        *operator new(size_t);
    inline void *operator new(size_t size, void *ptr) {return ptr;}
    inline void  operator delete(void *) { ; }
    inline void  operator delete(void *, void *) { ; }
    RexxSource(RexxString *, RexxArray *);
    RexxSource(RexxString *programname, RexxBuffer *source_buffer);
    RexxSource(RexxString *programname, const char *data, size_t length);
    RexxSource(RexxString *programname);
    inline RexxSource(RESTORETYPE restoreType) { ; };
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
    static int  precedence(RexxToken *);
    void        nextLine();
    void        position(size_t, size_t);
    void        live(size_t);
    void        liveGeneral(int reason);
    void        flatten(RexxEnvelope *);
    size_t      sourceSize();
    RexxString *get(size_t);
    void        nextClause();
    RexxToken  *sourceNextToken(RexxToken *);
    RexxString *traceBack(SourceLocation &, size_t, bool);
    RexxString *extract(SourceLocation &);
    RexxArray  *extractSource(SourceLocation &);
    RexxArray  *extractSource();
    void        startLocation(SourceLocation &);
    void        endLocation(SourceLocation &);
    bool        nextSpecial(unsigned int, SourceLocation &);
    unsigned int locateToken(RexxToken *);
    void        globalSetup();
    RexxString *packLiteral(size_t, size_t, int);
    RexxCode   *generateCode(bool isMethod);
    RexxCode   *interpretMethod(RexxDirectory *);
    RexxCode   *interpret(RexxString *, RexxDirectory *, size_t);
    void        checkDirective();
    bool        hasBody();
    RexxObject *toss(RexxObject *);
    void        cleanup();
    void        mergeRequired(RexxSource *);
    PackageClass *loadRequired(RexxString *target);
    PackageClass *loadRequired(RexxString *target, RexxArray *s);
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
    inline void        addReference(RexxObject *reference) { this->calls->addLast(reference); }
    inline void        pushDo(RexxInstruction *i) { this->control->pushRexx((RexxObject *)i); }
    inline RexxInstruction *popDo() { return (RexxInstruction *)(this->control->pullRexx()); };
    inline RexxInstruction *topDo() { return (RexxInstruction *)(this->control->peek()); };
           void        setProgramName(RexxString *name);
    inline RexxString *getProgramName() { return this->programName; }
    inline RexxString *getProgramDirectory() { return this->programDirectory; }
    inline RexxString *getProgramExtension() { return this->programExtension; }
    inline RexxString *getProgramFile() { return this->programFile; }
    inline RexxDirectory *getMethods() { return this->methods; };
    inline RexxDirectory *getRoutines() { return this->routines; };
    inline void        pushOperator(RexxToken *operatorToken) { this->operators->pushRexx((RexxObject *)operatorToken); };
    inline RexxToken  *popOperator() { return (RexxToken *)(this->operators->pullRexx()); };
    inline RexxToken  *topOperator() { return (RexxToken *)(this->operators->peek()); };
    inline void        reclaimClause()  { this->flags |= reclaimed; };
    inline bool        atEnd(void) { return (!(this->flags&reclaimed) && (this->line_number > (this->line_count))); };

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

    RexxInstruction *addressNew();
    RexxInstruction *assignmentNew(RexxToken *);
    RexxInstruction *assignmentOpNew(RexxToken *, RexxToken *);
    RexxInstruction *callNew();
    RexxInstruction *commandNew();
    RexxInstruction *doNew();
    RexxInstruction *loopNew();
    RexxInstruction *createLoop();
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

    static inline bool isSymbolCharacter(char ch)
    {
        // The anding is necessary to keep characters > 0x7F from being
        // treated as negative numbers and returning bogus values.
        return characterTable[((unsigned int)ch) & 0xff] != 0;
    }

    static inline int translateChar(char ch)
    {
        // The anding is necessary to keep characters > 0x7F from being
        // treated as negative numbers and returning bogus values.
        return characterTable[((unsigned int)ch) & 0xff];
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

    size_t flags;                        /* parsing flags                     */
    RexxArray  *sourceArray;             /* source lines for this code        */
    RexxString *programName;             /* name of the source program        */
    RexxString *programDirectory;        // the directory location of the program (used for resolving calls)
    RexxString *programFile;             // just the file name of the program
    RexxString *programExtension;        // optional program extension
    SecurityManager *securityManager;    /* source execution time security    */
    const char *current;                 /* current working line              */
    RexxClause *clause;                  /* current clause being created      */
    SourceLocation clauseLocation;       // current clause location for errors
    RexxBuffer *sourceBuffer;            /* contiguous buffered source        */
    RexxBuffer *sourceIndices;           /* line locations within buffer      */
    size_t current_length;               /* length of current line            */
    size_t line_count;                   /* size of source array              */
    size_t line_number;                  /* current line position             */
    size_t line_offset;                  /* current offset with in the line   */
    size_t interpret_adjust;             /* INTERPRET adjustment              */

    RexxCode *initCode;                  // the initialization code
                                         /* start of directives section       */
    RexxList      *loadedPackages;       // packages imported by this package
    PackageClass  *package;              // our package wrapper
    RexxSource    *parentSource;         // a parent source context environment;
    RexxDirectory *routines;             /* routines found on directives      */
    RexxDirectory *public_routines;      /* PUBLIC routines directive routines*/
    RexxList      *libraries;            // packages requiring loading
    RexxList      *requires;             /* requires directives               */
    RexxList      *classes;              /* classes found on directives       */
                                         /* all public installed classes      */
    RexxDirectory *installed_public_classes;
    RexxDirectory *installed_classes;    /* entire list of installed classes  */
    RexxDirectory *merged_public_classes;/* entire merged set of classes      */
                                         /* all public required routines      */
    RexxDirectory *merged_public_routines;
    RexxDirectory *methods;              /* methods found on directives       */

    size_t digits;                       /* numeric digits setting            */
    size_t fuzz;                         /* numeric fuzz setting              */
    bool form;                           /* numeric form setting              */
    size_t traceSetting;                 // the package trace setting
    size_t traceFlags;                   // version optimized for quick setting at startup
    intptr_t reserved1;                  // some reserved values for compatible expansion
    intptr_t reserved2;                  // some reserved values for compatible expansion
    intptr_t reserved3;                  // some reserved values for compatible expansion
    intptr_t reserved4;                  // some reserved values for compatible expansion

                                         /* start of global parsing section   */

    RexxIdentityTable *savelist;           /* saved objects                     */
    RexxStack       *holdstack;          /* stack for holding temporaries     */
    RexxDirectory   *literals;           /* root of associated literal list   */
    RexxDirectory   *strings;            /* common pool of created strings    */
    RexxQueue       *control;            /* queue of control structures       */
    RexxQueue       *terms;              /* stack of expression terms         */
    RexxQueue       *subTerms;           /* stack for arguments lists, et al. */
    RexxQueue       *operators;          /* stack of expression terms         */
    RexxDirectory   *class_dependencies; /* dependencies between classes      */
    ClassDirective  *active_class;       /* currently active ::CLASS directive*/

                                         /* start of block parsing section    */

    RexxInstruction *first;              /* first instruction of parse tree   */
    RexxInstruction *last;               /* last instruction of parse tree    */
    RexxInstruction *currentInstruction; /* current "protected" instruction   */
    RexxDirectory   *variables;          /* root of associated variable list  */
    RexxDirectory   *labels;             /* root of associated label list     */
    RexxIdentityTable *guard_variables;    /* exposed variables in guard list   */
    RexxDirectory   *exposed_variables;  /* root of exposed variables list    */
    RexxList        *calls;              /* root of call list                 */
    size_t           currentstack;       /* current expression stack depth    */
    size_t           maxstack;           /* maximum stack depth               */
    size_t           variableindex;      /* current variable index slot       */



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
