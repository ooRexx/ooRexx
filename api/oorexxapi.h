/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.ibm.com/developerworks/oss/CPLv1.0.htm                          */
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
/* ooRexx Macros                                                              */
/*                                                                            */
/* Header file for ooRexx methods written in C.                               */
/*                                                                            */
/******************************************************************************/
#ifndef RexxNativeInterface_Included
#define RexxNativeInterface_Included

#include "rexx.h"
#include "oorexxerrors.h"


/******************************************************************************/
/* Interface Datatypes (used in macro expansions)                             */
/******************************************************************************/
#define REXX_ARGUMENT_TERMINATOR  0
#define REXX_VALUE_ARGLIST     2
#define REXX_VALUE_NAME        3
#define REXX_VALUE_SCOPE       4
#define REXX_VALUE_CSELF       5
#define REXX_VALUE_OSELF       6
#define REXX_VALUE_SUPER       7

// each of the following types have an optional equivalent

#define REXX_VALUE_RexxObjectPtr          11
#define REXX_VALUE_int                    12
#define REXX_VALUE_wholenumber_t          13
#define REXX_VALUE_double                 14
#define REXX_VALUE_CSTRING                15
#define REXX_VALUE_POINTER                16
#define REXX_VALUE_RexxStringObject       17
#define REXX_VALUE_stringsize_t           18
#define REXX_VALUE_float                  19
#define REXX_VALUE_int8_t                 20
#define REXX_VALUE_int16_t                21
#define REXX_VALUE_int32_t                22
#define REXX_VALUE_int64_t                23
#define REXX_VALUE___int64_t               23
#define REXX_VALUE_uint8_t                24
#define REXX_VALUE_uint16_t               25
#define REXX_VALUE_uint32_t               26
#define REXX_VALUE_uint64_t               27
#define REXX_VALUE___uint64_t              27
#define REXX_VALUE_intptr_t               28
#define REXX_VALUE_uintptr_t              29
#define REXX_VALUE___uintptr_t              29
#define REXX_VALUE_logical_t              30
#define REXX_VALUE_RexxArrayObject        31
#define REXX_VALUE_RexxStemObject         32
#define REXX_VALUE_size_t                 33
#define REXX_VALUE_ssize_t                34
#define REXX_VALUE_POINTERSTRING          35
#define REXX_VALUE_RexxClassObject        36

#define REXX_OPTIONAL_ARGUMENT                 0x8000

#define REXX_VALUE_OPTIONAL_RexxObjectPtr         (REXX_OPTIONAL_ARGUMENT | REXX_VALUE_RexxObjectPtr)
#define REXX_VALUE_OPTIONAL_int                   (REXX_OPTIONAL_ARGUMENT | REXX_VALUE_int)
#define REXX_VALUE_OPTIONAL_wholenumber_t         (REXX_OPTIONAL_ARGUMENT | REXX_VALUE_wholenumber_t)
#define REXX_VALUE_OPTIONAL_double                (REXX_OPTIONAL_ARGUMENT | REXX_VALUE_double)
#define REXX_VALUE_OPTIONAL_CSTRING               (REXX_OPTIONAL_ARGUMENT | REXX_VALUE_CSTRING)
#define REXX_VALUE_OPTIONAL_POINTER               (REXX_OPTIONAL_ARGUMENT | REXX_VALUE_POINTER)
#define REXX_VALUE_OPTIONAL_RexxStringObject      (REXX_OPTIONAL_ARGUMENT | REXX_VALUE_RexxStringObject)
#define REXX_VALUE_OPTIONAL_stringsize_t          (REXX_OPTIONAL_ARGUMENT | REXX_VALUE_stringsize_t)
#define REXX_VALUE_OPTIONAL_float                 (REXX_OPTIONAL_ARGUMENT | REXX_VALUE_float)
#define REXX_VALUE_OPTIONAL_int8_t                (REXX_OPTIONAL_ARGUMENT | REXX_VALUE_int8_t)
#define REXX_VALUE_OPTIONAL_int16_t               (REXX_OPTIONAL_ARGUMENT | REXX_VALUE_int16_t)
#define REXX_VALUE_OPTIONAL_int32_t               (REXX_OPTIONAL_ARGUMENT | REXX_VALUE_int32_t)
#define REXX_VALUE_OPTIONAL_int64_t               (REXX_OPTIONAL_ARGUMENT | REXX_VALUE_int64_t)
#define REXX_VALUE_OPTIONAL_uint8_t               (REXX_OPTIONAL_ARGUMENT | REXX_VALUE_uint8_t)
#define REXX_VALUE_OPTIONAL_uint16_t              (REXX_OPTIONAL_ARGUMENT | REXX_VALUE_uint16_t)
#define REXX_VALUE_OPTIONAL_uint32_t              (REXX_OPTIONAL_ARGUMENT | REXX_VALUE_uint32_t)
#define REXX_VALUE_OPTIONAL_uint64_t              (REXX_OPTIONAL_ARGUMENT | REXX_VALUE_uint64_t)
#define REXX_VALUE_OPTIONAL_size_t                (REXX_OPTIONAL_ARGUMENT | REXX_VALUE_size_t)
#define REXX_VALUE_OPTIONAL_ssize_t               (REXX_OPTIONAL_ARGUMENT | REXX_VALUE_ssize_t)
#define REXX_VALUE_OPTIONAL_intptr_t              (REXX_OPTIONAL_ARGUMENT | REXX_VALUE_intptr_t)
#define REXX_VALUE_OPTIONAL_uintptr_t             (REXX_OPTIONAL_ARGUMENT | REXX_VALUE_uintptr_t)
#define REXX_VALUE_OPTIONAL_logical_t             (REXX_OPTIONAL_ARGUMENT | REXX_VALUE_logical_t)
#define REXX_VALUE_OPTIONAL_RexxArrayObject       (REXX_OPTIONAL_ARGUMENT | REXX_VALUE_RexxArrayObject)
#define REXX_VALUE_OPTIONAL_RexxStemObject        (REXX_OPTIONAL_ARGUMENT | REXX_VALUE_RexxStemObject)
#define REXX_VALUE_OPTIONAL_POINTERSTRING         (REXX_OPTIONAL_ARGUMENT | REXX_VALUE_POINTERSTRING)
#define REXX_VALUE_OPTIONAL_RexxClassObject       (REXX_OPTIONAL_ARGUMENT | REXX_VALUE_RexxClassObject)

BEGIN_EXTERN_C()

// forward defininitions of the context structure types
struct RexxInstance_;
#ifdef __cplusplus
typedef RexxInstance_ RexxInstance;
#else
typedef const struct RexxInstance_ *RexxInstance;
#endif

struct RexxThreadContext_;
#ifdef __cplusplus
typedef RexxThreadContext_ RexxThreadContext;
#else
typedef const struct RexxThreadContext_ *RexxThreadContext;
#endif

struct RexxMethodContext_;
#ifdef __cplusplus
typedef RexxMethodContext_ RexxMethodContext;
#else
typedef const struct RexxMethodContext_ *RexxMethodContext;
#endif

struct RexxCallContext_;
#ifdef __cplusplus
typedef RexxCallContext_ RexxCallContext;
#else
typedef const struct RexxCallContext_ *RexxCallContext;
#endif

struct RexxExitContext_;
#ifdef __cplusplus
typedef RexxExitContext_ RexxExitContext;
#else
typedef const struct RexxExitContext_ *RexxExitContext;
#endif


/* This typedef simplifies coding of an Exit handler.                */
typedef int REXXENTRY RexxContextExitHandler(RexxExitContext *, int, int, PEXIT);



typedef struct _RexxContextExit
{
   RexxContextExitHandler *handler;    /* subcom enviro for sysexit  */
   int   sysexit_code;                 /* sysexit function code      */
}  RexxContextExit;


typedef struct _RexxRoutineEntry
{
    int   style;                     // function call style
    int   reserved1;                 // reserved for future use
    const char *name;                // name of the function
    void *entryPoint;                // resolved function entry point
    int   reserved2;                 // reserved for future use
    int   reserved3;                 // reserved for future use
} RexxRoutineEntry;

#define ROUTINE_TYPED_STYLE 1
#define ROUTINE_CLASSIC_STYLE 2

#define REXX_ROUTINE(s, n, e)   { s, 0, #n, (void *)e, 0, 0 }

#define REXX_TYPED_ROUTINE(n, e) REXX_ROUTINE(ROUTINE_TYPED_STYLE, n, e)
#define REXX_CLASSIC_ROUTINE(n, e) REXX_ROUTINE(ROUTINE_CLASSIC_STYLE, n, e)
#define REXX_LAST_ROUTINE()        { 0, 0, NULL, (void *)NULL, 0, 0 }

#define REXX_CLASSIC_ROUTINE_PROTOTYPE(name) size_t RexxEntry name(const char *, size_t, CONSTRXSTRING *, const char *, RXSTRING *)

typedef struct _RexxMethodEntry
{
    int   style;                     // function call style
    int   reserved1;                 // reserved for future use
    const char *name;                // name of the method
    void *entryPoint;                // resolved function entry point
    int   reserved2;                 // reserved for future use
    int   reserved3;                 // reserved for future use
} RexxMethodEntry;

#define METHOD_TYPED_STYLE 1

#define REXX_METHOD_ENTRY(n, e) { METHOD_TYPED_STYLE, 0, #n, (void *)e, 0, 0 }

#define REXX_METHOD(n, e) REXX_METHOD_ENTRY(n, e)
#define REXX_LAST_METHOD()  { 0, 0, NULL, (void *)NULL, 0, 0 }

#define REXX_PACKAGE_API_NO 20081030
// The interpreter version gets defined using two digits for major, minor, and revision.
#define REXX_INTERPRETER_4_0_0  0x00040000
#define REXX_CURRENT_INTERPRETER_VERSION REXX_INTERPRETER_4_0_0
#define NO_VERSION_YET NULL

#define REXX_LANGUAGE_6_03 0x00000603
#define REXX_CURRENT_LANGUAGE_LEVEL REXX_LANGUAGE_6_03

#define STANDARD_PACKAGE_HEADER sizeof(RexxPackageEntry), REXX_PACKAGE_API_NO,

#define OOREXX_GET_PACKAGE(name) \
    BEGIN_EXTERN_C()\
    RexxPackageEntry *RexxEntry RexxGetPackage(void) { return &name##_package_entry; }\
    END_EXTERN_C()


typedef void (RexxEntry *RexxPackageLoader)(RexxThreadContext *);
typedef void (RexxEntry *RexxPackageUnloader)(RexxThreadContext *);

typedef struct _RexxPackageEntry
{
    int size;                      // size of the structure...helps compatibility
    int apiVersion;                // version this was compiled with
    int requiredVersion;           // minimum required interpreter version (0 means any)
    const char *packageName;       // package identifier
    const char  *packageVersion;   // package version #
    RexxPackageLoader loader;      // the package loader
    RexxPackageUnloader unloader;  // the package unloader
    struct _RexxRoutineEntry *routines; // routines contained in this package
    struct _RexxMethodEntry *methods;   // methods contained in this package
} RexxPackageEntry;

END_EXTERN_C()

// argument existence indicator
#define ARGUMENT_EXISTS   0x01
// the argument is a "special" virtual argument derived from context
#define SPECIAL_ARGUMENT  0x02

typedef struct
{
// union containing argument values for each of the passable/returnable
// types from a method/function call.  The arguments are pass/retrieved
// using the appropriate type names, which bypasses any endian issues of
// how different sized values might be stored with a union.
    union
    {
        RexxArrayObject       value_ARGLIST;
        CSTRING               value_NAME;
        RexxObjectPtr         value_SCOPE;
        POINTER               value_CSELF;
        RexxClassObject       value_OSELF;
        RexxClassObject       value_SUPER;
        RexxObjectPtr         value_RexxObjectPtr;
        RexxClassObject       value_RexxClassObject;
        int                   value_int;
        wholenumber_t         value_wholenumber_t;
        stringsize_t          value_stringsize_t;
        logical_t             value_logical_t;
        double                value_double;
        CSTRING               value_CSTRING;
        POINTER               value_POINTER;
        RexxStringObject      value_RexxStringObject;
        float                 value_float;
        int8_t                value_int8_t;
        int16_t               value_int16_t;
        int32_t               value_int32_t;
        int64_t               value_int64_t;
        int64_t               value___int64_t;
        uint8_t               value_uint8_t;
        uint16_t              value_uint16_t;
        uint32_t              value_uint32_t;
        uint64_t              value_uint64_t;
        uint64_t              value___uint64_t;
        intptr_t              value_intptr_t;
        uintptr_t             value_uintptr_t;
        uintptr_t             value___uintptr_t;
        size_t                value_size_t;
        ssize_t               value_ssize_t;
        RexxArrayObject       value_RexxArrayObject;
        RexxStemObject        value_RexxStemObject;
        POINTER               value_POINTERSTRING;

        // following just duplicate the non-optional variations...
        // it was difficult (if not impossible) to get the
        // preprocessor to generate a mapped symbol name.
        RexxObjectPtr         value_OPTIONAL_RexxObjectPtr;
        int                   value_OPTIONAL_int;
        wholenumber_t         value_OPTIONAL_wholenumber_t;
        stringsize_t          value_OPTIONAL_stringsize_t;
        logical_t             value_OPTIONAL_logical_t;
        double                value_OPTIONAL_double;
        CSTRING               value_OPTIONAL_CSTRING;
        RexxClassObject       value_OPTIONAL_RexxClassObject;
        POINTER               value_OPTIONAL_POINTER;
        RexxStringObject      value_OPTIONAL_RexxStringObject;
        float                 value_OPTIONAL_float;
        int8_t                value_OPTIONAL_int8_t;
        int16_t               value_OPTIONAL_int16_t;
        int32_t               value_OPTIONAL_int32_t;
        int64_t               value_OPTIONAL_int64_t;
        uint8_t               value_OPTIONAL_uint8_t;
        uint16_t              value_OPTIONAL_uint16_t;
        uint32_t              value_OPTIONAL_uint32_t;
        uint64_t              value_OPTIONAL_uint64_t;
        intptr_t              value_OPTIONAL_intptr_t;
        uintptr_t             value_OPTIONAL_uintptr_t;
        ssize_t               value_OPTIONAL_ssize_t;
        size_t                value_OPTIONAL_size_t;
        RexxArrayObject       value_OPTIONAL_RexxArrayObject;
        RexxStemObject        value_OPTIONAL_RexxStemObject;
        POINTER               value_OPTIONAL_POINTERSTRING;
    } value;

    uint16_t type;            // type of the value
    uint16_t flags;           // argument flags


// these methods are only available for C++ code
#ifdef __cplusplus
    inline operator RexxObjectPtr() { return value.value_RexxObjectPtr; }
    inline void operator=(RexxObjectPtr o) { type = REXX_VALUE_RexxObjectPtr; value.value_RexxObjectPtr = o; }
    inline operator RexxStringObject() { return value.value_RexxStringObject; }
    inline void operator=(RexxStringObject o) { type = REXX_VALUE_RexxStringObject; value.value_RexxStringObject = o; }
    inline operator RexxArrayObject() { return value.value_RexxArrayObject; }
    inline void operator=(RexxArrayObject o) { type = REXX_VALUE_RexxArrayObject; value.value_RexxArrayObject = o; }
    inline operator RexxStemObject() { return value.value_RexxStemObject; }
    inline void operator=(RexxStemObject o) { type = REXX_VALUE_RexxStemObject; value.value_RexxStemObject = o; }
    inline operator CSTRING() { return value.value_CSTRING; }
    inline void operator=(CSTRING o) { type = REXX_VALUE_CSTRING; value.value_CSTRING = o; }
    inline operator POINTER() { return value.value_POINTER; }
    inline void operator=(POINTER o) { type = REXX_VALUE_POINTER; value.value_POINTER = o; }
    inline operator wholenumber_t() { return value.value_wholenumber_t; }
    inline void operator=(wholenumber_t o) { type = REXX_VALUE_wholenumber_t; value.value_wholenumber_t = o; }
    inline operator stringsize_t() { return value.value_stringsize_t; }
    inline void operator=(float o) { type = REXX_VALUE_float; value.value_float = o; }
    inline operator float() { return value.value_float; }
    inline operator double() { return value.value_double; }
    inline void operator=(double o) { type = REXX_VALUE_double; value.value_double = o; }
#endif
} ValueDescriptor;


// The initial address environment, passed as a CSTRING value.
#define INITIAL_ADDRESS_ENVIRONMENT "InitialAddress"
// Opaque user data, passed as a POINTER value.
#define APPLICATION_DATA            "ApplicationData"
// External function search path
#define EXTERNAL_CALL_PATH          "ExternalCallPath"
// list of lookup extensions to search for on external calls
#define EXTERNAL_CALL_EXTENSIONS    "ExternalCallPathExt"
// a library that will be loaded during initialization
// specified as a CSTRING name
#define LOAD_REQUIRED_LIBRARY       "LoadRequiredLibrary"
// The set of exits to use.  These are old-style exits, using registered exit names.
#define REGISTERED_EXITS            "RegisteredExits"
// The set of exits to use.  These are new-style exits, using direct exit addresses and
// the object oriented RexxExitContext calling convention.
#define DIRECT_EXITS                "DirectExits"
// The set of command envronments to use.  These use registered command handler names.
#define REGISTERED_ENVIRONMENTS     "RegisteredEnvironments"
// The set of command environments to use.  These are direct call addresses using the
// object-oriented calling convetion.
#define DIRECT_ENVIRONMENTS         "DirectEnvironments"
// register a library for an in-process package
#define REGISTER_LIBRARY            "RegisterLibrary"


/* This typedef simplifies coding of an Exit handler.                */
typedef RexxObjectPtr REXXENTRY RexxContextCommandHandler(RexxExitContext *, RexxStringObject, RexxStringObject);


typedef struct
{
   RexxContextCommandHandler *handler;    // the environment handler
   const char *name;                      // the handler name
}  RexxContextEnvironment;

typedef struct
{
   const char *registeredName;            // the environment handler
   const char *name;                      // the handler name
}  RexxRegisteredEnvironment;

typedef struct
{
   const char *registeredName;            // the package name (case sensitive)
   RexxPackageEntry *table;               // the package table associated with the package
}  RexxLibraryPackage;

typedef struct
{
    const char *optionName;          // name of the option
    ValueDescriptor option;          // a value appropriate to the option
} RexxOption;


typedef struct {
  wholenumber_t code;                // full condition code
  wholenumber_t rc;                  // return code value
  size_t           position;         // line number position
  RexxStringObject conditionName;    // name of the condition
  RexxStringObject message;          // fully filled in message
  RexxStringObject errortext;        // major error text
  RexxStringObject program;          // program name
  RexxStringObject description;      // program name
  RexxArrayObject  additional;       // additional information
} RexxCondition;

#define INSTANCE_INTERFACE_VERSION 100

typedef struct
{
    wholenumber_t interfaceVersion;    // The interface version identifier

    void        (RexxEntry *Terminate)(RexxInstance *);
    logical_t   (RexxEntry *AttachThread)(RexxInstance *, RexxThreadContext **);
    size_t      (RexxEntry *InterpreterVersion)(RexxInstance *);
    size_t      (RexxEntry *LanguageLevel)(RexxInstance *);
    void        (RexxEntry *Halt)(RexxInstance *);
    void        (RexxEntry *SetTrace)(RexxInstance *, logical_t);
} RexxInstanceInterface;

#define THREAD_INTERFACE_VERSION 100

BEGIN_EXTERN_C()

typedef struct
{
    wholenumber_t interfaceVersion;    // The interface version identifier

    void             (RexxEntry *DetachThread)(RexxThreadContext *);
    void             (RexxEntry *HaltThread)(RexxThreadContext *);
    void             (RexxEntry *SetThreadTrace)(RexxThreadContext *, logical_t);
    RexxObjectPtr    (RexxEntry *RequestGlobalReference)(RexxThreadContext *, RexxObjectPtr);
    void             (RexxEntry *ReleaseGlobalReference)(RexxThreadContext *, RexxObjectPtr);
    void             (RexxEntry *ReleaseLocalReference)(RexxThreadContext *, RexxObjectPtr);

    RexxObjectPtr  (RexxEntry *SendMessage)(RexxThreadContext *, RexxObjectPtr, CSTRING, RexxArrayObject);
    RexxObjectPtr  (RexxEntry *SendMessage0)(RexxThreadContext *, RexxObjectPtr, CSTRING);
    RexxObjectPtr  (RexxEntry *SendMessage1)(RexxThreadContext *, RexxObjectPtr, CSTRING, RexxObjectPtr);
    RexxObjectPtr  (RexxEntry *SendMessage2)(RexxThreadContext *, RexxObjectPtr, CSTRING, RexxObjectPtr, RexxObjectPtr);

    RexxDirectoryObject (RexxEntry *GetLocalEnvironment)(RexxThreadContext *);
    RexxDirectoryObject (RexxEntry *GetGlobalEnvironment)(RexxThreadContext *);

    logical_t        (RexxEntry *IsInstanceOf)(RexxThreadContext *, RexxObjectPtr, RexxClassObject);
    logical_t        (RexxEntry *IsOfType)(RexxThreadContext *, RexxObjectPtr, CSTRING);
    logical_t        (RexxEntry *HasMethod)(RexxThreadContext *, RexxObjectPtr, CSTRING);

    RexxPackageObject (RexxEntry *LoadPackage)(RexxThreadContext *, CSTRING d);
    RexxPackageObject (RexxEntry *LoadPackageFromData)(RexxThreadContext *, CSTRING n, CSTRING d, size_t l);
    logical_t         (RexxEntry *LoadLibrary)(RexxThreadContext *, CSTRING n);
    logical_t         (RexxEntry *RegisterLibrary)(RexxThreadContext *, CSTRING n, RexxPackageEntry *);
    RexxClassObject  (RexxEntry *FindClass)(RexxThreadContext *, CSTRING);
    RexxClassObject  (RexxEntry *FindPackageClass)(RexxThreadContext *, RexxPackageObject, CSTRING);
    RexxDirectoryObject (RexxEntry *GetPackageRoutines)(RexxThreadContext *, RexxPackageObject);
    RexxDirectoryObject (RexxEntry *GetPackagePublicRoutines)(RexxThreadContext *, RexxPackageObject);
    RexxDirectoryObject (RexxEntry *GetPackageClasses)(RexxThreadContext *, RexxPackageObject);
    RexxDirectoryObject (RexxEntry *GetPackagePublicClasses)(RexxThreadContext *, RexxPackageObject);
    RexxDirectoryObject (RexxEntry *GetPackageMethods)(RexxThreadContext *, RexxPackageObject);
    RexxObjectPtr    (RexxEntry *CallRoutine)(RexxThreadContext *, RexxRoutineObject, RexxArrayObject);
    RexxObjectPtr    (RexxEntry *CallProgram)(RexxThreadContext *, CSTRING, RexxArrayObject);

    RexxMethodObject (RexxEntry *NewMethod)(RexxThreadContext *, CSTRING, CSTRING, size_t);
    RexxRoutineObject (RexxEntry *NewRoutine)(RexxThreadContext *, CSTRING, CSTRING, size_t);
    logical_t         (RexxEntry *IsRoutine)(RexxThreadContext *, RexxObjectPtr);
    logical_t         (RexxEntry *IsMethod)(RexxThreadContext *, RexxObjectPtr);
    RexxPackageObject (RexxEntry *GetRoutinePackage)(RexxThreadContext *, RexxRoutineObject);
    RexxPackageObject (RexxEntry *GetMethodPackage)(RexxThreadContext *, RexxMethodObject);

    POINTER          (RexxEntry *ObjectToCSelf)(RexxThreadContext *, RexxObjectPtr);
    RexxObjectPtr    (RexxEntry *WholeNumberToObject)(RexxThreadContext *, wholenumber_t);
    RexxObjectPtr    (RexxEntry *UintptrToObject)(RexxThreadContext *, uintptr_t);
    RexxObjectPtr    (RexxEntry *IntptrToObject)(RexxThreadContext *, intptr_t);
    RexxObjectPtr    (RexxEntry *ValueToObject)(RexxThreadContext *, ValueDescriptor *);
    RexxArrayObject  (RexxEntry *ValuesToObject)(RexxThreadContext *, ValueDescriptor *, size_t count);
    logical_t        (RexxEntry *ObjectToValue)(RexxThreadContext *, RexxObjectPtr, ValueDescriptor *);
    RexxObjectPtr    (RexxEntry *StringSizeToObject)(RexxThreadContext *, stringsize_t);
    logical_t        (RexxEntry *ObjectToWholeNumber)(RexxThreadContext *, RexxObjectPtr, wholenumber_t *);
    logical_t        (RexxEntry *ObjectToStringSize)(RexxThreadContext *, RexxObjectPtr, stringsize_t *);
    RexxObjectPtr    (RexxEntry *Int64ToObject)(RexxThreadContext *, int64_t);
    RexxObjectPtr    (RexxEntry *UnsignedInt64ToObject)(RexxThreadContext *, uint64_t);
    logical_t        (RexxEntry *ObjectToInt64)(RexxThreadContext *, RexxObjectPtr, int64_t *);
    logical_t        (RexxEntry *ObjectToUnsignedInt64)(RexxThreadContext *, RexxObjectPtr, uint64_t *);
    RexxObjectPtr    (RexxEntry *Int32ToObject)(RexxThreadContext *, int32_t);
    RexxObjectPtr    (RexxEntry *UnsignedInt32ToObject)(RexxThreadContext *, uint32_t);
    logical_t        (RexxEntry *ObjectToInt32)(RexxThreadContext *, RexxObjectPtr, int32_t *);
    logical_t        (RexxEntry *ObjectToUnsignedInt32)(RexxThreadContext *, RexxObjectPtr, uint32_t *);
    logical_t        (RexxEntry *ObjectToUintptr)(RexxThreadContext *, RexxObjectPtr, uintptr_t *);
    logical_t        (RexxEntry *ObjectToIntptr)(RexxThreadContext *, RexxObjectPtr, intptr_t *);
    logical_t        (RexxEntry *ObjectToLogical)(RexxThreadContext *, RexxObjectPtr, logical_t *);
    RexxObjectPtr    (RexxEntry *LogicalToObject)(RexxThreadContext *, logical_t);
    RexxObjectPtr    (RexxEntry *DoubleToObject)(RexxThreadContext *, double);
    RexxObjectPtr    (RexxEntry *DoubleToObjectWithPrecision)(RexxThreadContext *, double, size_t precision);
    logical_t        (RexxEntry *ObjectToDouble)(RexxThreadContext *, RexxObjectPtr, double *);

    RexxStringObject  (RexxEntry *ObjectToString)(RexxThreadContext *, RexxObjectPtr);
    CSTRING (RexxEntry *ObjectToStringValue)(RexxThreadContext *, RexxObjectPtr);
    size_t  (RexxEntry *StringGet)(RexxThreadContext *, RexxStringObject, size_t, POINTER, size_t);
    size_t  (RexxEntry *StringLength)(RexxThreadContext *, RexxStringObject);
    CSTRING (RexxEntry *StringData)(RexxThreadContext *, RexxStringObject);
    RexxStringObject  (RexxEntry *NewString)(RexxThreadContext *, CSTRING, size_t);
    RexxStringObject  (RexxEntry *NewStringFromAsciiz)(RexxThreadContext *, CSTRING);
    RexxStringObject  (RexxEntry *StringUpper)(RexxThreadContext *, RexxStringObject);
    RexxStringObject  (RexxEntry *StringLower)(RexxThreadContext *, RexxStringObject);
    logical_t         (RexxEntry *IsString)(RexxThreadContext *, RexxObjectPtr);

    RexxBufferStringObject  (RexxEntry *NewBufferString)(RexxThreadContext *, size_t);
    size_t  (RexxEntry *BufferStringLength)(RexxThreadContext *, RexxBufferStringObject);
    POINTER (RexxEntry *BufferStringData)(RexxThreadContext *, RexxBufferStringObject);
    RexxStringObject  (RexxEntry *FinishBufferString)(RexxThreadContext *, RexxBufferStringObject, size_t);

    void             (RexxEntry *DirectoryPut)(RexxThreadContext *, RexxDirectoryObject, RexxObjectPtr, CSTRING);
    RexxObjectPtr    (RexxEntry *DirectoryAt)(RexxThreadContext *, RexxDirectoryObject, CSTRING);
    RexxObjectPtr    (RexxEntry *DirectoryRemove)(RexxThreadContext *, RexxDirectoryObject, CSTRING);
    RexxDirectoryObject  (RexxEntry *NewDirectory)(RexxThreadContext *);
    logical_t        (RexxEntry *IsDirectory)(RexxThreadContext *, RexxObjectPtr);

    RexxObjectPtr   (RexxEntry *ArrayAt)(RexxThreadContext *, RexxArrayObject, size_t);
    void            (RexxEntry *ArrayPut)(RexxThreadContext *, RexxArrayObject, RexxObjectPtr, size_t);
    size_t          (RexxEntry *ArrayAppend)(RexxThreadContext *, RexxArrayObject, RexxObjectPtr);
    size_t          (RexxEntry *ArrayAppendString)(RexxThreadContext *, RexxArrayObject, CSTRING, size_t);
    size_t          (RexxEntry *ArraySize)(RexxThreadContext *, RexxArrayObject);
    size_t          (RexxEntry *ArrayItems)(RexxThreadContext *, RexxArrayObject);
    size_t          (RexxEntry *ArrayDimension)(RexxThreadContext *, RexxArrayObject);
    RexxArrayObject (RexxEntry *NewArray)(RexxThreadContext *, size_t);
    RexxArrayObject (RexxEntry *ArrayOfOne)(RexxThreadContext *, RexxObjectPtr);
    RexxArrayObject (RexxEntry *ArrayOfTwo)(RexxThreadContext *, RexxObjectPtr, RexxObjectPtr);
    RexxArrayObject (RexxEntry *ArrayOfThree)(RexxThreadContext *, RexxObjectPtr, RexxObjectPtr, RexxObjectPtr);
    RexxArrayObject (RexxEntry *ArrayOfFour)(RexxThreadContext *, RexxObjectPtr, RexxObjectPtr, RexxObjectPtr, RexxObjectPtr);
    logical_t       (RexxEntry *IsArray)(RexxThreadContext *, RexxObjectPtr);

    POINTER (RexxEntry *BufferData)(RexxThreadContext *, RexxBufferObject);
    size_t            (RexxEntry *BufferLength)(RexxThreadContext *, RexxBufferObject);
    RexxBufferObject  (RexxEntry *NewBuffer)(RexxThreadContext *, size_t);
    logical_t         (RexxEntry *IsBuffer)(RexxThreadContext *, RexxObjectPtr);

    POINTER           (RexxEntry *PointerValue)(RexxThreadContext *, RexxPointerObject);
    RexxPointerObject (RexxEntry *NewPointer)(RexxThreadContext *, POINTER);
    logical_t         (RexxEntry *IsPointer)(RexxThreadContext *, RexxObjectPtr);

    RexxObjectPtr    (RexxEntry *SupplierItem)(RexxThreadContext *, RexxSupplierObject);
    RexxObjectPtr    (RexxEntry *SupplierIndex)(RexxThreadContext *, RexxSupplierObject);
    logical_t        (RexxEntry *SupplierAvailable)(RexxThreadContext *, RexxSupplierObject);
    void             (RexxEntry *SupplierNext)(RexxThreadContext *, RexxSupplierObject);
    RexxSupplierObject (RexxEntry *NewSupplier)(RexxThreadContext *, RexxArrayObject values, RexxArrayObject names);

    RexxStemObject   (RexxEntry *NewStem)(RexxThreadContext *, CSTRING);
    void             (RexxEntry *SetStemElement)(RexxThreadContext *, RexxStemObject, CSTRING, RexxObjectPtr);
    RexxObjectPtr    (RexxEntry *GetStemElement)(RexxThreadContext *, RexxStemObject, CSTRING);
    void             (RexxEntry *DropStemElement)(RexxThreadContext *, RexxStemObject, CSTRING);
    void             (RexxEntry *SetStemArrayElement)(RexxThreadContext *, RexxStemObject, size_t, RexxObjectPtr);
    RexxObjectPtr    (RexxEntry *GetStemArrayElement)(RexxThreadContext *, RexxStemObject, size_t);
    void             (RexxEntry *DropStemArrayElement)(RexxThreadContext *, RexxStemObject, size_t);
    RexxDirectoryObject (RexxEntry *GetAllStemElements)(RexxThreadContext *, RexxStemObject);
    RexxObjectPtr    (RexxEntry *GetStemValue)(RexxThreadContext *, RexxStemObject);
    logical_t        (RexxEntry *IsStem)(RexxThreadContext *, RexxObjectPtr);

    void             (RexxEntry *RaiseException0)(RexxThreadContext *, size_t);
    void             (RexxEntry *RaiseException1)(RexxThreadContext *, size_t, RexxObjectPtr);
    void             (RexxEntry *RaiseException2)(RexxThreadContext *, size_t, RexxObjectPtr, RexxObjectPtr);
    void             (RexxEntry *RaiseException)(RexxThreadContext *, size_t, RexxArrayObject);
    void             (RexxEntry *RaiseCondition)(RexxThreadContext *, CSTRING, RexxStringObject, RexxObjectPtr, RexxObjectPtr);
    logical_t        (RexxEntry *CheckCondition)(RexxThreadContext *);
    RexxDirectoryObject (RexxEntry *GetConditionInfo)(RexxThreadContext *);
    void             (RexxEntry *DecodeConditionInfo)(RexxThreadContext *, RexxDirectoryObject, RexxCondition *);
    void             (RexxEntry *ClearCondition)(RexxThreadContext *);

    RexxObjectPtr    RexxNil;
    RexxObjectPtr    RexxTrue;
    RexxObjectPtr    RexxFalse;
    RexxStringObject RexxNullString;

} RexxThreadInterface;


#define METHOD_INTERFACE_VERSION 100

typedef struct
{
    wholenumber_t interfaceVersion;    // The interface version identifier

    RexxArrayObject  (RexxEntry *GetArguments)(RexxMethodContext *);
    RexxObjectPtr    (RexxEntry *GetArgument)(RexxMethodContext *, size_t);
    CSTRING    (RexxEntry *GetMessageName)(RexxMethodContext *);
    RexxMethodObject (RexxEntry *GetMethod)(RexxMethodContext *);
    RexxObjectPtr    (RexxEntry *GetSelf)(RexxMethodContext *);
    RexxClassObject  (RexxEntry *GetSuper)(RexxMethodContext *);
    RexxObjectPtr    (RexxEntry *GetScope)(RexxMethodContext *);
    void             (RexxEntry *SetObjectVariable)(RexxMethodContext *, CSTRING, RexxObjectPtr);
    RexxObjectPtr    (RexxEntry *GetObjectVariable)(RexxMethodContext *, CSTRING);
    void             (RexxEntry *DropObjectVariable)(RexxMethodContext *, CSTRING);
    RexxObjectPtr    (RexxEntry *ForwardMessage)(RexxMethodContext *, RexxObjectPtr, CSTRING, RexxClassObject, RexxArrayObject);
    void             (RexxEntry *SetGuardOn)(RexxMethodContext *);
    void             (RexxEntry *SetGuardOff)(RexxMethodContext *);
    RexxClassObject  (RexxEntry *FindContextClass)(RexxMethodContext *, CSTRING);
} MethodContextInterface;

#define CALL_INTERFACE_VERSION 100

typedef struct
{
    wholenumber_t interfaceVersion;    // The interface version identifier

    RexxArrayObject  (RexxEntry *GetArguments)(RexxCallContext *);
    RexxObjectPtr    (RexxEntry *GetArgument)(RexxCallContext *, size_t);
    CSTRING          (RexxEntry *GetRoutineName)(RexxCallContext *);
    RexxRoutineObject (RexxEntry *GetRoutine)(RexxCallContext *);
    void             (RexxEntry *SetContextVariable)(RexxCallContext *, CSTRING, RexxObjectPtr);
    RexxObjectPtr    (RexxEntry *GetContextVariable)(RexxCallContext *, CSTRING);
    void             (RexxEntry *DropContextVariable)(RexxCallContext *, CSTRING);
    RexxDirectoryObject (RexxEntry *GetAllContextVariables)(RexxCallContext *);
    RexxStemObject   (RexxEntry *ResolveStemVariable)(RexxCallContext *, RexxObjectPtr);
    void             (RexxEntry *InvalidRoutine)(RexxCallContext *);
    stringsize_t     (RexxEntry *GetContextDigits)(RexxCallContext *);
    stringsize_t     (RexxEntry *GetContextFuzz)(RexxCallContext *);
    logical_t        (RexxEntry *GetContextForm)(RexxCallContext *);
    RexxObjectPtr    (RexxEntry *GetCallerContext)(RexxCallContext *);
    RexxClassObject  (RexxEntry *FindContextClass)(RexxCallContext *, CSTRING);
} CallContextInterface;

#define EXIT_INTERFACE_VERSION 100

typedef struct
{
    wholenumber_t interfaceVersion;    // The interface version identifier
    void             (RexxEntry *SetContextVariable)(RexxExitContext *, CSTRING, RexxObjectPtr);
    RexxObjectPtr    (RexxEntry *GetContextVariable)(RexxExitContext *, CSTRING);
    void             (RexxEntry *DropContextVariable)(RexxExitContext *, CSTRING);
    RexxDirectoryObject (RexxEntry *GetAllContextVariables)(RexxExitContext *);
    RexxObjectPtr    (RexxEntry *GetCallerContext)(RexxExitContext *);
} ExitContextInterface;

END_EXTERN_C()

struct RexxInstance_
{
    RexxInstanceInterface *functions;   // the interface function vector
    void *applicationData;              // creator defined data pointer
#ifdef __cplusplus
    void        Terminate()
    {
        functions->Terminate(this);
    }
    logical_t AttachThread(RexxThreadContext **tc)
    {
        return functions->AttachThread(this, tc);
    }
    size_t InterpreterVersion()
    {
        return functions->InterpreterVersion(this);
    }
    size_t LanguageLevel()
    {
        return functions->LanguageLevel(this);
    }
    void Halt()
    {
        functions->Halt(this);
    }
    void SetTrace(logical_t s)
    {
        functions->SetTrace(this, s);
    }
#endif
};


struct RexxThreadContext_
{
    RexxInstance *instance;             // the owning instance
    RexxThreadInterface *functions;     // the interface function vector
#ifdef __cplusplus
    POINTER GetApplicationData()
    {
        return instance->applicationData;
    }
    size_t InterpreterVersion()
    {
        return instance->InterpreterVersion();
    }
    size_t LanguageLevel()
    {
        return instance->LanguageLevel();
    }
    void DetachThread()
    {
        functions->DetachThread(this);
    }
    void HaltThread()
    {
        functions->HaltThread(this);
    }
    void SetThreadTrace(logical_t s)
    {
        functions->SetThreadTrace(this, s);
    }
    RexxObjectPtr RequestGlobalReference(RexxObjectPtr o)
    {
        return functions->RequestGlobalReference(this, o);
    }
    void ReleaseGlobalReference(RexxObjectPtr o)
    {
        functions->ReleaseGlobalReference(this, o);
    }
    void ReleaseLocalReference(RexxObjectPtr o)
    {
        functions->ReleaseLocalReference(this, o);
    }

    RexxObjectPtr SendMessage(RexxObjectPtr o, CSTRING msg, RexxArrayObject arr)
    {
        return functions->SendMessage(this, o, msg, arr);
    }
    RexxObjectPtr SendMessage0(RexxObjectPtr o, CSTRING msg)
    {
        return functions->SendMessage0(this, o, msg);
    }
    RexxObjectPtr SendMessage1(RexxObjectPtr o, CSTRING msg, RexxObjectPtr a1)
    {
        return functions->SendMessage1(this, o, msg, a1);
    }
    RexxObjectPtr SendMessage2(RexxObjectPtr o, CSTRING msg, RexxObjectPtr a1, RexxObjectPtr a2)
    {
        return functions->SendMessage2(this, o, msg, a1, a2);
    }

    RexxDirectoryObject GetLocalEnvironment()
    {
        return functions->GetLocalEnvironment(this);
    }
    RexxDirectoryObject GetGlobalEnvironment()
    {
        return functions->GetGlobalEnvironment(this);
    }
    logical_t IsInstanceOf(RexxObjectPtr o, RexxClassObject co)
    {
        return functions->IsInstanceOf(this, o, co);
    }
    logical_t IsOfType(RexxObjectPtr o, CSTRING cn)
    {
        return functions->IsOfType(this, o, cn);
    }
    RexxClassObject FindClass(CSTRING s)
    {
        return functions->FindClass(this, s);
    }
    RexxClassObject FindPackageClass(RexxPackageObject m, CSTRING n)
    {
        return functions->FindPackageClass(this, m, n);
    }
    logical_t HasMethod(RexxObjectPtr o, CSTRING m)
    {
        return functions->HasMethod(this, o, m);
    }
    RexxMethodObject NewMethod(CSTRING n, CSTRING s, size_t l)
    {
        return functions->NewMethod(this, n, s, l);
    }
    RexxRoutineObject NewRoutine(CSTRING n, CSTRING s, size_t l)
    {
        return functions->NewRoutine(this, n, s, l);
    }
    logical_t IsRoutine(RexxObjectPtr o)
    {
        return functions->IsRoutine(this, o);
    }
    logical_t IsMethod(RexxObjectPtr o)
    {
        return functions->IsMethod(this, o);
    }
    RexxPackageObject GetRoutinePackage(RexxRoutineObject o)
    {
        return functions->GetRoutinePackage(this, o);
    }
    RexxPackageObject GetMethodPackage(RexxMethodObject o)
    {
        return functions->GetMethodPackage(this, o);
    }


    RexxDirectoryObject GetPackageRoutines(RexxPackageObject m)
    {
        return functions->GetPackageRoutines(this, m);
    }
    RexxDirectoryObject GetPackagePublicRoutines(RexxPackageObject m)
    {
        return functions->GetPackagePublicRoutines(this, m);
    }
    RexxDirectoryObject GetPackageClasses(RexxPackageObject m)
    {
        return functions->GetPackageClasses(this, m);
    }
    RexxDirectoryObject GetPackagePublicClasses(RexxPackageObject m)
    {
        return functions->GetPackagePublicClasses(this, m);
    }
    RexxDirectoryObject GetPackageMethods(RexxPackageObject m)
    {
        return functions->GetPackageMethods(this, m);
    }
    RexxObjectPtr CallRoutine(RexxRoutineObject m, RexxArrayObject a)
    {
        return functions->CallRoutine(this, m, a);
    }
    RexxObjectPtr CallProgram(CSTRING n,RexxArrayObject a)
    {
        return functions->CallProgram(this, n, a);
    }
    RexxPackageObject LoadPackage(CSTRING d)
    {
        return functions->LoadPackage(this, d);
    }
    RexxPackageObject LoadPackageFromData(CSTRING n, CSTRING d, size_t l)
    {
        return functions->LoadPackageFromData(this, n, d, l);
    }
    logical_t LoadLibrary(CSTRING n)
    {
        return functions->LoadLibrary(this, n);
    }
    logical_t RegisterLibrary(CSTRING n, RexxPackageEntry *e)
    {
        return functions->RegisterLibrary(this, n, e);
    }
    POINTER ObjectToCSelf(RexxObjectPtr o)
    {
        return functions->ObjectToCSelf(this, o);
    }
    RexxObjectPtr WholeNumberToObject(wholenumber_t n)
    {
        return functions->WholeNumberToObject(this, n);
    }
    RexxObjectPtr WholeNumber(wholenumber_t n)
    {
        return functions->WholeNumberToObject(this, n);
    }
    RexxObjectPtr UintptrToObject(uintptr_t n)
    {
        return functions->UintptrToObject(this, n);
    }
    RexxObjectPtr Uintptr(uintptr_t n)
    {
        return functions->UintptrToObject(this, n);
    }
    RexxObjectPtr IntptrToObject(intptr_t n)
    {
        return functions->IntptrToObject(this, n);
    }
    RexxObjectPtr Intptr(intptr_t n)
    {
        return functions->IntptrToObject(this, n);
    }
    RexxObjectPtr StringSizeToObject(size_t n)
    {
        return functions->StringSizeToObject(this, n);
    }
    RexxObjectPtr StringSize(size_t n)
    {
        return functions->StringSizeToObject(this, n);
    }
    logical_t ObjectToWholeNumber(RexxObjectPtr o, wholenumber_t *n)
    {
        return functions->ObjectToWholeNumber(this, o, n);
    }
    logical_t WholeNumber(RexxObjectPtr o, wholenumber_t *n)
    {
        return functions->ObjectToWholeNumber(this, o, n);
    }
    logical_t ObjectToStringSize(RexxObjectPtr o, size_t *n)
    {
        return functions->ObjectToStringSize(this, o, n);
    }
    logical_t StringSize(RexxObjectPtr o, size_t *n)
    {
        return functions->ObjectToStringSize(this, o, n);
    }
    RexxObjectPtr Int64ToObject(int64_t i)
    {
        return functions->Int64ToObject(this, i);
    }
    RexxObjectPtr Int64(int64_t i)
    {
        return functions->Int64ToObject(this, i);
    }
    RexxObjectPtr UnsignedInt64ToObject(uint64_t u)
    {
        return functions->UnsignedInt64ToObject(this, u);
    }
    RexxObjectPtr UnsignedInt64(uint64_t u)
    {
        return functions->UnsignedInt64ToObject(this, u);
    }
    logical_t ObjectToInt64(RexxObjectPtr o, int64_t *i)
    {
        return functions->ObjectToInt64(this, o, i);
    }
    logical_t Int64(RexxObjectPtr o, int64_t *i)
    {
        return functions->ObjectToInt64(this, o, i);
    }
    logical_t ObjectToUnsignedInt64(RexxObjectPtr o, uint64_t *u)
    {
        return functions->ObjectToUnsignedInt64(this, o, u);
    }
    logical_t UnsignedInt64(RexxObjectPtr o, uint64_t *u)
    {
        return functions->ObjectToUnsignedInt64(this, o, u);
    }
    RexxObjectPtr Int32ToObject(int32_t i)
    {
        return functions->Int32ToObject(this, i);
    }
    RexxObjectPtr Int32(int32_t i)
    {
        return functions->Int32ToObject(this, i);
    }
    RexxObjectPtr UnsignedInt32ToObject(uint32_t u)
    {
        return functions->UnsignedInt32ToObject(this, u);
    }
    RexxObjectPtr UnsignedInt32(uint32_t u)
    {
        return functions->UnsignedInt32ToObject(this, u);
    }
    logical_t ObjectToInt32(RexxObjectPtr o, int32_t *i)
    {
        return functions->ObjectToInt32(this, o, i);
    }
    logical_t Int32(RexxObjectPtr o, int32_t *i)
    {
        return functions->ObjectToInt32(this, o, i);
    }
    logical_t ObjectToUnsignedInt32(RexxObjectPtr o, uint32_t *u)
    {
        return functions->ObjectToUnsignedInt32(this, o, u);
    }
    logical_t UnsignedInt32(RexxObjectPtr o, uint32_t *u)
    {
        return functions->ObjectToUnsignedInt32(this, o, u);
    }
    logical_t ObjectToUintptr(RexxObjectPtr o, uintptr_t *n)
    {
        return functions->ObjectToUintptr(this, o, n);
    }
    logical_t Uintptr(RexxObjectPtr o, uintptr_t *n)
    {
        return functions->ObjectToUintptr(this, o, n);
    }
    logical_t ObjectToIntptr(RexxObjectPtr o, intptr_t *n)
    {
        return functions->ObjectToIntptr(this, o, n);
    }
    logical_t Intptr(RexxObjectPtr o, intptr_t *n)
    {
        return functions->ObjectToIntptr(this, o, n);
    }
    logical_t ObjectToLogical(RexxObjectPtr o, logical_t *n)
    {
        return functions->ObjectToLogical(this, o, n);
    }
    logical_t Logical(RexxObjectPtr o, logical_t *n)
    {
        return functions->ObjectToLogical(this, o, n);
    }
    RexxObjectPtr LogicalToObject(logical_t l)
    {
        return functions->LogicalToObject(this, l);
    }
    RexxObjectPtr Logical(logical_t l)
    {
        return functions->LogicalToObject(this, l);
    }
    RexxObjectPtr DoubleToObject(double d)
    {
        return functions->DoubleToObject(this, d);
    }
    RexxObjectPtr Double(double d)
    {
        return functions->DoubleToObject(this, d);
    }
    RexxObjectPtr DoubleToObjectWithPrecision(double d, size_t precision)
    {
        return functions->DoubleToObjectWithPrecision(this, d, precision);
    }
    logical_t ObjectToDouble(RexxObjectPtr o, double *d)
    {
        return functions->ObjectToDouble(this, o, d);
    }
    logical_t Double(RexxObjectPtr o, double *d)
    {
        return functions->ObjectToDouble(this, o, d);
    }
    RexxObjectPtr ValueToObject(ValueDescriptor *v)
    {
        return functions->ValueToObject(this, v);
    }
    RexxArrayObject ValuesToObject(ValueDescriptor *v, size_t c)
    {
        return functions->ValuesToObject(this, v, c);
    }
    logical_t ObjectToValue(RexxObjectPtr o, ValueDescriptor *v)
    {
        return functions->ObjectToValue(this, o, v);
    }
    RexxStringObject ObjectToString(RexxObjectPtr o)
    {
        return functions->ObjectToString(this, o);
    }
    CSTRING ObjectToStringValue(RexxObjectPtr o)
    {
        return functions->ObjectToStringValue(this, o);
    }
    CSTRING CString(RexxObjectPtr o)
    {
        return functions->ObjectToStringValue(this, o);
    }
    size_t StringGet(RexxStringObject o, size_t len1, POINTER s, size_t len2)
    {
        return functions->StringGet(this, o, len1, s, len2);
    }
    size_t StringLength(RexxStringObject o)
    {
        return functions->StringLength(this, o);
    }
    CSTRING StringData(RexxStringObject o)
    {
        return functions->StringData(this, o);
    }
    RexxStringObject NewString(CSTRING s, size_t len)
    {
        return functions->NewString(this, s, len);
    }
    RexxStringObject NewStringFromAsciiz(CSTRING s)
    {
        return functions->NewStringFromAsciiz(this, s);
    }
    RexxStringObject String(CSTRING s, size_t len)
    {
        return functions->NewString(this, s, len);
    }
    RexxStringObject String(CSTRING s)
    {
        return functions->NewStringFromAsciiz(this, s);
    }
    RexxStringObject CString(CSTRING s)
    {
        return functions->NewStringFromAsciiz(this, s);
    }
    RexxStringObject StringUpper(RexxStringObject s)
    {
        return functions->StringUpper(this, s);
    }
    RexxStringObject StringLower(RexxStringObject s)
    {
        return functions->StringLower(this, s);
    }
    logical_t IsString(RexxObjectPtr o)
    {
        return functions->IsString(this, o);
    }

    RexxBufferStringObject NewBufferString(size_t len)
    {
        return functions->NewBufferString(this, len);
    }

    size_t BufferStringLength(RexxBufferStringObject o)
    {
        return functions->BufferStringLength(this, o);
    }

    POINTER BufferStringData(RexxBufferStringObject o)
    {
        return functions->BufferStringData(this, o);
    }

    RexxStringObject FinishBufferString(RexxBufferStringObject o, size_t l)
    {
        return functions->FinishBufferString(this, o, l);
    }

    void DirectoryPut(RexxDirectoryObject diro, RexxObjectPtr o, CSTRING s)
    {
        functions->DirectoryPut(this, diro, o, s);
    }
    RexxObjectPtr DirectoryAt(RexxDirectoryObject to, CSTRING s)
    {
        return functions->DirectoryAt(this, to, s);
    }
    RexxObjectPtr DirectoryRemove(RexxDirectoryObject to, CSTRING s)
    {
        return functions->DirectoryRemove(this, to, s);
    }
    RexxDirectoryObject NewDirectory()
    {
        return functions->NewDirectory(this);
    }
    logical_t IsDirectory(RexxObjectPtr o)
    {
        return functions->IsDirectory(this, o);
    }

    RexxObjectPtr ArrayAt(RexxArrayObject ao, size_t n)
    {
        return functions->ArrayAt(this, ao, n);
    }
    void ArrayPut(RexxArrayObject ao, RexxObjectPtr o, size_t n)
    {
        functions->ArrayPut(this, ao, o, n);
    }
    size_t ArrayAppend(RexxArrayObject ao, RexxObjectPtr o)
    {
        return functions->ArrayAppend(this, ao, o);
    }
    size_t ArrayAppendString(RexxArrayObject ao, CSTRING s, size_t l)
    {
        return functions->ArrayAppendString(this, ao, s, l);
    }
    size_t ArraySize(RexxArrayObject ao)
    {
        return functions->ArraySize(this, ao);
    }
    size_t ArrayItems(RexxArrayObject ao)
    {
        return functions->ArrayItems(this, ao);
    }
    size_t ArrayDimension(RexxArrayObject ao)
    {
        return functions->ArrayDimension(this, ao);
    }
    RexxArrayObject NewArray(size_t n)
    {
        return functions->NewArray(this, n);
    }
    RexxArrayObject ArrayOfOne(RexxObjectPtr o)
    {
        return functions->ArrayOfOne(this, o);
    }
    RexxArrayObject ArrayOfTwo(RexxObjectPtr o1, RexxObjectPtr o2)
    {
        return functions->ArrayOfTwo(this, o1, o2);
    }
    RexxArrayObject ArrayOfThree(RexxObjectPtr o1, RexxObjectPtr o2, RexxObjectPtr o3)
    {
        return functions->ArrayOfThree(this, o1, o2, o3);
    }
    RexxArrayObject ArrayOfFour(RexxObjectPtr o1, RexxObjectPtr o2, RexxObjectPtr o3, RexxObjectPtr o4)
    {
        return functions->ArrayOfFour(this, o1, o2, o3, o4);
    }
    RexxArrayObject Array(RexxObjectPtr o)
    {
        return functions->ArrayOfOne(this, o);
    }
    RexxArrayObject Array(RexxObjectPtr o1, RexxObjectPtr o2)
    {
        return functions->ArrayOfTwo(this, o1, o2);
    }
    RexxArrayObject Array(RexxObjectPtr o1, RexxObjectPtr o2, RexxObjectPtr o3)
    {
        return functions->ArrayOfThree(this, o1, o2, o3);
    }
    RexxArrayObject Array(RexxObjectPtr o1, RexxObjectPtr o2, RexxObjectPtr o3, RexxObjectPtr o4)
    {
        return functions->ArrayOfFour(this, o1, o2, o3, o4);
    }
    logical_t IsArray(RexxObjectPtr o)
    {
        return functions->IsArray(this, o);
    }

    POINTER BufferData(RexxBufferObject bo)
    {
        return functions->BufferData(this, bo);
    }
    size_t BufferLength(RexxBufferObject bo)
    {
        return functions->BufferLength(this, bo);
    }
    RexxBufferObject NewBuffer(wholenumber_t n)
    {
        return functions->NewBuffer(this, n);
    }
    logical_t IsBuffer(RexxObjectPtr o)
    {
        return functions->IsBuffer(this, o);
    }

    POINTER PointerValue(RexxPointerObject po)
    {
        return functions->PointerValue(this, po);
    }
    RexxPointerObject NewPointer(POINTER po)
    {
        return functions->NewPointer(this, po);
    }
    logical_t IsPointer(RexxObjectPtr o)
    {
        return functions->IsPointer(this, o);
    }

    RexxObjectPtr SupplierItem(RexxSupplierObject so)
    {
        return functions->SupplierItem(this, so);
    }
    RexxObjectPtr SupplierIndex(RexxSupplierObject so)
    {
        return functions->SupplierIndex(this, so);
    }
    logical_t SupplierAvailable(RexxSupplierObject so)
    {
        return functions->SupplierAvailable(this, so);
    }
    void SupplierNext(RexxSupplierObject so)
    {
        functions->SupplierNext(this, so);
    }
    RexxSupplierObject NewSupplier(RexxArrayObject values, RexxArrayObject names)
    {
        return functions->NewSupplier(this, values, names);
    }

    RexxStemObject NewStem(CSTRING n)
    {
        return functions->NewStem(this, n);
    }
    void SetStemElement(RexxStemObject so, CSTRING s, RexxObjectPtr o)
    {
        functions->SetStemElement(this, so, s, o);
    }
    RexxObjectPtr GetStemElement(RexxStemObject so, CSTRING s)
    {
        return functions->GetStemElement(this, so, s);
    }
    void DropStemElement(RexxStemObject so, CSTRING s)
    {
        functions->DropStemElement(this, so, s);
    }
    void SetStemArrayElement(RexxStemObject so, size_t n, RexxObjectPtr o)
    {
        functions->SetStemArrayElement(this, so, n, o);
    }
    RexxObjectPtr GetStemArrayElement(RexxStemObject so, size_t n)
    {
        return functions->GetStemArrayElement(this, so, n);
    }
    void DropStemArrayElement(RexxStemObject so, size_t n)
    {
        functions->DropStemArrayElement(this, so, n);
    }
    RexxDirectoryObject GetAllStemElements(RexxStemObject so)
    {
        return functions->GetAllStemElements(this, so);
    }
    RexxObjectPtr GetStemValue(RexxStemObject so)
    {
        return functions->GetStemValue(this, so);
    }
    logical_t IsStem(RexxObjectPtr o)
    {
        return functions->IsStem(this, o);
    }
    void RaiseException0(size_t n)
    {
        functions->RaiseException0(this, n);
    }
    void RaiseException1(size_t n, RexxObjectPtr o)
    {
        functions->RaiseException1(this, n, o);
    }
    void RaiseException2(size_t n, RexxObjectPtr o1, RexxObjectPtr o2)
    {
        functions->RaiseException2(this, n, o1, o2);
    }
    void RaiseException(size_t n, RexxArrayObject ao)
    {
        functions->RaiseException(this, n, ao);
    }
    void RaiseCondition(CSTRING s1, RexxStringObject s2, RexxObjectPtr ao, RexxObjectPtr o)
    {
        functions->RaiseCondition(this, s1, s2, ao, o);
    }
    logical_t CheckCondition()
    {
        return functions->CheckCondition(this);
    }
    RexxDirectoryObject GetConditionInfo()
    {
        return functions->GetConditionInfo(this);
    }
    void DecodeConditionInfo(RexxDirectoryObject diro, RexxCondition *c)
    {
        functions->DecodeConditionInfo(this, diro, c);
    }
    void ClearCondition()
    {
        functions->ClearCondition(this);
    }

    RexxObjectPtr Nil()
    {
        return functions->RexxNil;
    }
    RexxObjectPtr True()
    {
        return functions->RexxTrue;
    }
    RexxObjectPtr False()
    {
        return functions->RexxFalse;
    }
    RexxStringObject NullString()
    {
        return functions->RexxNullString;
    }
#endif
};

struct RexxMethodContext_
{
    RexxThreadContext *threadContext;   // the interpreter instance state
    MethodContextInterface *functions;  // functions available in a method context
    ValueDescriptor *arguments;         // the argument descriptor

#ifdef __cplusplus
    POINTER GetApplicationData()
    {
        return threadContext->GetApplicationData();
    }
    size_t InterpreterVersion()
    {
        return threadContext->InterpreterVersion();
    }
    size_t LanguageLevel()
    {
        return threadContext->LanguageLevel();
    }
    RexxObjectPtr RequestGlobalReference(RexxObjectPtr o)
    {
        return threadContext->RequestGlobalReference(o);
    }
    void ReleaseGlobalReference(RexxObjectPtr o)
    {
        threadContext->ReleaseGlobalReference(o);
    }
    void ReleaseLocalReference(RexxObjectPtr o)
    {
        threadContext->ReleaseLocalReference(o);
    }

    RexxObjectPtr SendMessage(RexxObjectPtr o, CSTRING s, RexxArrayObject ao)
    {
        return threadContext->SendMessage(o, s, ao);
    }
    RexxObjectPtr SendMessage0(RexxObjectPtr o, CSTRING s)
    {
        return threadContext->SendMessage0(o, s);
    }
    RexxObjectPtr SendMessage1(RexxObjectPtr o, CSTRING s, RexxObjectPtr a1)
    {
        return threadContext->SendMessage1(o, s, a1);
    }
    RexxObjectPtr SendMessage2(RexxObjectPtr o, CSTRING s, RexxObjectPtr a1, RexxObjectPtr a2)
    {
        return threadContext->SendMessage2(o, s, a1, a2);
    }

    RexxDirectoryObject GetLocalEnvironment()
    {
        return threadContext->GetLocalEnvironment();
    }
    RexxDirectoryObject GetGlobalEnvironment()
    {
        return threadContext->GetGlobalEnvironment();
    }

    logical_t IsInstanceOf(RexxObjectPtr o, RexxClassObject co)
    {
        return threadContext->IsInstanceOf(o, co);
    }
    logical_t IsOfType(RexxObjectPtr o, CSTRING cn)
    {
        return threadContext->IsOfType(o, cn);
    }
    RexxClassObject FindClass(CSTRING s)
    {
        return threadContext->FindClass(s);
    }
    RexxClassObject FindPackageClass(RexxPackageObject m, CSTRING n)
    {
        return threadContext->FindPackageClass(m, n);
    }
    logical_t HasMethod(RexxObjectPtr o, CSTRING m)
    {
        return threadContext->HasMethod(o, m);
    }
    RexxMethodObject NewMethod(CSTRING n, CSTRING s, size_t l)
    {
        return threadContext->NewMethod(n, s, l);
    }
    RexxRoutineObject NewRoutine(CSTRING n, CSTRING s, size_t l)
    {
        return threadContext->NewRoutine(n, s, l);
    }
    logical_t IsRoutine(RexxObjectPtr o)
    {
        return threadContext->IsRoutine(o);
    }
    logical_t IsMethod(RexxObjectPtr o)
    {
        return threadContext->IsMethod(o);
    }
    RexxPackageObject GetRoutinePackage(RexxRoutineObject o)
    {
        return threadContext->GetRoutinePackage(o);
    }
    RexxPackageObject GetMethodPackage(RexxMethodObject o)
    {
        return threadContext->GetMethodPackage(o);
    }

    RexxDirectoryObject GetPackageRoutines(RexxPackageObject m)
    {
        return threadContext->GetPackageRoutines(m);
    }
    RexxDirectoryObject GetPackagePublicRoutines(RexxPackageObject m)
    {
        return threadContext->GetPackagePublicRoutines(m);
    }
    RexxDirectoryObject GetPackageClasses(RexxPackageObject m)
    {
        return threadContext->GetPackageClasses(m);
    }
    RexxDirectoryObject GetPackagePublicClasses(RexxPackageObject m)
    {
        return threadContext->GetPackagePublicClasses(m);
    }
    RexxDirectoryObject GetPackageMethods(RexxPackageObject m)
    {
        return threadContext->GetPackageMethods(m);
    }
    RexxObjectPtr CallRoutine(RexxRoutineObject m, RexxArrayObject a)
    {
        return threadContext->CallRoutine(m, a);
    }
    RexxObjectPtr CallProgram(CSTRING n,RexxArrayObject a)
    {
        return threadContext->CallProgram(n, a);
    }
    RexxPackageObject LoadPackage(CSTRING d)
    {
        return threadContext->LoadPackage(d);
    }
    RexxPackageObject LoadPackageFromData(CSTRING n, CSTRING d, size_t l)
    {
        return threadContext->LoadPackageFromData(n, d, l);
    }
    logical_t LoadLibrary(CSTRING n)
    {
        return threadContext->LoadLibrary(n);
    }
    logical_t RegisterLibrary(CSTRING n, RexxPackageEntry *e)
    {
        return threadContext->RegisterLibrary(n, e);
    }

    POINTER ObjectToCSelf(RexxObjectPtr o)
    {
        return threadContext->ObjectToCSelf(o);
    }
    RexxObjectPtr WholeNumberToObject(wholenumber_t n)
    {
        return threadContext->WholeNumberToObject(n);
    }
    RexxObjectPtr WholeNumber(wholenumber_t n)
    {
        return threadContext->WholeNumberToObject(n);
    }
    RexxObjectPtr UintptrToObject(uintptr_t n)
    {
        return threadContext->UintptrToObject(n);
    }
    RexxObjectPtr Uintptr(uintptr_t n)
    {
        return threadContext->UintptrToObject(n);
    }
    RexxObjectPtr IntptrToObject(intptr_t n)
    {
        return threadContext->IntptrToObject(n);
    }
    RexxObjectPtr Intptr(intptr_t n)
    {
        return threadContext->IntptrToObject(n);
    }
    RexxObjectPtr ValueToObject(ValueDescriptor *v)
    {
        return threadContext->ValueToObject(v);
    }
    RexxArrayObject ValuesToObject(ValueDescriptor *v, size_t c)
    {
        return threadContext->ValuesToObject(v, c);
    }
    logical_t ObjectToValue(RexxObjectPtr o, ValueDescriptor *v)
    {
        return threadContext->ObjectToValue(o, v);
    }
    RexxObjectPtr StringSizeToObject(size_t u)
    {
        return threadContext->StringSizeToObject(u);
    }
    RexxObjectPtr StringSize(size_t u)
    {
        return threadContext->StringSizeToObject(u);
    }
    logical_t ObjectToWholeNumber(RexxObjectPtr o, wholenumber_t *n)
    {
        return threadContext->ObjectToWholeNumber(o, n);
    }
    logical_t WholeNumber(RexxObjectPtr o, wholenumber_t *n)
    {
        return threadContext->ObjectToWholeNumber(o, n);
    }
    logical_t ObjectToStringSize(RexxObjectPtr o, size_t *n)
    {
        return threadContext->ObjectToStringSize(o, n);
    }
    logical_t StringSize(RexxObjectPtr o, size_t *n)
    {
        return threadContext->ObjectToStringSize(o, n);
    }
    RexxObjectPtr Int64ToObject(int64_t i)
    {
        return threadContext->Int64ToObject(i);
    }
    RexxObjectPtr Int64(int64_t i)
    {
        return threadContext->Int64ToObject(i);
    }
    RexxObjectPtr UnsignedInt64ToObject(uint64_t u)
    {
        return threadContext->UnsignedInt64ToObject(u);
    }
    RexxObjectPtr UnsignedInt64(uint64_t u)
    {
        return threadContext->UnsignedInt64ToObject(u);
    }
    logical_t ObjectToInt64(RexxObjectPtr o, int64_t *i)
    {
        return threadContext->ObjectToInt64(o, i);
    }
    logical_t Int64(RexxObjectPtr o, int64_t *i)
    {
        return threadContext->ObjectToInt64(o, i);
    }
    logical_t ObjectToUnsignedInt64(RexxObjectPtr o, uint64_t *u)
    {
        return threadContext->ObjectToUnsignedInt64(o, u);
    }
    logical_t UnsignedInt64(RexxObjectPtr o, uint64_t *u)
    {
        return threadContext->ObjectToUnsignedInt64(o, u);
    }
    RexxObjectPtr Int32ToObject(int32_t i)
    {
        return threadContext->Int32ToObject(i);
    }
    RexxObjectPtr Int32(int32_t i)
    {
        return threadContext->Int32ToObject(i);
    }
    RexxObjectPtr UnsignedInt32ToObject(uint32_t u)
    {
        return threadContext->UnsignedInt32ToObject(u);
    }
    RexxObjectPtr UnsignedInt32(uint32_t u)
    {
        return threadContext->UnsignedInt32ToObject(u);
    }
    logical_t ObjectToInt32(RexxObjectPtr o, int32_t *i)
    {
        return threadContext->ObjectToInt32(o, i);
    }
    logical_t Int32(RexxObjectPtr o, int32_t *i)
    {
        return threadContext->ObjectToInt32(o, i);
    }
    logical_t ObjectToUnsignedInt32(RexxObjectPtr o, uint32_t *u)
    {
        return threadContext->ObjectToUnsignedInt32(o, u);
    }
    logical_t UnsignedInt32(RexxObjectPtr o, uint32_t *u)
    {
        return threadContext->ObjectToUnsignedInt32(o, u);
    }
    logical_t ObjectToUintptr(RexxObjectPtr o, uintptr_t *n)
    {
        return threadContext->ObjectToUintptr(o, n);
    }
    logical_t Uintptr(RexxObjectPtr o, uintptr_t *n)
    {
        return threadContext->ObjectToUintptr(o, n);
    }
    logical_t ObjectToIntptr(RexxObjectPtr o, intptr_t *n)
    {
        return threadContext->ObjectToIntptr(o, n);
    }
    logical_t Intptr(RexxObjectPtr o, intptr_t *n)
    {
        return threadContext->ObjectToIntptr(o, n);
    }
    logical_t ObjectToLogical(RexxObjectPtr o, logical_t *n)
    {
        return threadContext->ObjectToLogical(o, n);
    }
    logical_t Logical(RexxObjectPtr o, logical_t *n)
    {
        return threadContext->ObjectToLogical(o, n);
    }
    RexxObjectPtr LogicalToObject(logical_t l)
    {
        return threadContext->LogicalToObject(l);
    }
    RexxObjectPtr Logical(logical_t l)
    {
        return threadContext->LogicalToObject(l);
    }
    RexxObjectPtr DoubleToObject(double d)
    {
        return threadContext->DoubleToObject(d);
    }
    RexxObjectPtr Double(double d)
    {
        return threadContext->DoubleToObject(d);
    }
    RexxObjectPtr DoubleToObjectWithPrecision(double d, size_t precision)
    {
        return threadContext->DoubleToObjectWithPrecision(d, precision);
    }
    logical_t ObjectToDouble(RexxObjectPtr o, double *d)
    {
        return threadContext->ObjectToDouble(o, d);
    }
    logical_t Double(RexxObjectPtr o, double *d)
    {
        return threadContext->ObjectToDouble(o, d);
    }

    RexxStringObject ObjectToString(RexxObjectPtr o)
    {
        return threadContext->ObjectToString(o);
    }
    CSTRING ObjectToStringValue(RexxObjectPtr o)
    {
        return threadContext->ObjectToStringValue(o);
    }
    CSTRING CString(RexxObjectPtr o)
    {
        return threadContext->ObjectToStringValue(o);
    }
    size_t  StringGet(RexxStringObject o, size_t n1, POINTER s, size_t n2)
    {
        return threadContext->StringGet(o, n1, s, n2);
    }
    size_t  StringLength(RexxStringObject o)
    {
        return threadContext->StringLength(o);
    }
    CSTRING StringData(RexxStringObject o)
    {
        return threadContext->StringData(o);
    }
    RexxStringObject NewString(CSTRING s, size_t len)
    {
        return threadContext->NewString(s, len);
    }
    RexxStringObject NewStringFromAsciiz(CSTRING s)
    {
        return threadContext->NewStringFromAsciiz(s);
    }
    RexxStringObject String(CSTRING s, size_t len)
    {
        return threadContext->NewString(s, len);
    }
    RexxStringObject String(CSTRING s)
    {
        return threadContext->NewStringFromAsciiz(s);
    }
    RexxStringObject CString(CSTRING s)
    {
        return threadContext->NewStringFromAsciiz(s);
    }
    RexxStringObject StringUpper(RexxStringObject s)
    {
        return threadContext->StringUpper(s);
    }
    RexxStringObject StringLower(RexxStringObject s)
    {
        return threadContext->StringLower(s);
    }
    logical_t IsString(RexxObjectPtr o)
    {
        return threadContext->IsString(o);
    }

    RexxBufferStringObject NewBufferString(size_t len)
    {
        return threadContext->NewBufferString(len);
    }

    size_t BufferStringLength(RexxBufferStringObject o)
    {
        return threadContext->BufferStringLength(o);
    }

    POINTER BufferStringData(RexxBufferStringObject o)
    {
        return threadContext->BufferStringData(o);
    }

    RexxStringObject FinishBufferString(RexxBufferStringObject o, size_t l)
    {
        return threadContext->FinishBufferString(o, l);
    }

    void DirectoryPut(RexxDirectoryObject diro, RexxObjectPtr o, CSTRING s)
    {
        threadContext->DirectoryPut(diro, o, s);
    }
    RexxObjectPtr DirectoryAt(RexxDirectoryObject to, CSTRING s)
    {
        return threadContext->DirectoryAt(to, s);
    }
    RexxObjectPtr DirectoryRemove(RexxDirectoryObject to, CSTRING s)
    {
        return threadContext->DirectoryRemove(to, s);
    }
    RexxDirectoryObject NewDirectory()
    {
        return threadContext->NewDirectory();
    }
    logical_t IsDirectory(RexxObjectPtr o)
    {
        return threadContext->IsDirectory(o);
    }

    RexxObjectPtr ArrayAt(RexxArrayObject ao, size_t n)
    {
        return threadContext->ArrayAt(ao, n);
    }
    void ArrayPut(RexxArrayObject ao, RexxObjectPtr o, size_t n)
    {
        threadContext->ArrayPut(ao, o, n);
    }
    size_t ArrayAppend(RexxArrayObject ao, RexxObjectPtr o)
    {
        return threadContext->ArrayAppend(ao, o);
    }
    size_t ArrayAppendString(RexxArrayObject ao, CSTRING s, size_t l)
    {
        return threadContext->ArrayAppendString(ao, s, l);
    }
    size_t ArraySize(RexxArrayObject ao)
    {
        return threadContext->ArraySize(ao);
    }
    size_t ArrayItems(RexxArrayObject ao)
    {
        return threadContext->ArrayItems(ao);
    }
    size_t ArrayDimension(RexxArrayObject ao)
    {
        return threadContext->ArrayDimension(ao);
    }
    RexxArrayObject NewArray(size_t n)
    {
        return threadContext->NewArray(n);
    }
    RexxArrayObject ArrayOfOne(RexxObjectPtr o)
    {
        return threadContext->ArrayOfOne(o);
    }
    RexxArrayObject ArrayOfTwo(RexxObjectPtr o1, RexxObjectPtr o2)
    {
        return threadContext->ArrayOfTwo(o1, o2);
    }
    RexxArrayObject ArrayOfThree(RexxObjectPtr o1, RexxObjectPtr o2, RexxObjectPtr o3)
    {
        return threadContext->ArrayOfThree(o1, o2, o3);
    }
    RexxArrayObject ArrayOfFour(RexxObjectPtr o1, RexxObjectPtr o2, RexxObjectPtr o3, RexxObjectPtr o4)
    {
        return threadContext->ArrayOfFour(o1, o2, o3, o4);
    }
    RexxArrayObject Array(RexxObjectPtr o)
    {
        return threadContext->ArrayOfOne(o);
    }
    RexxArrayObject Array(RexxObjectPtr o1, RexxObjectPtr o2)
    {
        return threadContext->ArrayOfTwo(o1, o2);
    }
    RexxArrayObject Array(RexxObjectPtr o1, RexxObjectPtr o2, RexxObjectPtr o3)
    {
        return threadContext->ArrayOfThree(o1, o2, o3);
    }
    RexxArrayObject Array(RexxObjectPtr o1, RexxObjectPtr o2, RexxObjectPtr o3, RexxObjectPtr o4)
    {
        return threadContext->ArrayOfFour(o1, o2, o3, o4);
    }
    logical_t IsArray(RexxObjectPtr o)
    {
        return threadContext->IsArray(o);
    }

    POINTER BufferData(RexxBufferObject bo)
    {
        return threadContext->BufferData(bo);
    }
    size_t BufferLength(RexxBufferObject bo)
    {
        return threadContext->BufferLength(bo);
    }
    RexxBufferObject NewBuffer(wholenumber_t n)
    {
        return threadContext->NewBuffer(n);
    }
    logical_t IsBuffer(RexxObjectPtr o)
    {
        return threadContext->IsBuffer(o);
    }

    POINTER PointerValue(RexxPointerObject po)
    {
        return threadContext->PointerValue(po);
    }
    RexxPointerObject NewPointer(POINTER p)
    {
        return threadContext->NewPointer(p);
    }
    logical_t IsPointer(RexxObjectPtr o)
    {
        return threadContext->IsPointer(o);
    }

    RexxObjectPtr SupplierItem(RexxSupplierObject so)
    {
        return threadContext->SupplierItem(so);
    }
    RexxObjectPtr SupplierIndex(RexxSupplierObject so)
    {
        return threadContext->SupplierIndex(so);
    }
    logical_t SupplierAvailable(RexxSupplierObject so)
    {
        return threadContext->SupplierAvailable(so);
    }
    void SupplierNext(RexxSupplierObject so)
    {
        threadContext->SupplierNext(so);
    }
    RexxSupplierObject NewSupplier(RexxArrayObject values, RexxArrayObject names)
    {
        return threadContext->NewSupplier(values, names);
    }

    RexxStemObject NewStem(CSTRING n)
    {
        return threadContext->NewStem(n);
    }
    void SetStemElement(RexxStemObject so, CSTRING s, RexxObjectPtr o)
    {
        threadContext->SetStemElement(so, s, o);
    }
    RexxObjectPtr GetStemElement(RexxStemObject so, CSTRING s)
    {
        return threadContext->GetStemElement(so, s);
    }
    void DropStemElement(RexxStemObject so, CSTRING s)
    {
        threadContext->DropStemElement(so, s);
    }
    void SetStemArrayElement(RexxStemObject so, size_t n, RexxObjectPtr o)
    {
        threadContext->SetStemArrayElement(so, n, o);
    }
    RexxObjectPtr GetStemArrayElement(RexxStemObject so, size_t n)
    {
        return threadContext->GetStemArrayElement(so, n);
    }
    void DropStemArrayElement(RexxStemObject so, size_t n)
    {
        threadContext->DropStemArrayElement(so, n);
    }
    RexxDirectoryObject GetAllStemElements(RexxStemObject so)
    {
        return threadContext->GetAllStemElements(so);
    }
    RexxObjectPtr GetStemValue(RexxStemObject so)
    {
        return threadContext->GetStemValue(so);
    }
    logical_t IsStem(RexxObjectPtr o)
    {
        return threadContext->IsStem(o);
    }

    void RaiseException0(size_t n)
    {
        threadContext->RaiseException0(n);
    }
    void RaiseException1(size_t n, RexxObjectPtr o)
    {
        threadContext->RaiseException1(n, o);
    }
    void RaiseException2(size_t n, RexxObjectPtr o1, RexxObjectPtr o2)
    {
        threadContext->RaiseException2(n, o1, o2);
    }
    void RaiseException(size_t n, RexxArrayObject ao)
    {
        threadContext->RaiseException(n, ao);
    }
    void RaiseCondition(CSTRING s1, RexxStringObject s2, RexxObjectPtr ao, RexxObjectPtr o)
    {
        threadContext->RaiseCondition(s1, s2, ao, o);
    }
    logical_t CheckCondition()
    {
        return threadContext->CheckCondition();
    }
    RexxDirectoryObject GetConditionInfo()
    {
        return threadContext->GetConditionInfo();
    }
    void DecodeConditionInfo(RexxDirectoryObject diro, RexxCondition *c)
    {
        threadContext->DecodeConditionInfo(diro, c);
    }
    void ClearCondition()
    {
        threadContext->ClearCondition();
    }

    RexxObjectPtr Nil()
    {
        return threadContext->Nil();
    }
    RexxObjectPtr True()
    {
        return threadContext->True();
    }
    RexxObjectPtr False()
    {
        return threadContext->False();
    }
    RexxStringObject NullString()
    {
        return threadContext->NullString();
    }

    RexxArrayObject GetArguments()
    {
        return functions->GetArguments(this);
    }
    RexxObjectPtr GetArgument(size_t n)
    {
        return functions->GetArgument(this, n);
    }
    CSTRING GetMessageName()
    {
        return functions->GetMessageName(this);
    }
    RexxMethodObject GetMethod()
    {
        return functions->GetMethod(this);
    }
    RexxObjectPtr GetSelf()
    {
        return functions->GetSelf(this);
    }
    RexxClassObject GetSuper()
    {
        return functions->GetSuper(this);
    }
    RexxObjectPtr GetScope()
    {
        return functions->GetScope(this);
    }
    void SetObjectVariable(CSTRING s, RexxObjectPtr o)
    {
        functions->SetObjectVariable(this, s, o);
    }
    RexxObjectPtr GetObjectVariable(CSTRING s)
    {
        return functions->GetObjectVariable(this, s);
    }
    void DropObjectVariable(CSTRING s)
    {
        functions->DropObjectVariable(this, s);
    }
    RexxObjectPtr ForwardMessage(RexxObjectPtr o, CSTRING s, RexxClassObject c, RexxArrayObject a)
    {
        return functions->ForwardMessage(this, o, s, c, a);
    }
    void SetGuardOn()
    {
        functions->SetGuardOn(this);
    }
    void SetGuardOff()
    {
        functions->SetGuardOff(this);
    }
    RexxClassObject FindContextClass(CSTRING n)
    {
        return functions->FindContextClass(this, n);
    }
#endif
};

struct RexxCallContext_
{
    RexxThreadContext *threadContext;   // the interpreter instance state
    CallContextInterface *functions;    // functions available in a method context
    ValueDescriptor *arguments;         // the argument descriptor

#ifdef __cplusplus
    POINTER GetApplicationData()
    {
        return threadContext->GetApplicationData();
    }
    size_t InterpreterVersion()
    {
        return threadContext->InterpreterVersion();
    }
    size_t LanguageLevel()
    {
        return threadContext->LanguageLevel();
    }
    RexxObjectPtr RequestGlobalReference(RexxObjectPtr o)
    {
        return threadContext->RequestGlobalReference(o);
    }
    void ReleaseGlobalReference(RexxObjectPtr o)
    {
        threadContext->ReleaseGlobalReference(o);
    }
    void ReleaseLocalReference(RexxObjectPtr o)
    {
        threadContext->ReleaseLocalReference(o);
    }
    RexxObjectPtr SendMessage(RexxObjectPtr o, CSTRING s, RexxArrayObject ao)
    {
        return threadContext->SendMessage(o, s, ao);
    }
    RexxObjectPtr SendMessage0(RexxObjectPtr o, CSTRING s)
    {
        return threadContext->SendMessage0(o, s);
    }
    RexxObjectPtr SendMessage1(RexxObjectPtr o, CSTRING s, RexxObjectPtr a1)
    {
        return threadContext->SendMessage1(o, s, a1);
    }
    RexxObjectPtr SendMessage2(RexxObjectPtr o, CSTRING s, RexxObjectPtr a1, RexxObjectPtr a2)
    {
        return threadContext->SendMessage2(o, s, a1, a2);
    }

    RexxDirectoryObject GetLocalEnvironment()
    {
        return threadContext->GetLocalEnvironment();
    }
    RexxDirectoryObject GetGlobalEnvironment()
    {
        return threadContext->GetGlobalEnvironment();
    }

    logical_t IsInstanceOf(RexxObjectPtr o, RexxClassObject co)
    {
        return threadContext->IsInstanceOf(o, co);
    }
    logical_t IsOfType(RexxObjectPtr o, CSTRING cn)
    {
        return threadContext->IsOfType(o, cn);
    }
    RexxClassObject FindClass(CSTRING s)
    {
        return threadContext->FindClass(s);
    }
    RexxClassObject FindPackageClass(RexxPackageObject m, CSTRING n)
    {
        return threadContext->FindPackageClass(m, n);
    }
    logical_t HasMethod(RexxObjectPtr o, CSTRING m)
    {
        return threadContext->HasMethod(o, m);
    }

    RexxMethodObject NewMethod(CSTRING n, CSTRING s, size_t l)
    {
        return threadContext->NewMethod(n, s, l);
    }

    RexxRoutineObject NewRoutine(CSTRING n, CSTRING s, size_t l)
    {
        return threadContext->NewRoutine(n, s, l);
    }
    logical_t IsRoutine(RexxObjectPtr o)
    {
        return threadContext->IsRoutine(o);
    }
    logical_t IsMethod(RexxObjectPtr o)
    {
        return threadContext->IsMethod(o);
    }
    RexxPackageObject GetRoutinePackage(RexxRoutineObject o)
    {
        return threadContext->GetRoutinePackage(o);
    }
    RexxPackageObject GetMethodPackage(RexxMethodObject o)
    {
        return threadContext->GetMethodPackage(o);
    }

    RexxDirectoryObject GetPackageRoutines(RexxPackageObject m)
    {
        return threadContext->GetPackageRoutines(m);
    }
    RexxDirectoryObject GetPackagePublicRoutines(RexxPackageObject m)
    {
        return threadContext->GetPackagePublicRoutines(m);
    }
    RexxDirectoryObject GetPackageClasses(RexxPackageObject m)
    {
        return threadContext->GetPackageClasses(m);
    }
    RexxDirectoryObject GetPackagePublicClasses(RexxPackageObject m)
    {
        return threadContext->GetPackagePublicClasses(m);
    }
    RexxDirectoryObject GetPackageMethods(RexxPackageObject m)
    {
        return threadContext->GetPackageMethods(m);
    }
    RexxObjectPtr CallRoutine(RexxRoutineObject m, RexxArrayObject a)
    {
        return threadContext->CallRoutine(m, a);
    }
    RexxObjectPtr CallProgram(CSTRING n,RexxArrayObject a)
    {
        return threadContext->CallProgram(n, a);
    }
    RexxPackageObject LoadPackage(CSTRING d)
    {
        return threadContext->LoadPackage(d);
    }
    RexxPackageObject LoadPackageFromData(CSTRING n, CSTRING d, size_t l)
    {
        return threadContext->LoadPackageFromData(n, d, l);
    }
    logical_t LoadLibrary(CSTRING n)
    {
        return threadContext->LoadLibrary(n);
    }
    logical_t RegisterLibrary(CSTRING n, RexxPackageEntry *e)
    {
        return threadContext->RegisterLibrary(n, e);
    }
    POINTER ObjectToCSelf(RexxObjectPtr o)
    {
        return threadContext->ObjectToCSelf(o);
    }
    RexxObjectPtr WholeNumberToObject(wholenumber_t n)
    {
        return threadContext->WholeNumberToObject(n);
    }
    RexxObjectPtr WholeNumber(wholenumber_t n)
    {
        return threadContext->WholeNumberToObject(n);
    }
    RexxObjectPtr UintptrToObject(uintptr_t n)
    {
        return threadContext->UintptrToObject(n);
    }
    RexxObjectPtr Uintptr(uintptr_t n)
    {
        return threadContext->UintptrToObject(n);
    }
    RexxObjectPtr IntptrToObject(intptr_t n)
    {
        return threadContext->IntptrToObject(n);
    }
    RexxObjectPtr Intptr(intptr_t n)
    {
        return threadContext->IntptrToObject(n);
    }
    RexxObjectPtr ValueToObject(ValueDescriptor *v)
    {
        return threadContext->ValueToObject(v);
    }
    RexxArrayObject ValuesToObject(ValueDescriptor *v, size_t c)
    {
        return threadContext->ValuesToObject(v, c);
    }
    logical_t ObjectToValue(RexxObjectPtr o, ValueDescriptor *v)
    {
        return threadContext->ObjectToValue(o, v);
    }
    RexxObjectPtr StringSizeToObject(size_t u)
    {
        return threadContext->StringSizeToObject(u);
    }
    RexxObjectPtr StringSize(size_t u)
    {
        return threadContext->StringSizeToObject(u);
    }
    logical_t ObjectToWholeNumber(RexxObjectPtr o, wholenumber_t *n)
    {
        return threadContext->ObjectToWholeNumber(o, n);
    }
    logical_t WholeNumber(RexxObjectPtr o, wholenumber_t *n)
    {
        return threadContext->ObjectToWholeNumber(o, n);
    }
    logical_t ObjectToStringSize(RexxObjectPtr o, size_t *n)
    {
        return threadContext->ObjectToStringSize(o, n);
    }
    logical_t StringSize(RexxObjectPtr o, size_t *n)
    {
        return threadContext->ObjectToStringSize(o, n);
    }
    RexxObjectPtr Int64ToObject(int64_t i)
    {
        return threadContext->Int64ToObject(i);
    }
    RexxObjectPtr Int64(int64_t i)
    {
        return threadContext->Int64ToObject(i);
    }
    RexxObjectPtr UnsignedInt64ToObject(uint64_t u)
    {
        return threadContext->UnsignedInt64ToObject(u);
    }
    RexxObjectPtr UnsignedInt64(uint64_t u)
    {
        return threadContext->UnsignedInt64ToObject(u);
    }
    logical_t ObjectToInt64(RexxObjectPtr o, int64_t *i)
    {
        return threadContext->ObjectToInt64(o, i);
    }
    logical_t Int64(RexxObjectPtr o, int64_t *i)
    {
        return threadContext->ObjectToInt64(o, i);
    }
    logical_t ObjectToUnsignedInt64(RexxObjectPtr o, uint64_t *u)
    {
        return threadContext->ObjectToUnsignedInt64(o, u);
    }
    logical_t UnsignedInt64(RexxObjectPtr o, uint64_t *u)
    {
        return threadContext->ObjectToUnsignedInt64(o, u);
    }
    RexxObjectPtr Int32ToObject(int32_t i)
    {
        return threadContext->Int32ToObject(i);
    }
    RexxObjectPtr Int32(int32_t i)
    {
        return threadContext->Int32ToObject(i);
    }
    RexxObjectPtr UnsignedInt32ToObject(uint32_t u)
    {
        return threadContext->UnsignedInt32ToObject(u);
    }
    RexxObjectPtr UnsignedInt32(uint32_t u)
    {
        return threadContext->UnsignedInt32ToObject(u);
    }
    logical_t ObjectToInt32(RexxObjectPtr o, int32_t *i)
    {
        return threadContext->ObjectToInt32(o, i);
    }
    logical_t Int32(RexxObjectPtr o, int32_t *i)
    {
        return threadContext->ObjectToInt32(o, i);
    }
    logical_t ObjectToUnsignedInt32(RexxObjectPtr o, uint32_t *u)
    {
        return threadContext->ObjectToUnsignedInt32(o, u);
    }
    logical_t UnsignedInt32(RexxObjectPtr o, uint32_t *u)
    {
        return threadContext->ObjectToUnsignedInt32(o, u);
    }
    logical_t ObjectToUintptr(RexxObjectPtr o, uintptr_t *n)
    {
        return threadContext->ObjectToUintptr(o, n);
    }
    logical_t Uintptr(RexxObjectPtr o, uintptr_t *n)
    {
        return threadContext->ObjectToUintptr(o, n);
    }
    logical_t ObjectToIntptr(RexxObjectPtr o, intptr_t *n)
    {
        return threadContext->ObjectToIntptr(o, n);
    }
    logical_t Intptr(RexxObjectPtr o, intptr_t *n)
    {
        return threadContext->ObjectToIntptr(o, n);
    }
    logical_t ObjectToLogical(RexxObjectPtr o, logical_t *n)
    {
        return threadContext->ObjectToLogical(o, n);
    }
    logical_t Logical(RexxObjectPtr o, logical_t *n)
    {
        return threadContext->ObjectToLogical(o, n);
    }
    RexxObjectPtr LogicalToObject(logical_t l)
    {
        return threadContext->LogicalToObject(l);
    }
    RexxObjectPtr Logical(logical_t l)
    {
        return threadContext->LogicalToObject(l);
    }
    RexxObjectPtr DoubleToObject(double d)
    {
        return threadContext->DoubleToObject(d);
    }
    RexxObjectPtr Double(double d)
    {
        return threadContext->DoubleToObject(d);
    }
    RexxObjectPtr DoubleToObjectWithPrecision(double d, size_t precision)
    {
        return threadContext->DoubleToObjectWithPrecision(d, precision);
    }
    logical_t ObjectToDouble(RexxObjectPtr o, double *d)
    {
        return threadContext->ObjectToDouble(o, d);
    }
    logical_t Double(RexxObjectPtr o, double *d)
    {
        return threadContext->ObjectToDouble(o, d);
    }

    RexxStringObject ObjectToString(RexxObjectPtr o)
    {
        return threadContext->ObjectToString(o);
    }
    CSTRING ObjectToStringValue(RexxObjectPtr o)
    {
        return threadContext->ObjectToStringValue(o);
    }
    CSTRING CString(RexxObjectPtr o)
    {
        return threadContext->ObjectToStringValue(o);
    }
    size_t StringGet(RexxStringObject o, size_t n1, POINTER s, size_t n2)
    {
        return threadContext->StringGet(o, n1, s, n2);
    }
    size_t StringLength(RexxStringObject o)
    {
        return threadContext->StringLength(o);
    }
    CSTRING StringData(RexxStringObject o)
    {
        return threadContext->StringData(o);
    }
    RexxStringObject NewString(CSTRING s, size_t len)
    {
        return threadContext->NewString(s, len);
    }
    RexxStringObject NewStringFromAsciiz(CSTRING s)
    {
        return threadContext->NewStringFromAsciiz(s);
    }
    RexxStringObject String(CSTRING s, size_t len)
    {
        return threadContext->NewString(s, len);
    }
    RexxStringObject String(CSTRING s)
    {
        return threadContext->NewStringFromAsciiz(s);
    }
    RexxStringObject CString(CSTRING s)
    {
        return threadContext->NewStringFromAsciiz(s);
    }
    RexxStringObject StringUpper(RexxStringObject s)
    {
        return threadContext->StringUpper(s);
    }
    RexxStringObject StringLower(RexxStringObject s)
    {
        return threadContext->StringLower(s);
    }
    logical_t IsString(RexxObjectPtr o)
    {
        return threadContext->IsString(o);
    }

    RexxBufferStringObject NewBufferString(size_t len)
    {
        return threadContext->NewBufferString(len);
    }

    size_t BufferStringLength(RexxBufferStringObject o)
    {
        return threadContext->BufferStringLength(o);
    }

    POINTER BufferStringData(RexxBufferStringObject o)
    {
        return threadContext->BufferStringData(o);
    }

    RexxStringObject FinishBufferString(RexxBufferStringObject o, size_t l)
    {
        return threadContext->FinishBufferString(o, l);
    }

    void DirectoryPut(RexxDirectoryObject diro, RexxObjectPtr o, CSTRING s)
    {
        threadContext->DirectoryPut(diro, o, s);
    }
    RexxObjectPtr DirectoryAt(RexxDirectoryObject to, CSTRING s)
    {
        return threadContext->DirectoryAt(to, s);
    }
    RexxObjectPtr DirectoryRemove(RexxDirectoryObject to, CSTRING s)
    {
        return threadContext->DirectoryRemove(to, s);
    }
    RexxDirectoryObject NewDirectory()
    {
        return threadContext->NewDirectory();
    }
    logical_t IsDirectory(RexxObjectPtr o)
    {
        return threadContext->IsDirectory(o);
    }

    RexxObjectPtr ArrayAt(RexxArrayObject ao, size_t n)
    {
        return threadContext->ArrayAt(ao, n);
    }
    void ArrayPut(RexxArrayObject ao, RexxObjectPtr o, size_t n)
    {
        threadContext->ArrayPut(ao, o, n);
    }
    size_t ArrayAppend(RexxArrayObject ao, RexxObjectPtr o)
    {
        return threadContext->ArrayAppend(ao, o);
    }
    size_t ArrayAppendString(RexxArrayObject ao, CSTRING s, size_t l)
    {
        return threadContext->ArrayAppendString(ao, s, l);
    }
    size_t ArraySize(RexxArrayObject ao)
    {
        return threadContext->ArraySize(ao);
    }
    size_t ArrayItems(RexxArrayObject ao)
    {
        return threadContext->ArrayItems(ao);
    }
    size_t ArrayDimension(RexxArrayObject ao)
    {
        return threadContext->ArrayDimension(ao);
    }
    RexxArrayObject NewArray(size_t n)
    {
        return threadContext->NewArray(n);
    }
    RexxArrayObject ArrayOfOne(RexxObjectPtr o)
    {
        return threadContext->ArrayOfOne(o);
    }
    RexxArrayObject ArrayOfTwo(RexxObjectPtr o1, RexxObjectPtr o2)
    {
        return threadContext->ArrayOfTwo(o1, o2);
    }
    RexxArrayObject ArrayOfThree(RexxObjectPtr o1, RexxObjectPtr o2, RexxObjectPtr o3)
    {
        return threadContext->ArrayOfThree(o1, o2, o3);
    }
    RexxArrayObject ArrayOfFour(RexxObjectPtr o1, RexxObjectPtr o2, RexxObjectPtr o3, RexxObjectPtr o4)
    {
        return threadContext->ArrayOfFour(o1, o2, o3, o4);
    }
    RexxArrayObject Array(RexxObjectPtr o)
    {
        return threadContext->ArrayOfOne(o);
    }
    RexxArrayObject Array(RexxObjectPtr o1, RexxObjectPtr o2)
    {
        return threadContext->ArrayOfTwo(o1, o2);
    }
    RexxArrayObject Array(RexxObjectPtr o1, RexxObjectPtr o2, RexxObjectPtr o3)
    {
        return threadContext->ArrayOfThree(o1, o2, o3);
    }
    RexxArrayObject Array(RexxObjectPtr o1, RexxObjectPtr o2, RexxObjectPtr o3, RexxObjectPtr o4)
    {
        return threadContext->ArrayOfFour(o1, o2, o3, o4);
    }
    logical_t IsArray(RexxObjectPtr o)
    {
        return threadContext->IsArray(o);
    }

    POINTER BufferData(RexxBufferObject bo)
    {
        return threadContext->BufferData(bo);
    }
    size_t BufferLength(RexxBufferObject bo)
    {
        return threadContext->BufferLength(bo);
    }
    RexxBufferObject NewBuffer(wholenumber_t n)
    {
        return threadContext->NewBuffer(n);
    }
    logical_t IsBuffer(RexxObjectPtr o)
    {
        return threadContext->IsBuffer(o);
    }

    POINTER PointerValue(RexxPointerObject po)
    {
        return threadContext->PointerValue(po);
    }
    RexxPointerObject NewPointer(POINTER p)
    {
        return threadContext->NewPointer(p);
    }
    logical_t IsPointer(RexxObjectPtr o)
    {
        return threadContext->IsPointer(o);
    }

    RexxObjectPtr SupplierItem(RexxSupplierObject so)
    {
        return threadContext->SupplierItem(so);
    }
    RexxObjectPtr SupplierIndex(RexxSupplierObject so)
    {
        return threadContext->SupplierIndex(so);
    }
    logical_t SupplierAvailable(RexxSupplierObject so)
    {
        return threadContext->SupplierAvailable(so);
    }
    void SupplierNext(RexxSupplierObject so)
    {
        threadContext->SupplierNext(so);
    }
    RexxSupplierObject NewSupplier(RexxArrayObject values, RexxArrayObject names)
    {
        return threadContext->NewSupplier(values, names);
    }

    RexxStemObject NewStem(CSTRING n)
    {
        return threadContext->NewStem(n);
    }
    void SetStemElement(RexxStemObject so, CSTRING s, RexxObjectPtr o)
    {
        threadContext->SetStemElement(so, s, o);
    }
    RexxObjectPtr GetStemElement(RexxStemObject so, CSTRING s)
    {
        return threadContext->GetStemElement(so, s);
    }
    void DropStemElement(RexxStemObject so, CSTRING s)
    {
        threadContext->DropStemElement(so, s);
    }
    void SetStemArrayElement(RexxStemObject so, size_t n, RexxObjectPtr o)
    {
        threadContext->SetStemArrayElement(so, n, o);
    }
    RexxObjectPtr GetStemArrayElement(RexxStemObject so, size_t n)
    {
        return threadContext->GetStemArrayElement(so, n);
    }
    void DropStemArrayElement(RexxStemObject so, size_t n)
    {
        threadContext->DropStemArrayElement(so, n);
    }
    RexxDirectoryObject GetAllStemElements(RexxStemObject so)
    {
        return threadContext->GetAllStemElements(so);
    }
    RexxObjectPtr GetStemValue(RexxStemObject so)
    {
        return threadContext->GetStemValue(so);
    }
    logical_t IsStem(RexxObjectPtr o)
    {
        return threadContext->IsStem(o);
    }

    void RaiseException0(size_t n)
    {
        threadContext->RaiseException0(n);
    }
    void RaiseException1(size_t n, RexxObjectPtr o)
    {
        threadContext->RaiseException1(n, o);
    }
    void RaiseException2(size_t n, RexxObjectPtr o1, RexxObjectPtr o2)
    {
        threadContext->RaiseException2(n, o1, o2);
    }
    void RaiseException(size_t n, RexxArrayObject ao)
    {
        threadContext->RaiseException(n, ao);
    }
    void RaiseCondition(CSTRING s1, RexxStringObject s2, RexxObjectPtr ao, RexxObjectPtr o)
    {
        threadContext->RaiseCondition(s1, s2, ao, o);
    }
    logical_t CheckCondition()
    {
        return threadContext->CheckCondition();
    }
    RexxDirectoryObject GetConditionInfo()
    {
        return threadContext->GetConditionInfo();
    }
    void DecodeConditionInfo(RexxDirectoryObject diro, RexxCondition *c)
    {
        threadContext->DecodeConditionInfo(diro, c);
    }
    void ClearCondition()
    {
        threadContext->ClearCondition();
    }

    RexxObjectPtr Nil()
    {
        return threadContext->Nil();
    }
    RexxObjectPtr True()
    {
        return threadContext->True();
    }
    RexxObjectPtr False()
    {
        return threadContext->False();
    }
    RexxStringObject NullString()
    {
        return threadContext->NullString();
    }

    RexxArrayObject GetArguments()
    {
        return functions->GetArguments(this);
    }
    RexxObjectPtr GetArgument(size_t n)
    {
        return functions->GetArgument(this, n);
    }
    CSTRING GetRoutineName()
    {
        return functions->GetRoutineName(this);
    }
    RexxRoutineObject GetRoutine()
    {
        return functions->GetRoutine(this);
    }
    void SetContextVariable(CSTRING s, RexxObjectPtr o)
    {
        functions->SetContextVariable(this, s, o);
    }
    RexxObjectPtr GetContextVariable(CSTRING s)
    {
        return functions->GetContextVariable(this, s);
    }
    void DropContextVariable(CSTRING s)
    {
        functions->DropContextVariable(this, s);
    }
    RexxStemObject ResolveStemVariable(RexxObjectPtr v)
    {
        return functions->ResolveStemVariable(this, v);
    }

    RexxDirectoryObject GetAllContextVariables()
    {
        return functions->GetAllContextVariables(this);
    }
    void InvalidRoutine()
    {
        functions->InvalidRoutine(this);
    }
    stringsize_t GetContextDigits()
    {
        return functions->GetContextDigits(this);
    }
    stringsize_t GetContextFuzz()
    {
        return functions->GetContextFuzz(this);
    }
    logical_t GetContextForm()
    {
        return functions->GetContextForm(this);
    }
    RexxObjectPtr GetCallerContext()
    {
        return functions->GetCallerContext(this);
    }
    RexxClassObject FindContextClass(CSTRING n)
    {
        return functions->FindContextClass(this, n);
    }

#endif
};

struct RexxExitContext_
{
    RexxThreadContext *threadContext;   // the interpreter instance state
    ExitContextInterface *functions;    // functions available in a method context
    ValueDescriptor *arguments;         // the argument descriptor

#ifdef __cplusplus
    POINTER GetApplicationData()
    {
        return threadContext->GetApplicationData();
    }
    size_t InterpreterVersion()
    {
        return threadContext->InterpreterVersion();
    }
    size_t LanguageLevel()
    {
        return threadContext->LanguageLevel();
    }
    RexxObjectPtr RequestGlobalReference(RexxObjectPtr o)
    {
        return threadContext->RequestGlobalReference(o);
    }
    void ReleaseGlobalReference(RexxObjectPtr o)
    {
        threadContext->ReleaseGlobalReference(o);
    }
    void ReleaseLocalReference(RexxObjectPtr o)
    {
        threadContext->ReleaseLocalReference(o);
    }

    RexxObjectPtr SendMessage(RexxObjectPtr o, CSTRING s, RexxArrayObject ao)
    {
        return threadContext->SendMessage(o, s, ao);
    }
    RexxObjectPtr SendMessage0(RexxObjectPtr o, CSTRING s)
    {
        return threadContext->SendMessage0(o, s);
    }
    RexxObjectPtr SendMessage1(RexxObjectPtr o, CSTRING s, RexxObjectPtr a1)
    {
        return threadContext->SendMessage1(o, s, a1);
    }
    RexxObjectPtr SendMessage2(RexxObjectPtr o, CSTRING s, RexxObjectPtr a1, RexxObjectPtr a2)
    {
        return threadContext->SendMessage2(o, s, a1, a2);
    }

    RexxDirectoryObject GetLocalEnvironment()
    {
        return threadContext->GetLocalEnvironment();
    }
    RexxDirectoryObject GetGlobalEnvironment()
    {
        return threadContext->GetGlobalEnvironment();
    }

    logical_t IsInstanceOf(RexxObjectPtr o, RexxClassObject co)
    {
        return threadContext->IsInstanceOf(o, co);
    }
    logical_t IsOfType(RexxObjectPtr o, CSTRING cn)
    {
        return threadContext->IsOfType(o, cn);
    }
    RexxClassObject FindClass(CSTRING s)
    {
        return threadContext->FindClass(s);
    }
    RexxClassObject FindPackageClass(RexxPackageObject m, CSTRING n)
    {
        return threadContext->FindPackageClass(m, n);
    }
    logical_t HasMethod(RexxObjectPtr o, CSTRING m)
    {
        return threadContext->HasMethod(o, m);
    }

    RexxMethodObject NewMethod(CSTRING n, CSTRING s, size_t l)
    {
        return threadContext->NewMethod(n, s, l);
    }
    RexxRoutineObject NewRoutine(CSTRING n, CSTRING s, size_t l)
    {
        return threadContext->NewRoutine(n, s, l);
    }
    logical_t IsRoutine(RexxObjectPtr o)
    {
        return threadContext->IsRoutine(o);
    }
    logical_t IsMethod(RexxObjectPtr o)
    {
        return threadContext->IsMethod(o);
    }
    RexxPackageObject GetRoutinePackage(RexxRoutineObject o)
    {
        return threadContext->GetRoutinePackage(o);
    }
    RexxPackageObject GetMethodPackage(RexxMethodObject o)
    {
        return threadContext->GetMethodPackage(o);
    }

    RexxDirectoryObject GetPackageRoutines(RexxPackageObject m)
    {
        return threadContext->GetPackageRoutines(m);
    }
    RexxDirectoryObject GetPackagePublicRoutines(RexxPackageObject m)
    {
        return threadContext->GetPackagePublicRoutines(m);
    }
    RexxDirectoryObject GetPackageClasses(RexxPackageObject m)
    {
        return threadContext->GetPackageClasses(m);
    }
    RexxDirectoryObject GetPackagePublicClasses(RexxPackageObject m)
    {
        return threadContext->GetPackagePublicClasses(m);
    }
    RexxDirectoryObject GetPackageMethods(RexxPackageObject m)
    {
        return threadContext->GetPackageMethods(m);
    }
    RexxObjectPtr CallRoutine(RexxRoutineObject m, RexxArrayObject a)
    {
        return threadContext->CallRoutine(m, a);
    }
    RexxObjectPtr CallProgram(CSTRING n,RexxArrayObject a)
    {
        return threadContext->CallProgram(n, a);
    }
    RexxPackageObject LoadPackage(CSTRING d)
    {
        return threadContext->LoadPackage(d);
    }
    RexxPackageObject LoadPackageFromData(CSTRING n, CSTRING d, size_t l)
    {
        return threadContext->LoadPackageFromData(n, d, l);
    }
    logical_t LoadLibrary(CSTRING n)
    {
        return threadContext->LoadLibrary(n);
    }
    logical_t RegisterLibrary(CSTRING n, RexxPackageEntry *e)
    {
        return threadContext->RegisterLibrary(n, e);
    }
    POINTER ObjectToCSelf(RexxObjectPtr o)
    {
        return threadContext->ObjectToCSelf(o);
    }
    RexxObjectPtr WholeNumberToObject(wholenumber_t n)
    {
        return threadContext->WholeNumberToObject(n);
    }
    RexxObjectPtr WholeNumber(wholenumber_t n)
    {
        return threadContext->WholeNumberToObject(n);
    }
    RexxObjectPtr UintptrToObject(uintptr_t n)
    {
        return threadContext->UintptrToObject(n);
    }
    RexxObjectPtr Uintptr(uintptr_t n)
    {
        return threadContext->UintptrToObject(n);
    }
    RexxObjectPtr IntptrToObject(intptr_t n)
    {
        return threadContext->IntptrToObject(n);
    }
    RexxObjectPtr Intptr(intptr_t n)
    {
        return threadContext->IntptrToObject(n);
    }
    RexxObjectPtr ValueToObject(ValueDescriptor *v)
    {
        return threadContext->ValueToObject(v);
    }
    RexxArrayObject ValuesToObject(ValueDescriptor *v, size_t c)
    {
        return threadContext->ValuesToObject(v, c);
    }
    logical_t ObjectToValue(RexxObjectPtr o, ValueDescriptor *v)
    {
        return threadContext->ObjectToValue(o, v);
    }
    RexxObjectPtr StringSizeToObject(size_t u)
    {
        return threadContext->StringSizeToObject(u);
    }
    RexxObjectPtr StringSize(size_t u)
    {
        return threadContext->StringSizeToObject(u);
    }
    logical_t ObjectToWholeNumber(RexxObjectPtr o, wholenumber_t *n)
    {
        return threadContext->ObjectToWholeNumber(o, n);
    }
    logical_t WholeNumber(RexxObjectPtr o, wholenumber_t *n)
    {
        return threadContext->ObjectToWholeNumber(o, n);
    }
    logical_t ObjectToStringSize(RexxObjectPtr o, size_t *n)
    {
        return threadContext->ObjectToStringSize(o, n);
    }
    logical_t StringSize(RexxObjectPtr o, size_t *n)
    {
        return threadContext->ObjectToStringSize(o, n);
    }
    RexxObjectPtr Int64ToObject(int64_t i)
    {
        return threadContext->Int64ToObject(i);
    }
    RexxObjectPtr Int64(int64_t i)
    {
        return threadContext->Int64ToObject(i);
    }
    RexxObjectPtr UnsignedInt64ToObject(uint64_t u)
    {
        return threadContext->UnsignedInt64ToObject(u);
    }
    RexxObjectPtr UnsignedInt64(uint64_t u)
    {
        return threadContext->UnsignedInt64ToObject(u);
    }
    logical_t ObjectToInt64(RexxObjectPtr o, int64_t *i)
    {
        return threadContext->ObjectToInt64(o, i);
    }
    logical_t Int64(RexxObjectPtr o, int64_t *i)
    {
        return threadContext->ObjectToInt64(o, i);
    }
    logical_t ObjectToUnsignedInt64(RexxObjectPtr o, uint64_t *u)
    {
        return threadContext->ObjectToUnsignedInt64(o, u);
    }
    logical_t UnsignedInt64(RexxObjectPtr o, uint64_t *u)
    {
        return threadContext->ObjectToUnsignedInt64(o, u);
    }
    RexxObjectPtr Int32ToObject(int32_t i)
    {
        return threadContext->Int32ToObject(i);
    }
    RexxObjectPtr Int32(int32_t i)
    {
        return threadContext->Int32ToObject(i);
    }
    RexxObjectPtr UnsignedInt32ToObject(uint32_t u)
    {
        return threadContext->UnsignedInt32ToObject(u);
    }
    RexxObjectPtr UnsignedInt32(uint32_t u)
    {
        return threadContext->UnsignedInt32ToObject(u);
    }
    logical_t ObjectToInt32(RexxObjectPtr o, int32_t *i)
    {
        return threadContext->ObjectToInt32(o, i);
    }
    logical_t Int32(RexxObjectPtr o, int32_t *i)
    {
        return threadContext->ObjectToInt32(o, i);
    }
    logical_t ObjectToUnsignedInt32(RexxObjectPtr o, uint32_t *u)
    {
        return threadContext->ObjectToUnsignedInt32(o, u);
    }
    logical_t UnsignedInt32(RexxObjectPtr o, uint32_t *u)
    {
        return threadContext->ObjectToUnsignedInt32(o, u);
    }
    logical_t ObjectToUintptr(RexxObjectPtr o, uintptr_t *n)
    {
        return threadContext->ObjectToUintptr(o, n);
    }
    logical_t Uintptr(RexxObjectPtr o, uintptr_t *n)
    {
        return threadContext->ObjectToUintptr(o, n);
    }
    logical_t ObjectToIntptr(RexxObjectPtr o, intptr_t *n)
    {
        return threadContext->ObjectToIntptr(o, n);
    }
    logical_t Intptr(RexxObjectPtr o, intptr_t *n)
    {
        return threadContext->ObjectToIntptr(o, n);
    }
    logical_t ObjectToLogical(RexxObjectPtr o, logical_t *n)
    {
        return threadContext->ObjectToLogical(o, n);
    }
    logical_t Logical(RexxObjectPtr o, logical_t *n)
    {
        return threadContext->ObjectToLogical(o, n);
    }
    RexxObjectPtr LogicalToObject(logical_t l)
    {
        return threadContext->LogicalToObject(l);
    }
    RexxObjectPtr Logical(logical_t l)
    {
        return threadContext->LogicalToObject(l);
    }
    RexxObjectPtr DoubleToObject(double d)
    {
        return threadContext->DoubleToObject(d);
    }
    RexxObjectPtr Double(double d)
    {
        return threadContext->DoubleToObject(d);
    }
    RexxObjectPtr DoubleToObjectWithPrecision(double d, size_t precision)
    {
        return threadContext->DoubleToObjectWithPrecision(d, precision);
    }
    logical_t ObjectToDouble(RexxObjectPtr o, double *d)
    {
        return threadContext->ObjectToDouble(o, d);
    }
    logical_t Double(RexxObjectPtr o, double *d)
    {
        return threadContext->ObjectToDouble(o, d);
    }

    RexxStringObject ObjectToString(RexxObjectPtr o)
    {
        return threadContext->ObjectToString(o);
    }
    CSTRING ObjectToStringValue(RexxObjectPtr o)
    {
        return threadContext->ObjectToStringValue(o);
    }
    CSTRING CString(RexxObjectPtr o)
    {
        return threadContext->ObjectToStringValue(o);
    }
    size_t StringGet(RexxStringObject o, size_t n1, POINTER s, size_t n2)
    {
        return threadContext->StringGet(o, n1, s, n2);
    }
    size_t StringLength(RexxStringObject o)
    {
        return threadContext->StringLength(o);
    }
    CSTRING StringData(RexxStringObject o)
    {
        return threadContext->StringData(o);
    }
    RexxStringObject NewString(CSTRING s, size_t len)
    {
        return threadContext->NewString(s, len);
    }
    RexxStringObject NewStringFromAsciiz(CSTRING s)
    {
        return threadContext->NewStringFromAsciiz(s);
    }
    RexxStringObject String(CSTRING s, size_t len)
    {
        return threadContext->NewString(s, len);
    }
    RexxStringObject String(CSTRING s)
    {
        return threadContext->NewStringFromAsciiz(s);
    }
    RexxStringObject CString(CSTRING s)
    {
        return threadContext->NewStringFromAsciiz(s);
    }
    RexxStringObject StringUpper(RexxStringObject s)
    {
        return threadContext->StringUpper(s);
    }
    RexxStringObject StringLower(RexxStringObject s)
    {
        return threadContext->StringLower(s);
    }
    logical_t IsString(RexxObjectPtr o)
    {
        return threadContext->IsString(o);
    }

    RexxBufferStringObject NewBufferString(size_t len)
    {
        return threadContext->NewBufferString(len);
    }

    size_t BufferStringLength(RexxBufferStringObject o)
    {
        return threadContext->BufferStringLength(o);
    }

    POINTER BufferStringData(RexxBufferStringObject o)
    {
        return threadContext->BufferStringData(o);
    }

    RexxStringObject FinishBufferString(RexxBufferStringObject o, size_t l)
    {
        return threadContext->FinishBufferString(o, l);
    }

    void DirectoryPut(RexxDirectoryObject diro, RexxObjectPtr o, CSTRING s)
    {
        threadContext->DirectoryPut(diro, o, s);
    }
    RexxObjectPtr DirectoryAt(RexxDirectoryObject to, CSTRING s)
    {
        return threadContext->DirectoryAt(to, s);
    }
    RexxObjectPtr DirectoryRemove(RexxDirectoryObject to, CSTRING s)
    {
        return threadContext->DirectoryRemove(to, s);
    }
    RexxDirectoryObject NewDirectory()
    {
        return threadContext->NewDirectory();
    }
    logical_t IsDirectory(RexxObjectPtr o)
    {
        return threadContext->IsDirectory(o);
    }

    RexxObjectPtr ArrayAt(RexxArrayObject ao, size_t n)
    {
        return threadContext->ArrayAt(ao, n);
    }
    void ArrayPut(RexxArrayObject ao, RexxObjectPtr o, size_t n)
    {
        threadContext->ArrayPut(ao, o, n);
    }
    size_t ArrayAppend(RexxArrayObject ao, RexxObjectPtr o)
    {
        return threadContext->ArrayAppend(ao, o);
    }
    size_t ArrayAppendString(RexxArrayObject ao, CSTRING s, size_t l)
    {
        return threadContext->ArrayAppendString(ao, s, l);
    }
    size_t ArraySize(RexxArrayObject ao)
    {
        return threadContext->ArraySize(ao);
    }
    size_t ArrayItems(RexxArrayObject ao)
    {
        return threadContext->ArrayItems(ao);
    }
    size_t ArrayDimension(RexxArrayObject ao)
    {
        return threadContext->ArrayDimension(ao);
    }
    RexxArrayObject NewArray(size_t n)
    {
        return threadContext->NewArray(n);
    }
    RexxArrayObject ArrayOfOne(RexxObjectPtr o)
    {
        return threadContext->ArrayOfOne(o);
    }
    RexxArrayObject ArrayOfTwo(RexxObjectPtr o1, RexxObjectPtr o2)
    {
        return threadContext->ArrayOfTwo(o1, o2);
    }
    RexxArrayObject ArrayOfThree(RexxObjectPtr o1, RexxObjectPtr o2, RexxObjectPtr o3)
    {
        return threadContext->ArrayOfThree(o1, o2, o3);
    }
    RexxArrayObject ArrayOfFour(RexxObjectPtr o1, RexxObjectPtr o2, RexxObjectPtr o3, RexxObjectPtr o4)
    {
        return threadContext->ArrayOfFour(o1, o2, o3, o4);
    }
    RexxArrayObject Array(RexxObjectPtr o)
    {
        return threadContext->ArrayOfOne(o);
    }
    RexxArrayObject Array(RexxObjectPtr o1, RexxObjectPtr o2)
    {
        return threadContext->ArrayOfTwo(o1, o2);
    }
    RexxArrayObject Array(RexxObjectPtr o1, RexxObjectPtr o2, RexxObjectPtr o3)
    {
        return threadContext->ArrayOfThree(o1, o2, o3);
    }
    RexxArrayObject Array(RexxObjectPtr o1, RexxObjectPtr o2, RexxObjectPtr o3, RexxObjectPtr o4)
    {
        return threadContext->ArrayOfFour(o1, o2, o3, o4);
    }
    logical_t IsArray(RexxObjectPtr o)
    {
        return threadContext->IsArray(o);
    }

    POINTER BufferData(RexxBufferObject bo)
    {
        return threadContext->BufferData(bo);
    }
    size_t BufferLength(RexxBufferObject bo)
    {
        return threadContext->BufferLength(bo);
    }
    RexxBufferObject NewBuffer(wholenumber_t n)
    {
        return threadContext->NewBuffer(n);
    }
    logical_t IsBuffer(RexxObjectPtr o)
    {
        return threadContext->IsBuffer(o);
    }

    POINTER PointerValue(RexxPointerObject po)
    {
        return threadContext->PointerValue(po);
    }
    RexxPointerObject NewPointer(POINTER p)
    {
        return threadContext->NewPointer(p);
    }
    logical_t IsPointer(RexxObjectPtr o)
    {
        return threadContext->IsPointer(o);
    }

    RexxObjectPtr SupplierItem(RexxSupplierObject so)
    {
        return threadContext->SupplierItem(so);
    }
    RexxObjectPtr SupplierIndex(RexxSupplierObject so)
    {
        return threadContext->SupplierIndex(so);
    }
    logical_t SupplierAvailable(RexxSupplierObject so)
    {
        return threadContext->SupplierAvailable(so);
    }
    void SupplierNext(RexxSupplierObject so)
    {
        threadContext->SupplierNext(so);
    }
    RexxSupplierObject NewSupplier(RexxArrayObject values, RexxArrayObject names)
    {
        return threadContext->NewSupplier(values, names);
    }

    RexxStemObject NewStem(CSTRING n)
    {
        return threadContext->NewStem(n);
    }
    void SetStemElement(RexxStemObject so, CSTRING s, RexxObjectPtr o)
    {
        threadContext->SetStemElement(so, s, o);
    }
    RexxObjectPtr GetStemElement(RexxStemObject so, CSTRING s)
    {
        return threadContext->GetStemElement(so, s);
    }
    void DropStemElement(RexxStemObject so, CSTRING s)
    {
        threadContext->DropStemElement(so, s);
    }
    void SetStemArrayElement(RexxStemObject so, size_t n, RexxObjectPtr o)
    {
        threadContext->SetStemArrayElement(so, n, o);
    }
    RexxObjectPtr GetStemArrayElement(RexxStemObject so, size_t n)
    {
        return threadContext->GetStemArrayElement(so, n);
    }
    void DropStemArrayElement(RexxStemObject so, size_t n)
    {
        threadContext->DropStemArrayElement(so, n);
    }
    RexxDirectoryObject GetAllStemElements(RexxStemObject so)
    {
        return threadContext->GetAllStemElements(so);
    }
    RexxObjectPtr GetStemValue(RexxStemObject so)
    {
        return threadContext->GetStemValue(so);
    }
    logical_t IsStem(RexxObjectPtr o)
    {
        return threadContext->IsStem(o);
    }

    void RaiseException0(size_t n)
    {
        threadContext->RaiseException0(n);
    }
    void RaiseException1(size_t n, RexxObjectPtr o)
    {
        threadContext->RaiseException1(n, o);
    }
    void RaiseException2(size_t n, RexxObjectPtr o1, RexxObjectPtr o2)
    {
        threadContext->RaiseException2(n, o1, o2);
    }
    void RaiseException(size_t n, RexxArrayObject ao)
    {
        threadContext->RaiseException(n, ao);
    }
    void RaiseCondition(CSTRING s1, RexxStringObject s2, RexxObjectPtr ao, RexxObjectPtr o)
    {
        threadContext->RaiseCondition(s1, s2, ao, o);
    }
    logical_t CheckCondition()
    {
        return threadContext->CheckCondition();
    }
    RexxDirectoryObject GetConditionInfo()
    {
        return threadContext->GetConditionInfo();
    }
    void DecodeConditionInfo(RexxDirectoryObject diro, RexxCondition *c)
    {
        threadContext->DecodeConditionInfo(diro, c);
    }
    void ClearCondition()
    {
        threadContext->ClearCondition();
    }

    RexxObjectPtr Nil()
    {
        return threadContext->Nil();
    }
    RexxObjectPtr True()
    {
        return threadContext->True();
    }
    RexxObjectPtr False()
    {
        return threadContext->False();
    }
    RexxStringObject NullString()
    {
        return threadContext->NullString();
    }
    void SetContextVariable(CSTRING s, RexxObjectPtr o)
    {
        functions->SetContextVariable(this, s, o);
    }
    RexxObjectPtr GetContextVariable(CSTRING s)
    {
        return functions->GetContextVariable(this, s);
    }
    void DropContextVariable(CSTRING s)
    {
        functions->DropContextVariable(this, s);
    }
    RexxDirectoryObject GetAllContextVariables()
    {
        return functions->GetAllContextVariables(this);
    }
    RexxObjectPtr GetCallerContext()
    {
        return functions->GetCallerContext(this);
    }
#endif
};

BEGIN_EXTERN_C()

RexxReturnCode RexxEntry RexxCreateInterpreter(RexxInstance **, RexxThreadContext **, RexxOption *);

END_EXTERN_C()

#define ARGUMENT_TYPE_ARGLIST    RexxArrayObject
#define ARGUMENT_TYPE_NAME       CSTRING
#define ARGUMENT_TYPE_SCOPE      RexxObjectPtr
#define ARGUMENT_TYPE_CSELF      POINTER
#define ARGUMENT_TYPE_OSELF      RexxObjectPtr
#define ARGUMENT_TYPE_SUPER      RexxClassObject

// each of the following types have an optional equivalent

#define ARGUMENT_TYPE_RexxObjectPtr         RexxObjectPtr
#define ARGUMENT_TYPE_RexxClassObject       RexxClassObject
#define ARGUMENT_TYPE_int                   int
#define ARGUMENT_TYPE_wholenumber_t         wholenumber_t
#define ARGUMENT_TYPE_stringsize_t          stringsize_t
#define ARGUMENT_TYPE_double                double
#define ARGUMENT_TYPE_CSTRING               CSTRING
#define ARGUMENT_TYPE_POINTER               POINTER
#define ARGUMENT_TYPE_RexxStringObject      RexxStringObject
#define ARGUMENT_TYPE_float                 float
#define ARGUMENT_TYPE_int8_t                int8_t
#define ARGUMENT_TYPE_int16_t               int16_t
#define ARGUMENT_TYPE_int32_t               int32_t
#define ARGUMENT_TYPE_int64_t               int64_t
#define ARGUMENT_TYPE___int64_t              int64_t
#define ARGUMENT_TYPE_uint8_t               uint8_t
#define ARGUMENT_TYPE_uint16_t              uint16_t
#define ARGUMENT_TYPE_uint32_t              uint32_t
#define ARGUMENT_TYPE_uint64_t              uint64_t
#define ARGUMENT_TYPE___uint64_t             uint64_t
#define ARGUMENT_TYPE_size_t                size_t
#define ARGUMENT_TYPE_ssize_t               ssize_t
#define ARGUMENT_TYPE_intptr_t              intptr_t
#define ARGUMENT_TYPE_uintptr_t             uintptr_t
#define ARGUMENT_TYPE___uintptr_t             uintptr_t
#define ARGUMENT_TYPE_logical_t             logical_t
#define ARGUMENT_TYPE_RexxArrayObject       RexxArrayObject
#define ARGUMENT_TYPE_RexxStemObject        RexxStemObject
#define ARGUMENT_TYPE_POINTERSTRING         POINTER

#define ARGUMENT_TYPE_OPTIONAL_RexxObjectPtr         RexxObjectPtr
#define ARGUMENT_TYPE_OPTIONAL_int                   int
#define ARGUMENT_TYPE_OPTIONAL_wholenumber_t         wholenumber_t
#define ARGUMENT_TYPE_OPTIONAL_stringsize_t          stringsize_t
#define ARGUMENT_TYPE_OPTIONAL_double                double
#define ARGUMENT_TYPE_OPTIONAL_CSTRING               CSTRING
#define ARGUMENT_TYPE_OPTIONAL_POINTER               POINTER
#define ARGUMENT_TYPE_OPTIONAL_RexxStringObject      RexxStringObject
#define ARGUMENT_TYPE_OPTIONAL_float                 float
#define ARGUMENT_TYPE_OPTIONAL_int8_t                int8_t
#define ARGUMENT_TYPE_OPTIONAL_int16_t               int16_t
#define ARGUMENT_TYPE_OPTIONAL_int32_t               int32_t
#define ARGUMENT_TYPE_OPTIONAL_int64_t               int64_t
#define ARGUMENT_TYPE_OPTIONAL_uint8_t               uint8_t
#define ARGUMENT_TYPE_OPTIONAL_uint16_t              uint16_t
#define ARGUMENT_TYPE_OPTIONAL_uint32_t              uint32_t
#define ARGUMENT_TYPE_OPTIONAL_uint64_t              uint64_t
#define ARGUMENT_TYPE_OPTIONAL_size_t                size_t
#define ARGUMENT_TYPE_OPTIONAL_ssize_t               ssize_t
#define ARGUMENT_TYPE_OPTIONAL_intptr_t              intptr_t
#define ARGUMENT_TYPE_OPTIONAL_uintptr_t             uintptr_t
#define ARGUMENT_TYPE_OPTIONAL_logical_t             logical_t
#define ARGUMENT_TYPE_OPTIONAL_RexxArrayObject       RexxArrayObject
#define ARGUMENT_TYPE_OPTIONAL_RexxStemObject        RexxStemObject
#define ARGUMENT_TYPE_OPTIONAL_POINTERSTRING         POINTER
#define ARGUMENT_TYPE_OPTIONAL_RexxClassObject       RexxClassObject

#define ARGUMENT_TYPE(t) ((t) & ~REXX_OPTIONAL_ARGUMENT)
#define IS_OPTIONAL_ARGUMENT(t) (((t) & REXX_OPTIONAL_ARGUMENT) != 0)


#define argumentExists(i) ((context->arguments[i].flags & ARGUMENT_EXISTS) != 0)
#define argumentOmitted(i) (!argumentExists(i))


#define __type(t)   ARGUMENT_TYPE_##t
#define __arg(p, t) arguments[p].value.value_##t
#define __ret(t, v) arguments[0].value.value_##t = (v); return NULL;
#define __adcl(t, n) __type(t) n
#define __tdcl(t)    REXX_VALUE_##t

#define __methodstub(name) uint16_t * RexxEntry name (RexxMethodContext *context, ValueDescriptor *arguments)

#ifdef __cplusplus
#define __cpp_method_proto(name) extern "C" __methodstub(name);
#else
#define __cpp_method_proto(name) __methodstub(name);
#endif


// macro to simply the process of a setting a value descriptor
#define SET_REXX_VALUE(v, t, val)    \
{                               \
    (v).type = REXX_VALUE##t;   \
    (v).value.value_##t = (val);\
}

// macro to simplify getting a value from a value descriptor
#define GET_REXX_VALUE(v, t) ((v).value.value_##t)


#define REXX_METHOD_PROTOTYPE(name) __cpp_method_proto(name)

// zero argument method call

#define RexxMethod0(returnType, name) \
/* forward reference definition for method */ \
__type(returnType) name##_impl (RexxMethodContext * context);  \
                               \
/* method signature definition */ \
static uint16_t name##_types[] = {__tdcl(returnType), REXX_ARGUMENT_TERMINATOR};    \
\
__cpp_method_proto(name) \
/* generated calling stub function */ \
__methodstub(name) \
{ \
    if (arguments != NULL) /* if no arguments passed, this a signature request */ \
    {                                                                             \
        /* forward to the method implementation */                                \
        __ret(returnType, (name##_impl(context)));                                \
    }                                                                             \
    return name##_types;     /* return the type signature */                      \
} \
/* the real target method code */  \
__type(returnType) name##_impl(RexxMethodContext *context)


// method with one argument
#define RexxMethod1(returnType ,name, t1, n1) \
/* forward reference definition for method */ \
__type(returnType) name##_impl (RexxMethodContext * context, __adcl(t1, n1));  \
                               \
/* method signature definition */ \
static uint16_t name##_types[] = {__tdcl(returnType), __tdcl(t1), REXX_ARGUMENT_TERMINATOR};    \
\
__cpp_method_proto(name) \
/* generated calling stub function */ \
__methodstub(name) \
{ \
    if (arguments != NULL) /* if no arguments passed, this a signature request */ \
    {                                                                             \
        /* forward to the method implementation */                                \
        __ret(returnType, name##_impl(context, __arg(1, t1)));                    \
    }                                                                             \
    return name##_types;     /* return the type signature */                      \
} \
/* the real target method code */  \
__type(returnType) name##_impl(RexxMethodContext *context, __adcl(t1, n1))


// method with two arguments
#define RexxMethod2(returnType ,name, t1, n1, t2, n2) \
/* forward reference definition for method */ \
__type(returnType) name##_impl (RexxMethodContext * context, __adcl(t1, n1), __adcl(t2, n2));  \
                               \
/* method signature definition */ \
static uint16_t name##_types[] = {__tdcl(returnType), __tdcl(t1), __tdcl(t2), REXX_ARGUMENT_TERMINATOR};    \
\
__cpp_method_proto(name) \
/* generated calling stub function */ \
__methodstub(name) \
{ \
    if (arguments != NULL) /* if no arguments passed, this a signature request */ \
    {                                                                             \
        /* forward to the method implementation */                                \
        __ret(returnType, name##_impl(context, __arg(1, t1), __arg(2, t2)));                    \
    }                                                                             \
    return name##_types;     /* return the type signature */                      \
} \
/* the real target method code */  \
__type(returnType) name##_impl(RexxMethodContext *context, __adcl(t1, n1), __adcl(t2, n2))


// method with three arguments
#define RexxMethod3(returnType ,name, t1, n1, t2, n2, t3, n3) \
/* forward reference definition for method */ \
__type(returnType) name##_impl (RexxMethodContext * context, __adcl(t1, n1), __adcl(t2, n2), __adcl(t3, n3));  \
                               \
/* method signature definition */ \
static uint16_t name##_types[] = {__tdcl(returnType), __tdcl(t1), __tdcl(t2), __tdcl(t3), REXX_ARGUMENT_TERMINATOR};    \
\
__cpp_method_proto(name) \
/* generated calling stub function */ \
__methodstub(name) \
{ \
    if (arguments != NULL) /* if no arguments passed, this a signature request */ \
    {                                                                             \
        /* forward to the method implementation */                                \
        __ret(returnType, name##_impl(context, __arg(1, t1), __arg(2, t2), __arg(3, t3)));      \
    }                                                                             \
    return name##_types;     /* return the type signature */                      \
} \
/* the real target method code */  \
__type(returnType) name##_impl(RexxMethodContext *context, __adcl(t1, n1), __adcl(t2, n2), __adcl(t3, n3))


// method with four arguments
#define RexxMethod4(returnType ,name, t1, n1, t2, n2, t3, n3, t4, n4) \
/* forward reference definition for method */ \
__type(returnType) name##_impl (RexxMethodContext * context, __adcl(t1, n1), __adcl(t2, n2), __adcl(t3, n3), __adcl(t4, n4));  \
                               \
/* method signature definition */ \
static uint16_t name##_types[] = {__tdcl(returnType), __tdcl(t1), __tdcl(t2), __tdcl(t3), __tdcl(t4), REXX_ARGUMENT_TERMINATOR};    \
\
__cpp_method_proto(name) \
/* generated calling stub function */ \
__methodstub(name) \
{ \
    if (arguments != NULL) /* if no arguments passed, this a signature request */ \
    {                                                                             \
        /* forward to the method implementation */                                \
        __ret(returnType, name##_impl(context, __arg(1, t1), __arg(2, t2), __arg(3, t3), __arg(4, t4)));      \
    }                                                                             \
    return name##_types;     /* return the type signature */                      \
} \
/* the real target method code */  \
__type(returnType) name##_impl(RexxMethodContext *context, __adcl(t1, n1), __adcl(t2, n2), __adcl(t3, n3), __adcl(t4, n4))


// method with five arguments
#define RexxMethod5(returnType ,name, t1, n1, t2, n2, t3, n3, t4, n4, t5, n5) \
/* forward reference definition for method */ \
__type(returnType) name##_impl (RexxMethodContext * context, __adcl(t1, n1), __adcl(t2, n2), __adcl(t3, n3), __adcl(t4, n4), __adcl(t5, n5));  \
                               \
/* method signature definition */ \
static uint16_t name##_types[] = {__tdcl(returnType), __tdcl(t1), __tdcl(t2), __tdcl(t3), __tdcl(t4), __tdcl(t5), REXX_ARGUMENT_TERMINATOR};    \
\
__cpp_method_proto(name) \
/* generated calling stub function */ \
__methodstub(name) \
{ \
    if (arguments != NULL) /* if no arguments passed, this a signature request */ \
    {                                                                             \
        /* forward to the method implementation */                                \
        __ret(returnType, name##_impl(context, __arg(1, t1), __arg(2, t2), __arg(3, t3), __arg(4, t4), __arg(5, t5)));      \
    }                                                                             \
    return name##_types;     /* return the type signature */                      \
} \
/* the real target method code */  \
__type(returnType) name##_impl(RexxMethodContext *context, __adcl(t1, n1), __adcl(t2, n2), __adcl(t3, n3), __adcl(t4, n4), __adcl(t5, n5))


// method with six arguments
#define RexxMethod6(returnType, name, t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6) \
/* forward reference definition for method */ \
__type(returnType) name##_impl (RexxMethodContext * context, __adcl(t1, n1), __adcl(t2, n2), __adcl(t3, n3), __adcl(t4, n4), __adcl(t5, n5), __adcl(t6, n6));  \
                               \
/* method signature definition */ \
static uint16_t name##_types[] = {__tdcl(returnType), __tdcl(t1), __tdcl(t2), __tdcl(t3), __tdcl(t4), __tdcl(t5), __tdcl(t6), REXX_ARGUMENT_TERMINATOR};    \
\
__cpp_method_proto(name) \
/* generated calling stub function */ \
__methodstub(name) \
{ \
    if (arguments != NULL) /* if no arguments passed, this a signature request */ \
    {                                                                             \
        /* forward to the method implementation */                                \
        __ret(returnType, name##_impl(context, __arg(1, t1), __arg(2, t2), __arg(3, t3), __arg(4, t4), __arg(5, t5), __arg(6, t6)));      \
    }                                                                             \
    return name##_types;     /* return the type signature */                      \
} \
/* the real target method code */  \
__type(returnType) name##_impl(RexxMethodContext *context, __adcl(t1, n1), __adcl(t2, n2), __adcl(t3, n3), __adcl(t4, n4), __adcl(t5, n5), __adcl(t6, n6))


// method with seven arguments
#define RexxMethod7(returnType, name, t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7) \
/* forward reference definition for method */ \
__type(returnType) name##_impl (RexxMethodContext * context, __adcl(t1, n1), __adcl(t2, n2), __adcl(t3, n3), __adcl(t4, n4), __adcl(t5, n5), __adcl(t6, n6), __adcl(t7, n7));  \
                               \
/* method signature definition */ \
static uint16_t name##_types[] = {__tdcl(returnType), __tdcl(t1), __tdcl(t2), __tdcl(t3), __tdcl(t4), __tdcl(t5), __tdcl(t6), __tdcl(t7), REXX_ARGUMENT_TERMINATOR};    \
\
__cpp_method_proto(name) \
/* generated calling stub function */ \
__methodstub(name) \
{ \
    if (arguments != NULL) /* if no arguments passed, this a signature request */ \
    {                                                                             \
        /* forward to the method implementation */                                \
        __ret(returnType, name##_impl(context, __arg(1, t1), __arg(2, t2), __arg(3, t3), __arg(4, t4), __arg(5, t5), __arg(6, t6), __arg(7, t7)));      \
    }                                                                             \
    return name##_types;     /* return the type signature */                      \
} \
/* the real target method code */  \
__type(returnType) name##_impl(RexxMethodContext *context, __adcl(t1, n1), __adcl(t2, n2), __adcl(t3, n3), __adcl(t4, n4), __adcl(t5, n5), __adcl(t6, n6), __adcl(t7, n7))


// method with eight arguments
#define RexxMethod8(returnType, name, t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8) \
/* forward reference definition for method */ \
__type(returnType) name##_impl (RexxMethodContext * context, __adcl(t1, n1), __adcl(t2, n2), __adcl(t3, n3), __adcl(t4, n4), __adcl(t5, n5), __adcl(t6, n6), __adcl(t7, n7), __adcl(t8, n8));  \
                               \
/* method signature definition */ \
static uint16_t name##_types[] = {__tdcl(returnType), __tdcl(t1), __tdcl(t2), __tdcl(t3), __tdcl(t4), __tdcl(t5), __tdcl(t6), __tdcl(t7), __tdcl(t8), REXX_ARGUMENT_TERMINATOR};    \
\
__cpp_method_proto(name) \
/* generated calling stub function */ \
__methodstub(name) \
{ \
    if (arguments != NULL) /* if no arguments passed, this a signature request */ \
    {                                                                             \
        /* forward to the method implementation */                                \
        __ret(returnType, name##_impl(context, __arg(1, t1), __arg(2, t2), __arg(3, t3), __arg(4, t4), __arg(5, t5), __arg(6, t6), __arg(7, t7), __arg(8, t8)));      \
    }                                                                             \
    return name##_types;     /* return the type signature */                      \
} \
/* the real target method code */  \
__type(returnType) name##_impl(RexxMethodContext *context, __adcl(t1, n1), __adcl(t2, n2), __adcl(t3, n3), __adcl(t4, n4), __adcl(t5, n5), __adcl(t6, n6), __adcl(t7, n7), __adcl(t8, n8))


// method with nine arguments
#define RexxMethod9(returnType, name, t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8, t9, n9) \
/* forward reference definition for method */ \
__type(returnType) name##_impl (RexxMethodContext * context, __adcl(t1, n1), __adcl(t2, n2), __adcl(t3, n3), __adcl(t4, n4), __adcl(t5, n5), __adcl(t6, n6), __adcl(t7, n7), __adcl(t8, n8), __adcl(t9, n9));  \
                               \
/* method signature definition */ \
static uint16_t name##_types[] = {__tdcl(returnType), __tdcl(t1), __tdcl(t2), __tdcl(t3), __tdcl(t4), __tdcl(t5), __tdcl(t6), __tdcl(t7), __tdcl(t8), __tdcl(t9), REXX_ARGUMENT_TERMINATOR};    \
\
__cpp_method_proto(name) \
/* generated calling stub function */ \
__methodstub(name) \
{ \
    if (arguments != NULL) /* if no arguments passed, this a signature request */ \
    {                                                                             \
        /* forward to the method implementation */                                \
        __ret(returnType, name##_impl(context, __arg(1, t1), __arg(2, t2), __arg(3, t3), __arg(4, t4), __arg(5, t5), __arg(6, t6), __arg(7, t7), __arg(8, t8), __arg(9, t9)));      \
    }                                                                             \
    return name##_types;     /* return the type signature */                      \
} \
/* the real target method code */  \
__type(returnType) name##_impl(RexxMethodContext *context, __adcl(t1, n1), __adcl(t2, n2), __adcl(t3, n3), __adcl(t4, n4), __adcl(t5, n5), __adcl(t6, n6), __adcl(t7, n7), __adcl(t8, n8), __adcl(t9, n9))


// method with 10 arguments
#define RexxMethod10(returnType, name, t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8, t9, n9, t10, n10) \
/* forward reference definition for method */ \
__type(returnType) name##_impl (RexxMethodContext * context, __adcl(t1, n1), __adcl(t2, n2), __adcl(t3, n3), __adcl(t4, n4), __adcl(t5, n5), __adcl(t6, n6), __adcl(t7, n7), __adcl(t8, n8), __adcl(t9, n9), __adcl(t10, n10));  \
                               \
/* method signature definition */ \
static uint16_t name##_types[] = {__tdcl(returnType), __tdcl(t1), __tdcl(t2), __tdcl(t3), __tdcl(t4), __tdcl(t5), __tdcl(t6), __tdcl(t7), __tdcl(t8), __tdcl(t9), __tdcl(t10), REXX_ARGUMENT_TERMINATOR};    \
\
__cpp_method_proto(name) \
/* generated calling stub function */ \
__methodstub(name) \
{ \
    if (arguments != NULL) /* if no arguments passed, this a signature request */ \
    {                                                                             \
        /* forward to the method implementation */                                \
        __ret(returnType, name##_impl(context, __arg(1, t1), __arg(2, t2), __arg(3, t3), __arg(4, t4), __arg(5, t5), __arg(6, t6), __arg(7, t7), __arg(8, t8), __arg(9, t9), __arg(10, t10)));      \
    }                                                                             \
    return name##_types;     /* return the type signature */                      \
} \
/* the real target method code */  \
__type(returnType) name##_impl(RexxMethodContext *context, __adcl(t1, n1), __adcl(t2, n2), __adcl(t3, n3), __adcl(t4, n4), __adcl(t5, n5), __adcl(t6, n6), __adcl(t7, n7), __adcl(t8, n8), __adcl(t9, n9), __adcl(t10, n10))


#define __functionstub(name) uint16_t * RexxEntry name(RexxCallContext *context, ValueDescriptor *arguments)

#ifdef __cplusplus
#define __cpp_function_proto(name) extern "C" __functionstub(name);
#else
#define __cpp_function_proto(name) __functionstub(name);
#endif

#define REXX_TYPED_ROUTINE_PROTOTYPE(name) __cpp_function_proto(name)

// zero argument function call

#define RexxRoutine0(returnType, name) \
/* forward reference definition for method */ \
__type(returnType) name##_impl (RexxCallContext * context);  \
                               \
/* method signature definition */ \
static uint16_t name##_types[] = {__tdcl(returnType), REXX_ARGUMENT_TERMINATOR};    \
\
__cpp_function_proto(name) \
/* generated calling stub function */ \
__functionstub(name) \
{ \
    if (arguments != NULL) /* if no arguments passed, this a signature request */ \
    {                                                                             \
        /* forward to the method implementation */                                \
        __ret(returnType, name##_impl(context));                                 \
    }                                                                             \
    return name##_types;     /* return the type signature */                      \
} \
/* the real target method code */  \
__type(returnType) name##_impl(RexxCallContext *context)


// method with one argument
#define RexxRoutine1(returnType ,name, t1, n1) \
/* forward reference definition for method */ \
__type(returnType) name##_impl (RexxCallContext * context, __adcl(t1, n1));  \
                               \
/* method signature definition */ \
static uint16_t name##_types[] = {__tdcl(returnType), __tdcl(t1), REXX_ARGUMENT_TERMINATOR};    \
\
__cpp_function_proto(name) \
/* generated calling stub function */ \
__functionstub(name) \
{ \
    if (arguments != NULL) /* if no arguments passed, this a signature request */ \
    {                                                                             \
        /* forward to the method implementation */                                \
        __ret(returnType, name##_impl(context, __arg(1, t1)));                    \
    }                                                                             \
    return name##_types;     /* return the type signature */                      \
} \
/* the real target method code */  \
__type(returnType) name##_impl(RexxCallContext *context, __adcl(t1, n1))


// method with two arguments
#define RexxRoutine2(returnType ,name, t1, n1, t2, n2) \
/* forward reference definition for method */ \
__type(returnType) name##_impl (RexxCallContext * context, __adcl(t1, n1), __adcl(t2, n2));  \
                               \
/* method signature definition */ \
static uint16_t name##_types[] = {__tdcl(returnType), __tdcl(t1), __tdcl(t2), REXX_ARGUMENT_TERMINATOR };    \
\
__cpp_function_proto(name) \
/* generated calling stub function */ \
__functionstub(name) \
{ \
    if (arguments != NULL) /* if no arguments passed, this a signature request */ \
    {                                                                             \
        /* forward to the method implementation */                                \
        __ret(returnType, name##_impl(context, __arg(1, t1), __arg(2, t2)));      \
    }                                                                             \
    return name##_types;     /* return the type signature */                      \
} \
/* the real target method code */  \
__type(returnType) name##_impl(RexxCallContext *context, __adcl(t1, n1), __adcl(t2, n2))


// method with three arguments
#define RexxRoutine3(returnType ,name, t1, n1, t2, n2, t3, n3) \
/* forward reference definition for method */ \
__type(returnType) name##_impl (RexxCallContext * context, __adcl(t1, n1), __adcl(t2, n2), __adcl(t3, n3));  \
                               \
/* method signature definition */ \
static uint16_t name##_types[] = {__tdcl(returnType), __tdcl(t1), __tdcl(t2), __tdcl(t3), REXX_ARGUMENT_TERMINATOR};    \
\
__cpp_function_proto(name) \
/* generated calling stub function */ \
__functionstub(name) \
{ \
    if (arguments != NULL) /* if no arguments passed, this a signature request */ \
    {                                                                             \
        /* forward to the method implementation */                                \
        __ret(returnType, name##_impl(context, __arg(1, t1), __arg(2, t2), __arg(3, t3)));      \
    }                                                                             \
    return name##_types;     /* return the type signature */                      \
} \
/* the real target method code */  \
__type(returnType) name##_impl(RexxCallContext *context, __adcl(t1, n1), __adcl(t2, n2), __adcl(t3, n3))


// method with four arguments
#define RexxRoutine4(returnType ,name, t1, n1, t2, n2, t3, n3, t4, n4) \
/* forward reference definition for method */ \
__type(returnType) name##_impl (RexxCallContext * context, __adcl(t1, n1), __adcl(t2, n2), __adcl(t3, n3), __adcl(t4, n4));  \
                               \
/* method signature definition */ \
static uint16_t name##_types[] = {__tdcl(returnType), __tdcl(t1), __tdcl(t2), __tdcl(t3), __tdcl(t4), REXX_ARGUMENT_TERMINATOR};    \
\
__cpp_function_proto(name) \
/* generated calling stub function */ \
__functionstub(name) \
{ \
    if (arguments != NULL) /* if no arguments passed, this a signature request */ \
    {                                                                             \
        /* forward to the method implementation */                                \
        __ret(returnType, name##_impl(context, __arg(1, t1), __arg(2, t2), __arg(3, t3), __arg(4, t4)));      \
    }                                                                             \
    return name##_types;     /* return the type signature */                      \
} \
/* the real target method code */  \
__type(returnType) name##_impl(RexxCallContext *context, __adcl(t1, n1), __adcl(t2, n2), __adcl(t3, n3), __adcl(t4, n4))


// method with five arguments
#define RexxRoutine5(returnType ,name, t1, n1, t2, n2, t3, n3, t4, n4, t5, n5) \
/* forward reference definition for method */ \
__type(returnType) name##_impl (RexxCallContext * context, __adcl(t1, n1), __adcl(t2, n2), __adcl(t3, n3), __adcl(t4, n4), __adcl(t5, n5));  \
                               \
/* method signature definition */ \
static uint16_t name##_types[] = {__tdcl(returnType), __tdcl(t1), __tdcl(t2), __tdcl(t3), __tdcl(t4), __tdcl(t5), REXX_ARGUMENT_TERMINATOR};    \
\
__cpp_function_proto(name) \
/* generated calling stub function */ \
__functionstub(name) \
{ \
    if (arguments != NULL) /* if no arguments passed, this a signature request */ \
    {                                                                             \
        /* forward to the method implementation */                                \
        __ret(returnType, name##_impl(context, __arg(1, t1), __arg(2, t2), __arg(3, t3), __arg(4, t4), __arg(5, t5)));      \
    }                                                                             \
    return name##_types;     /* return the type signature */                      \
} \
/* the real target method code */  \
__type(returnType) name##_impl(RexxCallContext *context, __adcl(t1, n1), __adcl(t2, n2), __adcl(t3, n3), __adcl(t4, n4), __adcl(t5, n5))


// method with six arguments
#define RexxRoutine6(returnType, name, t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6) \
/* forward reference definition for method */ \
__type(returnType) name##_impl (RexxCallContext * context, __adcl(t1, n1), __adcl(t2, n2), __adcl(t3, n3), __adcl(t4, n4), __adcl(t5, n5), __adcl(t6, n6));  \
                               \
/* method signature definition */ \
static uint16_t name##_types[] = {__tdcl(returnType), __tdcl(t1), __tdcl(t2), __tdcl(t3), __tdcl(t4), __tdcl(t5), __tdcl(t6), REXX_ARGUMENT_TERMINATOR};    \
\
__cpp_function_proto(name) \
/* generated calling stub function */ \
__functionstub(name) \
{ \
    if (arguments != NULL) /* if no arguments passed, this a signature request */ \
    {                                                                             \
        /* forward to the method implementation */                                \
        __ret(returnType, name##_impl(context, __arg(1, t1), __arg(2, t2), __arg(3, t3), __arg(4, t4), __arg(5, t5), __arg(6, t6)));      \
    }                                                                             \
    return name##_types;     /* return the type signature */                      \
} \
/* the real target method code */  \
__type(returnType) name##_impl(RexxCallContext *context, __adcl(t1, n1), __adcl(t2, n2), __adcl(t3, n3), __adcl(t4, n4), __adcl(t5, n5), __adcl(t6, n6))

// method with seven arguments
#define RexxRoutine7(returnType, name, t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7) \
/* forward reference definition for method */ \
__type(returnType) name##_impl (RexxCallContext * context, __adcl(t1, n1), __adcl(t2, n2), __adcl(t3, n3), __adcl(t4, n4), __adcl(t5, n5), __adcl(t6, n6), __adcl(t7, n7)); \
                               \
/* method signature definition */ \
static uint16_t name##_types[] = {__tdcl(returnType), __tdcl(t1), __tdcl(t2), __tdcl(t3), __tdcl(t4), __tdcl(t5), __tdcl(t6), __tdcl(t7), REXX_ARGUMENT_TERMINATOR};    \
\
__cpp_function_proto(name) \
/* generated calling stub function */ \
__functionstub(name) \
{ \
    if (arguments != NULL) /* if no arguments passed, this a signature request */ \
    {                                                                             \
        /* forward to the method implementation */                                \
        __ret(returnType, name##_impl(context, __arg(1, t1), __arg(2, t2), __arg(3, t3), __arg(4, t4), __arg(5, t5), __arg(6, t6), __arg(7, t7)));      \
    }                                                                             \
    return name##_types;     /* return the type signature */                      \
} \
/* the real target method code */  \
__type(returnType) name##_impl(RexxCallContext *context, __adcl(t1, n1), __adcl(t2, n2), __adcl(t3, n3), __adcl(t4, n4), __adcl(t5, n5), __adcl(t6, n6), __adcl(t7, n7))


// function with eight arguments
#define RexxRoutine8(returnType, name, t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8) \
/* forward reference definition for method */ \
__type(returnType) name##_impl (RexxCallContext * context, __adcl(t1, n1), __adcl(t2, n2), __adcl(t3, n3), __adcl(t4, n4), __adcl(t5, n5), __adcl(t6, n6), __adcl(t7, n7), __adcl(t8, n8));  \
                               \
/* method signature definition */ \
static uint16_t name##_types[] = {__tdcl(returnType), __tdcl(t1), __tdcl(t2), __tdcl(t3), __tdcl(t4), __tdcl(t5), __tdcl(t6), __tdcl(t7), __tdcl(t8), REXX_ARGUMENT_TERMINATOR};    \
\
__cpp_function_proto(name) \
/* generated calling stub function */ \
__functionstub(name) \
{ \
    if (arguments != NULL) /* if no arguments passed, this a signature request */ \
    {                                                                             \
        /* forward to the method implementation */                                \
        __ret(returnType, name##_impl(context, __arg(1, t1), __arg(2, t2), __arg(3, t3), __arg(4, t4), __arg(5, t5), __arg(6, t6), __arg(7, t7), __arg(8, t8)));      \
    }                                                                             \
    return name##_types;     /* return the type signature */                      \
} \
/* the real target method code */  \
__type(returnType) name##_impl(RexxCallContext *context, __adcl(t1, n1), __adcl(t2, n2), __adcl(t3, n3), __adcl(t4, n4), __adcl(t5, n5), __adcl(t6, n6), __adcl(t7, n7), __adcl(t8, n8))


// function with nine arguments
#define RexxRoutine9(returnType, name, t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8, t9, n9) \
/* forward reference definition for method */ \
__type(returnType) name##_impl (RexxCallContext * context, __adcl(t1, n1), __adcl(t2, n2), __adcl(t3, n3), __adcl(t4, n4), __adcl(t5, n5), __adcl(t6, n6), __adcl(t7, n7), __adcl(t8, n8), __adcl(t9, n9));  \
                               \
/* method signature definition */ \
static uint16_t name##_types[] = {__tdcl(returnType), __tdcl(t1), __tdcl(t2), __tdcl(t3), __tdcl(t4), __tdcl(t5), __tdcl(t6), __tdcl(t7), __tdcl(t8), __tdcl(t9), REXX_ARGUMENT_TERMINATOR};    \
\
__cpp_function_proto(name) \
/* generated calling stub function */ \
__functionstub(name) \
{ \
    if (arguments != NULL) /* if no arguments passed, this a signature request */ \
    {                                                                             \
        /* forward to the method implementation */                                \
        __ret(returnType, name##_impl(context, __arg(1, t1), __arg(2, t2), __arg(3, t3), __arg(4, t4), __arg(5, t5), __arg(6, t6), __arg(7, t7), __arg(8, t8), __arg(9, t9)));      \
    }                                                                             \
    return name##_types;     /* return the type signature */                      \
} \
/* the real target method code */  \
__type(returnType) name##_impl(RexxCallContext *context, __adcl(t1, n1), __adcl(t2, n2), __adcl(t3, n3), __adcl(t4, n4), __adcl(t5, n5), __adcl(t6, n6), __adcl(t7, n7), __adcl(t8, n8), __adcl(t9, n9))


// function with ten arguments
#define RexxRoutine10(returnType, name, t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8, t9, n9, t10, n10) \
/* forward reference definition for method */ \
__type(returnType) name##_impl (RexxCallContext * context, __adcl(t1, n1), __adcl(t2, n2), __adcl(t3, n3), __adcl(t4, n4), __adcl(t5, n5), __adcl(t6, n6), __adcl(t7, n7), __adcl(t8, n8), __adcl(t9, n9), __adcl(t10, n10));  \
                               \
/* method signature definition */ \
static uint16_t name##_types[] = {__tdcl(returnType), __tdcl(t1), __tdcl(t2), __tdcl(t3), __tdcl(t4), __tdcl(t5), __tdcl(t6), __tdcl(t7), __tdcl(t8), __tdcl(t9), __tdcl(t10), REXX_ARGUMENT_TERMINATOR};    \
\
__cpp_function_proto(name) \
/* generated calling stub function */ \
__functionstub(name) \
{ \
    if (arguments != NULL) /* if no arguments passed, this a signature request */ \
    {                                                                             \
        /* forward to the method implementation */                                \
        __ret(returnType, name##_impl(context, __arg(1, t1), __arg(2, t2), __arg(3, t3), __arg(4, t4), __arg(5, t5), __arg(6, t6), __arg(7, t7), __arg(8, t8), __arg(9, t9), __arg(10, t10)));      \
    }                                                                             \
    return name##_types;     /* return the type signature */                      \
} \
/* the real target method code */  \
__type(returnType) name##_impl(RexxCallContext *context, __adcl(t1, n1), __adcl(t2, n2), __adcl(t3, n3), __adcl(t4, n4), __adcl(t5, n5), __adcl(t6, n6), __adcl(t7, n7), __adcl(t8, n8), __adcl(t9, n9), __adcl(t10, n10))

/******************************************************************************/
/* Types (used in macro expansions and function prototypes)                   */
/******************************************************************************/
typedef RexxObjectPtr OSELF;
typedef void *        CSELF;
typedef void *        BUFFER;
typedef RexxArrayObject ARGLIST;
typedef RexxObjectPtr   SCOPE;
typedef RexxClassObject SUPER;
typedef CSTRING         NAME;

#endif
