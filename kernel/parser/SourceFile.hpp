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
/* REXX Kernel                                                  SourceFile.hpp  */
/*                                                                            */
/* Translater Source Class Definitions                                        */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxSource
#define Included_RexxSource

#include "ListClass.hpp"
#include "QueueClass.hpp"
#include "StackClass.hpp"
#include "Token.hpp"
#include "Clause.hpp"

                                       /* handy defines to easy coding      */
#define new_instruction(name, type) this->sourceNewObject(sizeof(RexxInstruction##type), The##type##InstructionBehaviour, KEYWORD_##name)
#define new_variable_instruction(name, type, size) this->sourceNewObject(size, The##type##InstructionBehaviour, KEYWORD_##name)

#define requires_allowed 0x00000001    /* ::REQUIRES directives still valid */
#define _interpret       0x00000002    /* this is interpret translation     */
#define _install         0x00000004    /* installation stuff required       */
#define reclaimed        0x00000008    /* last clause only partially used   */
#define DBCS_scanning    0x00000010    /* need to scan in DBCS mode         */
#define reclaim_possible 0x00000020    /* can re-establish source connect   */
#define no_clause        0x00000040    /* last clause of a block reached    */

class RexxSource : public RexxInternalObject {
 public:
  void       *operator new(size_t);
  inline void       *operator new(size_t size, void *ptr) {return ptr;};
  RexxSource(RexxString *, RexxArray *);
  inline RexxSource(RESTORETYPE restoreType) { ; };
  void        initBuffered(RexxObject *);
  void        initFile();
  BOOL        reconnect();
  void        setReconnect();
  void        setBufferedSource(RexxBuffer *newSource) { this->initBuffered((RexxObject *)newSource); discard(this);}
  void        interpretLine(size_t);
  void        comment();
  void        DBCScomment();
  void        needVariable(RexxToken *);
  void        needVariableOrDotSymbol(RexxToken *);
  BOOL        terminator(int, RexxObject *);
  int         subKeyword(RexxToken *);
  int         keyword(RexxToken *);
  int         builtin(RexxToken *);
  int         resolveBuiltin(RexxString *);
  int         condition(RexxToken *);
  int         parseOption(RexxToken *);
  int         keyDirective(RexxToken *);
  int         subDirective(RexxToken *);
  int         precedence(RexxToken *);
  void        nextLine();
  void        position(size_t, size_t);
  void        live();
  void        liveGeneral();
  void        flatten(RexxEnvelope *);
  size_t      sourceSize();
  RexxString *get(size_t);
  void        nextClause();
  RexxToken  *sourceNextToken(RexxToken *);
  RexxString *traceBack(PLOCATIONINFO, size_t, BOOL);
  RexxString *extract(PLOCATIONINFO);
  RexxArray  *extractSource(PLOCATIONINFO);
  void        startLocation(PLOCATIONINFO);
  void        endLocation(PLOCATIONINFO);
  BOOL        nextSpecial(UINT, PLOCATIONINFO);
  UINT        locateToken(RexxToken *);
  void        globalSetup();
  RexxString *packLiteral(INT, INT, INT);
  RexxMethod *method();
  RexxMethod *interpretMethod(RexxDirectory *);
  RexxMethod *interpret(RexxString *, RexxDirectory *, size_t);
  void        checkDirective();
  bool        hasBody();
  RexxObject *toss(RexxObject *);
  void        cleanup();
  void        mergeRequired(RexxSource *);
  RexxMethod *resolveRoutine(RexxString *);
  RexxClass  *resolveClass(RexxString *, RexxActivation *);
  void        processInstall(RexxActivation *);
  RexxMethod *translate(RexxDirectory *);
  void        resolveDependencies();
  void        completeClass();
  void        directive();
  void        routineDirective();
  void        requiresDirective();
  void        methodDirective();
  void        classDirective();
  void        attributeDirective();
  void        createMethod(RexxDirectory *target, RexxString *name, bool privateMethod, bool protectedMethod, bool guardedMethod);
  void        createAttributeGetterMethod(RexxDirectory *target, RexxString *name, RexxVariableBase *retriever, bool privateMethod, bool protectedMethod, bool guardedMethod);
  void        createAttributeSetterMethod(RexxDirectory *target, RexxString *name, RexxVariableBase *retriever, bool privateMethod, bool protectedMethod, bool guardedMethod);
  void        flushControl(RexxInstruction *);
  RexxMethod *translateBlock(RexxDirectory *);
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
  RexxObject *message(RexxObject *, int, int);
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
  RexxSource *classNewBuffered(RexxString *, RexxBuffer *);
  RexxSource *classNewFile(RexxString *);
  RexxObject *sourceNewObject(size_t, RexxBehaviour *, INT);
  void        parseTraceSetting(RexxString *, PINT, PINT);
  size_t      processVariableList(INT);
  RexxObject *parseConditional(PINT, INT);
  RexxObject *parseLogical(RexxToken *first, int terminators);

  BOOL        terminator(INT, RexxToken *);
  BOOL        traceable(void);

  inline void        install(RexxActivation *activation) { if (this->flags&_install) this->processInstall(activation); };
  inline void        addReference(RexxObject *reference) { this->calls->addLast(reference); }
  inline void        pushDo(RexxInstruction *instruction) { this->control->pushRexx((RexxObject *)instruction); }
  inline RexxInstruction *popDo() { return (RexxInstruction *)(this->control->pullRexx()); };
  inline RexxInstruction *topDo() { return (RexxInstruction *)(this->control->peek()); };
  inline void        setProgramName(RexxString *name) { OrefSet(this, this->programName, name); };
  inline RexxString *getProgramName() { return this->programName; }
  inline RexxDirectory *getMethods() { return this->methods; };
  inline void        pushOperator(RexxToken *operatorToken) { this->operators->pushRexx((RexxObject *)operatorToken); };
  inline RexxToken  *popOperator() { return (RexxToken *)(this->operators->pullRexx()); };
  inline RexxToken  *topOperator() { return (RexxToken *)(this->operators->peek()); };
  inline void        reclaimClause()  { this->flags |= reclaimed; };
  inline BOOL        atEnd(void) { return (!(this->flags&reclaimed) && (this->line_number > (this->line_count))); };

  inline RexxToken  *nextToken() { return clause->next(); }
  inline RexxToken  *nextReal() { return clause->nextRealToken(); }
  inline void        previousToken() { clause->previous(); }
  inline void        firstToken() { clause->firstToken(); }
  inline void        trimClause() { clause->trim(); }
  inline size_t      markPosition() { return clause->mark(); }
  inline void        resetPosition(size_t p) { clause->reset(p); }
  inline void        report_error_line(int errorcode, RexxInstruction *instruction) { this->errorLine(errorcode, instruction); }
  inline void        report_error_block(RexxInstruction *instruction) { this->blockError(instruction); }
  inline void        report_error_position(int errorcode, RexxToken *token) { this->errorPosition(errorcode, token); }
  inline void        report_error1(int errorcode, RexxObject *a1) { this->error(errorcode, a1); }
  inline void        report_error2(int errorcode, RexxObject *a1, RexxObject *a2) { this->error(errorcode, a1, a2); }
  inline void        report_error3(int errorcode, RexxObject *a1, RexxObject *a2, RexxObject *a3) { this->error(errorcode, a1, a2, a3); }
  inline void        report_error_token(int errorcode, RexxToken *token) { this->errorToken(errorcode, token); }
  inline void        report_error(int errorcode) { this->error(errorcode); }

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
  RexxInstruction *ifNew(INT);
  RexxInstruction *instructionNew(int);
  RexxInstruction *interpretNew();
  RexxInstruction *labelNew();
  RexxInstruction *leaveNew(INT);
  RexxInstruction *messageNew(RexxExpressionMessage *);
  RexxInstruction *messageAssignmentNew(RexxExpressionMessage *, RexxObject *);
  RexxInstruction *messageAssignmentOpNew(RexxExpressionMessage *, RexxToken *, RexxObject *);
  RexxInstruction *nopNew();
  RexxInstruction *numericNew();
  RexxInstruction *optionsNew();
  RexxInstruction *otherwiseNew(RexxToken *);
  RexxInstruction *parseNew(INT);
  RexxInstruction *procedureNew();
  RexxInstruction *queueNew(INT);
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
  void        setSecurityManager(RexxObject *manager) { OrefSet(this, this->securityManager, manager); }

  LONG  flags;                         /* parsing flags                     */
  RexxArray  *sourceArray;             /* source lines for this code        */
  RexxString *programName;             /* name of the source program        */
  RexxObject *securityManager;         /* source execution time security    */
  PCHAR current;                       /* current working line              */
  RexxClause *clause;                  /* current clause being created      */
  RexxBuffer *sourceBuffer;            /* contiguous buffered source        */
  RexxBuffer *sourceIndices;           /* line locations within buffer      */
  size_t current_length;               /* length of current line            */
  size_t line_count;                   /* size of source array              */
  size_t line_number;                  /* current line position             */
  size_t line_offset;                  /* current offset with in the line   */
  size_t interpret_adjust;             /* INTERPRET adjustment              */

                                       /* start of directives section       */

  RexxDirectory *routines;             /* routines found on directives      */
  RexxDirectory *public_routines;      /* PUBLIC routines directive routines*/
  RexxArray     *requires;             /* requires directives               */
  RexxArray     *classes;              /* classes found on directives       */
                                       /* all public installed classes      */
  RexxDirectory *installed_public_classes;
  RexxDirectory *installed_classes;    /* entire list of installed classes  */
  RexxDirectory *merged_public_classes;/* entire merged set of classes      */
                                       /* all public required routines      */
  RexxDirectory *merged_public_routines;
  RexxDirectory *methods;              /* methods found on directives       */
                                       /* start of global parsing section   */

  RexxObjectTable *savelist;           /* saved objects                     */
  RexxStack       *holdstack;          /* stack for holding temporaries     */
  RexxDirectory   *literals;           /* root of associated literal list   */
  RexxDirectory   *strings;            /* common pool of created strings    */
  RexxQueue       *control;            /* queue of control structures       */
  RexxQueue       *terms;              /* stack of expression terms         */
  RexxQueue       *subTerms;           /* stack for arguments lists, et al. */
  RexxQueue       *operators;          /* stack of expression terms         */
  RexxDirectory   *class_dependencies; /* dependencies between classes      */
  RexxArray       *active_class;       /* currently active ::CLASS directive*/

                                       /* start of block parsing section    */

  RexxInstruction *first;              /* first instruction of parse tree   */
  RexxInstruction *last;               /* last instruction of parse tree    */
  RexxInstruction *currentInstruction; /* current "protected" instruction   */
  RexxDirectory   *variables;          /* root of associated variable list  */
  RexxDirectory   *labels;             /* root of associated label list     */
  RexxObjectTable *guard_variables;    /* exposed variables in guard list   */
  RexxDirectory   *exposed_variables;  /* root of exposed variables list    */
  RexxList        *calls;              /* root of call list                 */
  size_t           currentstack;       /* current expression stack depth    */
  size_t           maxstack;           /* maximum stack depth               */
  size_t           variableindex;      /* current variable index slot       */
};
#endif
