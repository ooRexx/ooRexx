/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2008 Rexx Language Association. All rights reserved.    */
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

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Module Name: rexx.h                                                        */
/*                                                                            */
/* ooRexx Common Definitions File                                             */
/*                                                                            */
/* Note: This is a revision of the original IBM rexx.h header file. All of the*/
/* conditional sections have been removed and it has been split into multiple */
/* header files, some of which are platform specific. Many of the types have  */
/* been changed to more portable types.                                       */
/*                                                                            */
/*----------------------------------------------------------------------------*/

#ifndef REXXSAA_INCLUDED
#define REXXSAA_INCLUDED


#ifdef __cplusplus
#define BEGIN_EXTERN_C() extern "C" {
#define END_EXTERN_C() }
#else
#define BEGIN_EXTERN_C()
#define END_EXTERN_C()
#endif


#include "rexxapitypes.h"              // Platform specific stuff

/*----------------------------------------------------------------------------*/
/*                                                                            */
/*                               Common                                       */
/*                                                                            */
/*----------------------------------------------------------------------------*/

typedef int RexxReturnCode;           // API return type

/******************************************************************************/
/* Types (for general use)                                                    */
/******************************************************************************/
typedef const char *CSTRING;          /* pointer to zero-terminated string */
typedef void *POINTER;

#ifdef __cplusplus

class _RexxObjectPtr {};
class _RexxStringObject : public _RexxObjectPtr {};
class _RexxBufferStringObject : public _RexxStringObject {};
class _RexxArrayObject : public _RexxObjectPtr {};
class _RexxBufferObject : public _RexxObjectPtr {};
class _RexxPointerObject : public _RexxObjectPtr {};
class _RexxMethodObject : public _RexxObjectPtr {};
class _RexxRoutineObject : public _RexxObjectPtr {};
class _RexxPackageObject : public _RexxObjectPtr {};
class _RexxClassObject : public _RexxObjectPtr {};
class _RexxDirectoryObject : public _RexxObjectPtr {};
class _RexxSupplierObject : public _RexxObjectPtr {};
class _RexxStemObject : public _RexxObjectPtr {};

typedef _RexxObjectPtr *RexxObjectPtr;
typedef _RexxStringObject *RexxStringObject;
typedef _RexxBufferStringObject *RexxBufferStringObject;
typedef _RexxArrayObject *RexxArrayObject;
typedef _RexxBufferObject *RexxBufferObject;
typedef _RexxPointerObject *RexxPointerObject;
typedef _RexxMethodObject *RexxMethodObject;
typedef _RexxRoutineObject *RexxRoutineObject;
typedef _RexxPackageObject *RexxPackageObject;
typedef _RexxClassObject *RexxClassObject;
typedef _RexxDirectoryObject *RexxDirectoryObject;
typedef _RexxSupplierObject *RexxSupplierObject;
typedef _RexxStemObject *RexxStemObject;
#else
struct _RexxObjectPtr;
struct _RexxStringObject;
struct _RexxArrayObject;
struct _RexxBufferObject;
struct _RexxPointerObject;
struct _RexxMethodObject;
struct _RexxRoutineObject;
struct _RexxPackageObject;
struct _RexxClassObject;
struct _RexxDirectoryObject;
struct _RexxSupplierObject;
struct _RexxStemObject;

typedef struct _RexxObjectPtr *RexxObjectPtr;
typedef struct _RexxStringObject *RexxStringObject;
typedef struct _RexxBufferStringObject *RexxBufferStringObject;
typedef struct _RexxArrayObject *RexxArrayObject;
typedef struct _RexxBufferObject *RexxBufferObject;
typedef struct _RexxPointerObject *RexxPointerObject;
typedef struct _RexxMethodObject *RexxMethodObject;
typedef struct _RexxRoutineObject *RexxRoutineObject;
typedef struct _RexxPackageObject *RexxPackageObject;
typedef struct _RexxClassObject *RexxClassObject;
typedef struct _RexxDirectoryObject *RexxDirectoryObject;
typedef struct _RexxSupplierObject *RexxSupplierObject;
typedef struct _RexxStemObject *RexxStemObject;
#endif

/******************************************************************************/
/* Constant values (for general use)                                          */
/******************************************************************************/
#define NO_CSTRING            NULL
#define NULLOBJECT            NULL


/*----------------------------------------------------------------------------*/
/***    RXSTRING defines                                                      */
/*----------------------------------------------------------------------------*/

/***    Structure for external interface string (RXSTRING) */

typedef struct _RXSTRING {             /* rxstr                      */
        size_t  strlength;             /*   length of string         */
        char   *strptr;                /*   pointer to string        */
} RXSTRING;

typedef struct _CONSTRXSTRING {        /* const rxstr                */
    size_t  strlength;                 /*   length of string         */
    const char   *strptr;              /*   pointer to string        */
} CONSTRXSTRING;

/***    Macros for RexxString manipulation                   */

#define RXNULLSTRING(r)      ((r).strptr == NULL)
#define RXZEROLENSTRING(r)   ((r).strptr != NULL && (r).strlength == 0)
#define RXVALIDSTRING(r)     ((r).strptr != NULL && (r).strlength != 0)
#define RXSTRLEN(r)          (RXNULLSTRING(r) ? 0 : (r).strlength)
#define RXSTRPTR(r)          ((r).strptr)
#define MAKERXSTRING(r,p,l)  { (r).strptr = p; (r).strlength = l; }


typedef RXSTRING      *PRXSTRING;      /* pointer to a RXSTRING      */
typedef CONSTRXSTRING *PCONSTRXSTRING; /* pointer to a RXSTRING      */

/***    Structure for system exit block (RXSYSEXIT) */

typedef struct _RXSYSEXIT {            /* syse */
   const char *sysexit_name;           /* subcom enviro for sysexit  */
   int   sysexit_code;                 /* sysexit function code      */
}  RXSYSEXIT;
typedef RXSYSEXIT *PRXSYSEXIT;         /* pointer to a RXSYSEXIT     */



/*----------------------------------------------------------------------------*/
/***    Shared Variable Pool Interface defines                                */
/*----------------------------------------------------------------------------*/

/***    Structure of Shared Variable Request Block (SHVBLOCK) */

typedef struct _SHVBLOCK {            /* shvb */
    struct _SHVBLOCK  *shvnext;       /* pointer to the next block   */
    CONSTRXSTRING      shvname;       /* Pointer to the name buffer  */
    RXSTRING           shvvalue;      /* Pointer to the value buffer */
    size_t             shvnamelen;    /* Length of the name value    */
    size_t             shvvaluelen;   /* Length of the fetch value   */
    unsigned char      shvcode;       /* Function code for this block*/
    unsigned char      shvret;        /* Individual Return Code Flags*/
}   SHVBLOCK;
typedef SHVBLOCK *PSHVBLOCK;

typedef char *PEXIT;                  /* ptr to exit parameter block */




/*----------------------------------------------------------------------------*/
/***    Include the other common and platform specific stuff                  */
/*----------------------------------------------------------------------------*/

/* These must be placed after RXSTRING and CONSTRXSTRING are defined */
#include "rexxapidefs.h"
#include "rexxplatformdefs.h"          // Platform specific stuff

typedef size_t  stringsize_t;          // a Rexx string size
typedef ssize_t wholenumber_t;         // a Rexx whole number
typedef size_t  logical_t;             // a Rexx logical (1 or 0) value

// a synonym for readability
#define RexxEntry REXXENTRY


/***    RexxPullFromQueue - Retrieve data from an External Data Queue */
typedef struct _REXXDATETIME {         /* REXX time stamp format            */
  uint16_t       hours;                /* hour of the day (24-hour)         */
  uint16_t       minutes;              /* minute of the hour                */
  uint16_t       seconds;              /* second of the minute              */
  uint16_t       hundredths;           /* hundredths of a second            */
  uint16_t       day;                  /* day of the month                  */
  uint16_t       month;                /* month of the year                 */
  uint16_t       year;                 /* current year                      */
  uint16_t       weekday;              /* day of the week                   */
  uint32_t       microseconds;         /* microseconds                      */
  uint32_t       yearday;              /* day number within the year        */
} REXXDATETIME;


typedef struct _RexxConditionData
{
  wholenumber_t code;                 // The condition CODE information
  wholenumber_t rc;                   // The condition RC value
  RXSTRING message;                   // The condition secondary message text
  RXSTRING errortext;                 // The condition error text.
  size_t  position;                   // The failure line number value
  RXSTRING program;                   // The running program name
} RexxConditionData;

/*----------------------------------------------------------------------------*/
/*                                                                            */
/*                               32-bit                                       */
/*                                                                            */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/***    Main Entry Point to the Rexx Interpreter                              */
/*----------------------------------------------------------------------------*/

BEGIN_EXTERN_C()

int REXXENTRY RexxStart (
         size_t,                       /* Num of args passed to rexx */
         PCONSTRXSTRING,               /* Array of args passed to rex */
         const char *,                 /* [d:][path] filename[.ext]  */
         PRXSTRING,                    /* Loc of rexx proc in memory */
         const char *,                 /* ASCIIZ initial environment.*/
         int,                          /* type (command,subrtn,funct) */
         PRXSYSEXIT,                   /* SysExit env. names &  codes */
         short *,                      /* Ret code from if numeric   */
         PRXSTRING );                  /* Retvalue from the rexx proc */
typedef RexxReturnCode (REXXENTRY *PFNREXXSTART)(size_t, PCONSTRXSTRING, const char *, PRXSTRING,
                                        const char *, int, PRXSYSEXIT, short *,
                                        PRXSTRING);
#define REXXSTART RexxStart

// the following APIs are deprecated, and are included only for binary compatibility.
// These are nops if called.
void REXXENTRY RexxWaitForTermination(void);
typedef void (REXXENTRY *PFNREXXWAITFORTERMINATION)(void);
#define REXXWAITFORTERMINATION RexxWaitForTermination

RexxReturnCode REXXENTRY RexxDidRexxTerminate(void);
typedef RexxReturnCode (REXXENTRY *PFNREXXDIDREXXTERMINATE)(void);
#define REXXDIDREXXTERMINATE RexxDidRexxTerminate


RexxReturnCode REXXENTRY RexxTranslateProgram(
    const char *,                       // input program name
    const char *,                       // output file name
    PRXSYSEXIT);                        // system exits to use during translation


typedef RexxReturnCode (REXXENTRY *PFNREXXTRANSLATEPROGRAM)(const char *, const char *, PRXSYSEXIT);

#define REXXTRANSLATEPROGRAM RexxTranslateProgram


RexxReturnCode REXXENTRY RexxTranslateInstoreProgram(
    const char *,                       // input program name
    CONSTRXSTRING *,                    // program source
    RXSTRING *);                        // returned image


typedef RexxReturnCode (REXXENTRY *PFNREXXTRANSLATEINSTOREPROGRAM)(const char *, CONSTRXSTRING *, RXSTRING *);

#define REXXTRANSLATEINSTOREPROGRAM RexxTranslateInstoreProgram


char *REXXENTRY RexxGetVersionInformation(void);

typedef char *(REXXENTRY *PFNGETVERSIONINFORMATION)(void);

#define REXXGETVERSIONINFORMATON RexxGetVersionInformation


/*----------------------------------------------------------------------------*/
/***    Subcommand Interface                                                  */
/*----------------------------------------------------------------------------*/

/* This typedef simplifies coding of a Subcommand handler.           */
typedef RexxReturnCode REXXENTRY RexxSubcomHandler(PCONSTRXSTRING,
                                unsigned short *,
                                PRXSTRING);

/***   RexxRegisterSubcomDll -- Register a DLL entry point           */
/***   as a Subcommand handler */

RexxReturnCode REXXENTRY RexxRegisterSubcomDll (
         const char *,                         /* Name of subcom handler     */
         const char *,                         /* Name of DLL                */
         const char *,                         /* Name of procedure in DLL   */
         const char *,                         /* User area                  */
         size_t );                             /* Drop authority.            */
typedef RexxReturnCode (REXXENTRY *PFNREXXREGISTERSUBCOMDLL)(const char *, const char *, const char *,
                                                    char *, size_t);
#define REXXREGISTERSUBCOMDLL  RexxRegisterSubcomDll


/***   RexxRegisterSubcomExe -- Register an EXE entry point          */
/***   as a Subcommand handler */

RexxReturnCode REXXENTRY RexxRegisterSubcomExe (
         const char *,                 /* Name of subcom handler     */
         REXXPFN,                      /* address of handler in EXE  */
         const char *);                /* User area                  */
typedef RexxReturnCode (REXXENTRY *PFNREXXREGISTERSUBCOMEXE)(const char *, REXXPFN, char *);
#define REXXREGISTERSUBCOMEXE  RexxRegisterSubcomExe


/***    RexxQuerySubcom - Query an environment for Existance */

RexxReturnCode REXXENTRY RexxQuerySubcom(
         const char *,                 /* Name of the Environment    */
         const char *,                 /* DLL Module Name            */
         unsigned short *,             /* Stor for existance code    */
         char *);                      /* Stor for user word         */
typedef RexxReturnCode (REXXENTRY *PFNREXXQUERYSUBCOM)(const char *, const char *, unsigned short *,
                                              char *);
#define REXXQUERYSUBCOM  RexxQuerySubcom


/***    RexxDeregisterSubcom - Drop registration of a Subcommand     */
/***    environment */

RexxReturnCode REXXENTRY RexxDeregisterSubcom(
         const char *,                         /* Name of the Environment    */
         const char * );                       /* DLL Module Name            */
typedef RexxReturnCode (REXXENTRY *PFNREXXDEREGISTERSUBCOM)(const char *, const char *);
#define REXXDEREGISTERSUBCOM  RexxDeregisterSubcom


/*----------------------------------------------------------------------------*/
/***    Shared Variable Pool Interface                                        */
/*----------------------------------------------------------------------------*/

/***    RexxVariablePool - Request Variable Pool Service */

RexxReturnCode REXXENTRY RexxVariablePool(
         PSHVBLOCK);                  /* Pointer to list of SHVBLOCKs*/
typedef RexxReturnCode (REXXENTRY *PFNREXXVARIABLEPOOL)(PSHVBLOCK);
#define REXXVARIABLEPOOL  RexxVariablePool


/*----------------------------------------------------------------------------*/
/***    External Function Interface                                           */
/*----------------------------------------------------------------------------*/

/* This typedef simplifies coding of an External Function.           */

typedef size_t REXXENTRY RexxRoutineHandler(const char *,
                                  size_t,
                                  PCONSTRXSTRING,
                                  const char *,
                                  PRXSTRING);

/***    RexxRegisterFunctionDll - Register a function in the AFT */

RexxReturnCode REXXENTRY RexxRegisterFunctionDll (
        const char *,                          /* Name of function to add    */
        const char *,                          /* Dll file name (if in dll)  */
        const char *);                         /* Entry in dll               */
typedef RexxReturnCode (REXXENTRY *PFNREXXREGISTERFUNCTIONDLL)(const char *, const char *, const char *);
#define REXXREGISTERFUNCTIONDLL  RexxRegisterFunctionDll


/***    RexxRegisterFunctionExe - Register a function in the AFT */

RexxReturnCode REXXENTRY RexxRegisterFunctionExe (
        const char *,                  /* Name of function to add    */
        REXXPFN);                      /* Entry point in EXE         */
typedef RexxReturnCode (REXXENTRY *PFNREXXREGISTERFUNCTIONEXE)(const char *, REXXPFN);
#define REXXREGISTERFUNCTIONEXE  RexxRegisterFunctionExe


/***    RexxDeregisterFunction - Delete a function from the AFT */

RexxReturnCode REXXENTRY RexxDeregisterFunction (
        const char * );                         /* Name of function to remove */
typedef RexxReturnCode (REXXENTRY *PFNREXXDEREGISTERFUNCTION)(const char *);
#define REXXDEREGISTERFUNCTION  RexxDeregisterFunction


/***    RexxQueryFunction - Scan the AFT for a function */

RexxReturnCode REXXENTRY RexxQueryFunction (
        const char * );                         /* Name of function to find   */
typedef RexxReturnCode (REXXENTRY *PFNREXXQUERYFUNCTION)(const char *);
#define REXXQUERYFUNCTION  RexxQueryFunction


/*----------------------------------------------------------------------------*/
/***   System Exits                                                           */
/*----------------------------------------------------------------------------*/

/***    Subfunction RXFNCCAL - External Function Calls */

typedef  struct _RXFNC_FLAGS {          /* fl */
   unsigned rxfferr  : 1;              /* Invalid call to routine.   */
   unsigned rxffnfnd : 1;              /* Function not found.        */
   unsigned rxffsub  : 1;              /* Called as a subroutine     */
}  RXFNC_FLAGS ;

typedef  struct _RXFNCCAL_PARM {        /* fnc */
   RXFNC_FLAGS       rxfnc_flags ;     /* function flags             */
   const char *      rxfnc_name;       /* Pointer to function name.  */
   unsigned short    rxfnc_namel;      /* Length of function name.   */
   const char *      rxfnc_que;        /* Current queue name.        */
   unsigned short    rxfnc_quel;       /* Length of queue name.      */
   unsigned short    rxfnc_argc;       /* Number of args in list.    */
   PCONSTRXSTRING    rxfnc_argv;       /* Pointer to argument list.  */
   RXSTRING          rxfnc_retc;       /* Return value.              */
}  RXFNCCAL_PARM;


/***    Subfunction RXOFNCCAL - Object valued external function call */

typedef  struct _RXOFNC_FLAGS {        /* fl */
   unsigned rxfferr  : 1;              /* Invalid call to routine.   */
   unsigned rxffnfnd : 1;              /* Function not found.        */
   unsigned rxffsub  : 1;              /* Called as a subroutine     */
}  RXOFNC_FLAGS ;

typedef  struct _RXOFNCCAL_PARM {      /* fnc */
   RXOFNC_FLAGS      rxfnc_flags ;     /* function flags             */
   CONSTRXSTRING     rxfnc_name;       // the called function name
   size_t            rxfnc_argc;       /* Number of args in list.    */
   RexxObjectPtr    *rxfnc_argv;       /* Pointer to argument list.  */
   RexxObjectPtr     rxfnc_retc;       /* Return value.              */
}  RXOFNCCAL_PARM;



/***    Subfunction RXEXFCAL - Scripting Function Calls */

typedef  struct _RXEXF_FLAGS {          /* fl */
   unsigned rxfferr  : 1;              /* Invalid call to routine.   */
   unsigned rxffnfnd : 1;              /* Function not found.        */
   unsigned rxffsub  : 1;              /* Called as a subroutine     */
}  RXEXF_FLAGS ;

typedef  struct _RXEXFCAL_PARM {        /* fnc */
   RXEXF_FLAGS       rxfnc_flags ;     /* function flags             */
   CONSTRXSTRING     rxfnc_name;       // the called function name
   size_t            rxfnc_argc;       /* Number of args in list.    */
   RexxObjectPtr    *rxfnc_argv;       /* Pointer to argument list.  */
   RexxObjectPtr     rxfnc_retc;       /* Return value.              */
}  RXEXFCAL_PARM;

/***    Subfunction RXCMDHST -- Process Host Commands     */

typedef  struct _RXCMD_FLAGS {          /* fl */
   unsigned rxfcfail : 1;              /* Command failed.            */
   unsigned rxfcerr  : 1;              /* Command ERROR occurred.    */
}  RXCMD_FLAGS;

typedef  struct _RXCMDHST_PARM {        /* rx */
   RXCMD_FLAGS       rxcmd_flags;      /* error/failure flags        */
   const char *      rxcmd_address;    /* Pointer to address name.   */
   unsigned short    rxcmd_addressl;   /* Length of address name.    */
   const char *      rxcmd_dll;        /* dll name for command.      */
   unsigned short    rxcmd_dll_len;    /* Length of dll name.        */
   CONSTRXSTRING     rxcmd_command;    /* The command string.        */
   RXSTRING          rxcmd_retc;       /* Pointer to return buffer   */
}  RXCMDHST_PARM;


/***     Subfunction RXMSQPLL -- Pull Entry from Queue */

typedef struct _RXMSQPLL_PARM {        /* pll */
   RXSTRING          rxmsq_retc;       /* Pointer to dequeued entry  */
                                       /* buffer.  User allocated.   */
} RXMSQPLL_PARM;


/***    Subfunction RXMSQPSH -- Push Entry on Queue */
typedef  struct _RXMSQ_FLAGS {          /* fl */
   unsigned rxfmlifo : 1;               /* Stack entry LIFO if set    */
}  RXMSQ_FLAGS;


typedef  struct _RXMSQPSH_PARM {       /* psh */
   RXMSQ_FLAGS       rxmsq_flags;      /* LIFO/FIFO flag             */
   CONSTRXSTRING     rxmsq_value;      /* The entry to be pushed.    */
}  RXMSQPSH_PARM;


/***    Subfunction RXMSQSIZ -- Return the Current Queue Size */

typedef struct _RXMSQSIZ_PARM {        /* siz */
   size_t            rxmsq_size;       /* Number of Lines in Queue   */
}  RXMSQSIZ_PARM;


/***    Subfunction RXMSQNAM -- Set Current Queue Name */

typedef struct _RXMSQNAM_PARM {        /* nam */
   RXSTRING     rxmsq_name;            /* RXSTRING containing        */
                                       /* queue name.                */
}  RXMSQNAM_PARM;


/***    Subfunction RXSIOSAY -- Perform SAY Clause */

typedef struct _RXSIOSAY_PARM {        /* say */
   CONSTRXSTRING     rxsio_string;     /* String to display.         */
}  RXSIOSAY_PARM;


/***    Subfunction RXSIOTRC -- Write Trace Output */

typedef struct _RXSIOTRC_PARM { /* trcparm */
   CONSTRXSTRING     rxsio_string;     /* Trace line to display.     */
}  RXSIOTRC_PARM;


/***    Subfunction RXSIOTRD -- Read Input from the Terminal */

typedef struct _RXSIOTRD_PARM {        /* trd */
   RXSTRING          rxsiotrd_retc;    /* RXSTRING for output.       */
}  RXSIOTRD_PARM;


/***    Subfunction RXSIODTR -- Read Debug Input from the Terminal */

typedef struct _RXSIODTR_PARM {        /* dtr */
   RXSTRING          rxsiodtr_retc;    /* RXSTRING for output.       */
}  RXSIODTR_PARM;


/***    Subfunction RXHSTTST -- Test for HALT Condition */

typedef struct _RXHLT_FLAGS {          /* fl Halt flag               */
   unsigned rxfhhalt : 1;              /* Set if HALT occurred.      */
}  RXHLT_FLAGS;

typedef struct _RXHLTTST_PARM {        /* tst */
   RXHLT_FLAGS rxhlt_flags;            /* Set if HALT occurred       */
}  RXHLTTST_PARM;


/***    Subfunction RXTRCTST -- Test for TRACE Condition */

typedef struct _RXTRC_FLAGS {          /* fl Trace flags             */
   unsigned rxftrace : 1;              /* Set to run external trace. */
}  RXTRC_FLAGS;


typedef struct _RXTRCTST_PARM {        /* tst */
   RXTRC_FLAGS rxtrc_flags;            /* Set to run external trace  */
}  RXTRCTST_PARM;


typedef  struct _RXVARNOVALUE_PARM {   /* var */
   RexxStringObject  variable_name;    // the request variable name
   RexxObjectPtr     value;            // returned variable value
}  RXVARNOVALUE_PARM;


typedef  struct _RXVALCALL_PARM {      /* val */
   RexxStringObject  selector;         // the environment selector name
   RexxStringObject  variable_name;    // the request variable name
   RexxObjectPtr     value;            // returned variable value
}  RXVALCALL_PARM;

/* This typedef simplifies coding of an Exit handler.                */
typedef int REXXENTRY RexxExitHandler(int, int, PEXIT);


/***      RexxRegisterExitDll - Register a system exit. */

RexxReturnCode REXXENTRY RexxRegisterExitDll (
         const char *,                 /* Name of the exit handler   */
         const char *,                 /* Name of the DLL            */
         const char *,                 /* Name of the procedure      */
         const char *,                 /* User area                  */
         size_t);                      /* Drop authority             */
typedef RexxReturnCode (REXXENTRY *PFNREXXREGISTEREXITDLL)(const char *, const char *, const char *,
                                                  char *, size_t);
#define REXXREGISTEREXITDLL  RexxRegisterExitDll


/***      RexxRegisterExitExe - Register a system exit. */

RexxReturnCode REXXENTRY RexxRegisterExitExe (
         const char *,                 /* Name of the exit handler   */
         REXXPFN,                      /* Address of exit handler    */
         const char *);                /* User area                  */
typedef RexxReturnCode (REXXENTRY *PFNREXXREGISTEREXITEXE)(const char *, REXXPFN, char *);
#define REXXREGISTEREXITEXE  RexxRegisterExitExe


/***    RexxDeregisterExit - Drop registration of a system exit. */

RexxReturnCode REXXENTRY RexxDeregisterExit (
         const char *,                          /* Exit name                  */
         const char * ) ;                       /* DLL module name            */
typedef RexxReturnCode (REXXENTRY *PFNREXXDEREGISTEREXIT)(const char *, const char *);
#define REXXDEREGISTEREXIT  RexxDeregisterExit


/***    RexxQueryExit - Query an exit for existance. */

RexxReturnCode REXXENTRY RexxQueryExit (
         const char *,                 /* Exit name                  */
         const char *,                 /* DLL Module name.           */
         unsigned short *,             /* Existance flag.            */
         char * );                     /* User data.                 */
typedef RexxReturnCode (REXXENTRY *PFNREXXQUERYEXIT)(const char *, const char *, unsigned short *, char *);
#define REXXQUERYEXIT  RexxQueryExit


/*----------------------------------------------------------------------------*/
/***    Asynchronous Request Interface                                        */
/*----------------------------------------------------------------------------*/

/***    RexxSetHalt - Request Program Halt */

RexxReturnCode REXXENTRY RexxSetHalt(
         process_id_t,                /* Process Id                  */
         thread_id_t);                /* Thread Id                   */
typedef RexxReturnCode (REXXENTRY *PFNREXXSETHALT)(process_id_t, thread_id_t);
#define REXXSETHALT  RexxSetHalt


/***    RexxSetTrace - Request Program Trace */

RexxReturnCode REXXENTRY RexxSetTrace(
         process_id_t,                /* Process Id                  */
         thread_id_t);                /* Thread Id                   */
typedef RexxReturnCode (REXXENTRY *PFNREXXSETTRACE)(process_id_t, thread_id_t);
#define REXXSETTRACE  RexxSetTrace


/***    RexxResetTrace - Turn Off Program Trace */

RexxReturnCode REXXENTRY RexxResetTrace(
         process_id_t,                /* Process Id                  */
         thread_id_t);                /* Thread Id                   */
typedef RexxReturnCode (REXXENTRY *PFNREXXRESETTRACE)(process_id_t, thread_id_t);
#define REXXRESETTRACE  RexxResetTrace


/*----------------------------------------------------------------------------*/
/***    Macro Space Interface                                                 */
/*----------------------------------------------------------------------------*/

/***    RexxAddMacro - Register a function in the Macro Space        */

RexxReturnCode REXXENTRY RexxAddMacro(
         const char *,                 /* Function to add or change   */
         const char *,                 /* Name of file to get function*/
         size_t);                      /* Flag indicating search pos  */
typedef RexxReturnCode (REXXENTRY *PFNREXXADDMACRO)(const char *, const char *, size_t);
#define REXXADDMACRO  RexxAddMacro


/***    RexxDropMacro - Remove a function from the Macro Space       */

RexxReturnCode REXXENTRY RexxDropMacro (
         const char * );                        /* Name of function to remove */
typedef RexxReturnCode (REXXENTRY *PFNREXXDROPMACRO)(const char *);
#define REXXDROPMACRO  RexxDropMacro


/***    RexxSaveMacroSpace - Save Macro Space functions to a file    */

RexxReturnCode REXXENTRY RexxSaveMacroSpace (
         size_t,                              /* Argument count (0==save all)*/
         const char * *,                      /* List of funct names to save */
         const char *);                       /* File to save functions in   */
typedef RexxReturnCode (REXXENTRY * PFNREXXSAVEMACROSPACE)(size_t, const char * *, const char *);
#define REXXSAVEMACROSPACE  RexxSaveMacroSpace


/***    RexxLoadMacroSpace - Load Macro Space functions from a file  */

RexxReturnCode REXXENTRY RexxLoadMacroSpace (
         size_t,                              /* Argument count (0==load all)*/
         const char * *,                      /* List of funct names to load */
         const char *);                       /* File to load functions from */
typedef RexxReturnCode (REXXENTRY *PFNREXXLOADMACROSPACE)(size_t, const char * *, const char *);
#define REXXLOADMACROSPACE  RexxLoadMacroSpace


/***    RexxQueryMacro - Find a function's search-order position     */

RexxReturnCode REXXENTRY RexxQueryMacro (
         const char *,                         /* Function to search for      */
         unsigned short * );                   /* Ptr for position flag return*/
typedef RexxReturnCode (REXXENTRY *PFNREXXQUERYMACRO)(const char *, unsigned short *);
#define REXXQUERYMACRO  RexxQueryMacro


/***    RexxReorderMacro - Change a function's search-order          */
/***                            position                             */

RexxReturnCode REXXENTRY RexxReorderMacro(
         const char *,                        /* Name of funct change order  */
         size_t);                             /* New position for function   */
typedef RexxReturnCode (REXXENTRY *PFNREXXREORDERMACRO)(const char *, size_t);
#define REXXREORDERMACRO  RexxReorderMacro


/***    RexxClearMacroSpace - Remove all functions from a MacroSpace */

RexxReturnCode REXXENTRY RexxClearMacroSpace(
         void );                      /* No Arguments.               */
typedef RexxReturnCode (REXXENTRY *PFNREXXCLEARMACROSPACE)(void);
#define REXXCLEARMACROSPACE  RexxClearMacroSpace


/*----------------------------------------------------------------------------*/
/***    Queing Services                                                       */
/*----------------------------------------------------------------------------*/

#define MAX_QUEUE_NAME_LENGTH 250

/***    RexxCreateQueue - Create an External Data Queue */

RexxReturnCode REXXENTRY RexxCreateQueue (
        char *,                                /* Name of queue created       */
        size_t,                                /* Size of buf for ret name    */
        const char *,                          /* Requested name for queue    */
        size_t *);                             /* Duplicate name flag.        */
typedef RexxReturnCode (REXXENTRY *PFNREXXCREATEQUEUE)(char *, size_t, const char *, size_t);

/***    RexxOpenQueue - Create a named external queue, if necessary */

RexxReturnCode REXXENTRY RexxOpenQueue (
        const char *,                          /* Requested name for queue    */
        size_t *);                             /* Flag for already created queue */
typedef RexxReturnCode (REXXENTRY *PFNREXXOPENQUEUE)(const char *, size_t);


/***    RexxQueueExists - Check for the existance of an external data queue */

RexxReturnCode REXXENTRY RexxQueueExists (
        const char * );                         /* Name of queue to be deleted */
typedef RexxReturnCode (REXXENTRY *PFNREXXQUEUEEXISTS)(const char *);

/***    RexxDeleteQueue - Delete an External Data Queue */

RexxReturnCode REXXENTRY RexxDeleteQueue (
        const char * );                         /* Name of queue to be deleted */
typedef RexxReturnCode (REXXENTRY *PFNREXXDELETEQUEUE)(const char *);


/*** RexxQueryQueue - Query an External Data Queue for number of      */
/***                  entries                                         */

RexxReturnCode REXXENTRY RexxQueryQueue (
        const char *,                          /* Name of queue to query      */
        size_t *);                             /* Place to put element count  */
typedef RexxReturnCode (REXXENTRY *PFNREXXQUERYQUEUE)(const char *, size_t *);


/***    RexxAddQueue - Add an entry to an External Data Queue */

RexxReturnCode REXXENTRY RexxAddQueue (
        const char *,                          /* Name of queue to add to     */
        PCONSTRXSTRING,                        /* Data string to add          */
        size_t);                               /* Queue type (FIFO|LIFO)      */
typedef RexxReturnCode (REXXENTRY *PFNREXXADDQUEUE)(const char *, PCONSTRXSTRING, size_t);

/***    RexxPullFromQueue - Retrieve data from an External Data Queue */
RexxReturnCode REXXENTRY RexxPullFromQueue (
        const char *,                          /* Name of queue to read from  */
        PRXSTRING,                             /* RXSTRING to receive data    */
        REXXDATETIME *,                        /* Stor for data date/time     */
        size_t);                               /* wait status (WAIT|NOWAIT)   */
typedef RexxReturnCode (REXXENTRY *PFNREXXPULLFROMQUEUE)(const char *, PRXSTRING, REXXDATETIME *,
                                           size_t);

/***    RexxClearQueue - Clear all lines in a queue */

RexxReturnCode REXXENTRY RexxClearQueue (
        const char * );                         /* Name of queue to be deleted */
typedef RexxReturnCode (REXXENTRY *PFNREXXCLEARQUEUE)(const char *);


#include "rexxplatformapis.h"

/*----------------------------------------------------------------------------*/
/***    Memory Allocation Services                                            */
/*----------------------------------------------------------------------------*/

/***   RexxAllocateMemory            */

void *REXXENTRY RexxAllocateMemory(
                   size_t);                    /* number of bytes to allocate */
typedef void *(REXXENTRY *PFNREXXALLOCATEMEMORY)(size_t );


/***   RexxFreeMemory                */

RexxReturnCode REXXENTRY RexxFreeMemory(
                   void *);  /* pointer to the memory returned by    */
                             /* RexxAllocateMemory                   */
typedef RexxReturnCode (REXXENTRY *PFNREXXFREEMEMORY)(void *);


END_EXTERN_C()

#endif /* REXXSAA_INCLUDED */

