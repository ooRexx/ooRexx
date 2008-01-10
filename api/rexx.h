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

#include "rexxapitypes.h"              // Platform specific stuff

/*----------------------------------------------------------------------------*/
/*                                                                            */
/*                               Common                                       */
/*                                                                            */
/*----------------------------------------------------------------------------*/

typedef unsigned int APIRET;           // API return type

/******************************************************************************/
/* Types (for general use)                                                    */
/******************************************************************************/
typedef const char *CSTRING;          /* pointer to zero-terminated string */
typedef void *POINTER;
typedef void *REXXOBJECT;             /* reference to a REXX object        */
typedef REXXOBJECT STRING;            /* REXX string object                */
typedef REXXOBJECT REXXSTRING;        /* REXX string object                */

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

/***    Structure for system exit block (RXSYSEXIT) 32-bit */

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

typedef size_t stringsize_t;           // a Rexx string size
typedef ssize_t wholenumber_t;         // a Rexx whole number

/*----------------------------------------------------------------------------*/
/*                                                                            */
/*                               32-bit                                       */
/*                                                                            */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/***    Main Entry Point to the Rexx Interpreter                              */
/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

int APIENTRY RexxStart (
         size_t,                       /* Num of args passed to rexx */
         PCONSTRXSTRING,               /* Array of args passed to rex */
         const char *,                 /* [d:][path] filename[.ext]  */
         PRXSTRING,                    /* Loc of rexx proc in memory */
         const char *,                 /* ASCIIZ initial environment.*/
         int,                          /* type (command,subrtn,funct) */
         PRXSYSEXIT,                   /* SysExit env. names &  codes */
         short *,                      /* Ret code from if numeric   */
         PRXSTRING );                  /* Retvalue from the rexx proc */
typedef APIRET (APIENTRY *PFNREXXSTART)(size_t, PCONSTRXSTRING, const char *, PRXSTRING,
                                        const char *, int, PRXSYSEXIT, short *,
                                        PRXSTRING);
#define REXXSTART RexxStart


/*----------------------------------------------------------------------------*/
/***    Subcommand Interface                                                  */
/*----------------------------------------------------------------------------*/

/* This typedef simplifies coding of a Subcommand handler.           */
typedef APIRET APIENTRY RexxSubcomHandler(PCONSTRXSTRING,
                                unsigned short *,
                                PRXSTRING);

/***   RexxRegisterSubcomDll -- Register a DLL entry point           */
/***   as a Subcommand handler */

APIRET APIENTRY RexxRegisterSubcomDll (
         const char *,                         /* Name of subcom handler     */
         const char *,                         /* Name of DLL                */
         const char *,                         /* Name of procedure in DLL   */
         const char *,                         /* User area                  */
         size_t );                             /* Drop authority.            */
typedef APIRET (APIENTRY *PFNREXXREGISTERSUBCOMDLL)(const char *, const char *, const char *,
                                                    char *, size_t);
#define REXXREGISTERSUBCOMDLL  RexxRegisterSubcomDll


/***   RexxRegisterSubcomExe -- Register an EXE entry point          */
/***   as a Subcommand handler */

APIRET APIENTRY RexxRegisterSubcomExe (
         const char *,                 /* Name of subcom handler     */
         REXXPFN,                      /* address of handler in EXE  */
         const char *);                /* User area                  */
typedef APIRET (APIENTRY *PFNREXXREGISTERSUBCOMEXE)(const char *, REXXPFN, char *);
#define REXXREGISTERSUBCOMEXE  RexxRegisterSubcomExe


/***    RexxQuerySubcom - Query an environment for Existance */

APIRET APIENTRY RexxQuerySubcom(
         const char *,                 /* Name of the Environment    */
         const char *,                 /* DLL Module Name            */
         unsigned short *,             /* Stor for existance code    */
         char *);                      /* Stor for user word         */
typedef APIRET (APIENTRY *PFNREXXQUERYSUBCOM)(const char *, const char *, unsigned short *,
                                              char *);
#define REXXQUERYSUBCOM  RexxQuerySubcom


/***    RexxDeregisterSubcom - Drop registration of a Subcommand     */
/***    environment */

APIRET APIENTRY RexxDeregisterSubcom(
         const char *,                         /* Name of the Environment    */
         const char * );                       /* DLL Module Name            */
typedef APIRET (APIENTRY *PFNREXXDEREGISTERSUBCOM)(const char *, const char *);
#define REXXDEREGISTERSUBCOM  RexxDeregisterSubcom


/*----------------------------------------------------------------------------*/
/***    Shared Variable Pool Interface                                        */
/*----------------------------------------------------------------------------*/

/***    RexxVariablePool - Request Variable Pool Service */

APIRET APIENTRY RexxVariablePool(
         PSHVBLOCK);                  /* Pointer to list of SHVBLOCKs*/
typedef APIRET (APIENTRY *PFNREXXVARIABLEPOOL)(PSHVBLOCK);
#define REXXVARIABLEPOOL  RexxVariablePool


/*----------------------------------------------------------------------------*/
/***    External Function Interface                                           */
/*----------------------------------------------------------------------------*/

/* This typedef simplifies coding of an External Function.           */

typedef unsigned int APIENTRY RexxFunctionHandler(const char *,
                                  size_t,
                                  PCONSTRXSTRING,
                                  const char *,
                                  PRXSTRING);

/***    RexxRegisterFunctionDll - Register a function in the AFT */

APIRET APIENTRY RexxRegisterFunctionDll (
        const char *,                          /* Name of function to add    */
        const char *,                          /* Dll file name (if in dll)  */
        const char *);                         /* Entry in dll               */
typedef APIRET (APIENTRY *PFNREXXREGISTERFUNCTIONDLL)(const char *, const char *, const char *);
#define REXXREGISTERFUNCTIONDLL  RexxRegisterFunctionDll


/***    RexxRegisterFunctionExe - Register a function in the AFT */

APIRET APIENTRY RexxRegisterFunctionExe (
        const char *,                  /* Name of function to add    */
        REXXPFN);                      /* Entry point in EXE         */
typedef APIRET (APIENTRY *PFNREXXREGISTERFUNCTIONEXE)(const char *, REXXPFN);
#define REXXREGISTERFUNCTIONEXE  RexxRegisterFunctionExe


/***    RexxDeregisterFunction - Delete a function from the AFT */

APIRET APIENTRY RexxDeregisterFunction (
        const char * );                         /* Name of function to remove */
typedef APIRET (APIENTRY *PFNREXXDEREGISTERFUNCTION)(const char *);
#define REXXDEREGISTERFUNCTION  RexxDeregisterFunction


/***    RexxQueryFunction - Scan the AFT for a function */

APIRET APIENTRY RexxQueryFunction (
        const char * );                         /* Name of function to find   */
typedef APIRET (APIENTRY *PFNREXXQUERYFUNCTION)(const char *);
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



/***    Subfunction RXEXFCAL - Scripting Function Calls */

typedef  struct _RXEXF_FLAGS {          /* fl */
   unsigned rxfferr  : 1;              /* Invalid call to routine.   */
   unsigned rxffnfnd : 1;              /* Function not found.        */
   unsigned rxffsub  : 1;              /* Called as a subroutine     */
}  RXEXF_FLAGS ;

typedef  struct _RXEXFCAL_PARM {        /* fnc */
   RXFNC_FLAGS       rxfnc_flags ;     /* function flags             */
   CONSTRXSTRING     rxfnc_name;       // the called function name
   size_t            rxfnc_argc;       /* Number of args in list.    */
   REXXOBJECT       *rxfnc_argv;       /* Pointer to argument list.  */
   REXXOBJECT        rxfnc_retc;       /* Return value.              */
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


typedef struct _RXDBG_FLAGS {          /* fl Trace flags             */
   unsigned rxftrace;                  /* Set to run external trace. */
}  RXDBG_FLAGS;


typedef struct _RXDBGTST_PARM {        /* tst */
   RXDBG_FLAGS   rxdbg_flags;          /* Set to run external trace before */
   size_t        rxdbg_line;
   CONSTRXSTRING rxdbg_filename;
   CONSTRXSTRING rxdbg_routine;
}  RXDBGTST_PARM;


typedef  struct _RXVARNOVALUE_PARM {   /* var */
   CONSTRXSTRING     variable_name;    // the request variable name
   REXXOBJECT        value;            // returned variable value
}  RXVARNOVALUE_PARM;


typedef  struct _RXVALCALL_PARM {      /* val */
   CONSTRXSTRING     selector;         // the environment selector name
   CONSTRXSTRING     variable_name;    // the request variable name
   REXXOBJECT        value;            // returned variable value
}  RXVALCALL_PARM;

/* This typedef simplifies coding of an Exit handler.                */
typedef int APIENTRY RexxExitHandler(int, int, PEXIT);

/***      RexxRegisterExitDll - Register a system exit. */

APIRET APIENTRY RexxRegisterExitDll (
         const char *,                 /* Name of the exit handler   */
         const char *,                 /* Name of the DLL            */
         const char *,                 /* Name of the procedure      */
         const char *,                 /* User area                  */
         size_t);                      /* Drop authority             */
typedef APIRET (APIENTRY *PFNREXXREGISTEREXITDLL)(const char *, const char *, const char *,
                                                  char *, size_t);
#define REXXREGISTEREXITDLL  RexxRegisterExitDll


/***      RexxRegisterExitExe - Register a system exit. */

APIRET APIENTRY RexxRegisterExitExe (
         const char *,                 /* Name of the exit handler   */
         REXXPFN,                      /* Address of exit handler    */
         const char *);                /* User area                  */
typedef APIRET (APIENTRY *PFNREXXREGISTEREXITEXE)(const char *, REXXPFN, char *);
#define REXXREGISTEREXITEXE  RexxRegisterExitExe


/***    RexxDeregisterExit - Drop registration of a system exit. */

APIRET APIENTRY RexxDeregisterExit (
         const char *,                          /* Exit name                  */
         const char * ) ;                       /* DLL module name            */
typedef APIRET (APIENTRY *PFNREXXDEREGISTEREXIT)(const char *, const char *);
#define REXXDEREGISTEREXIT  RexxDeregisterExit


/***    RexxQueryExit - Query an exit for existance. */

APIRET APIENTRY RexxQueryExit (
         const char *,                 /* Exit name                  */
         const char *,                 /* DLL Module name.           */
         unsigned short *,             /* Existance flag.            */
         char * );                     /* User data.                 */
typedef APIRET (APIENTRY *PFNREXXQUERYEXIT)(const char *, const char *, unsigned short *, char *);
#define REXXQUERYEXIT  RexxQueryExit


/*----------------------------------------------------------------------------*/
/***    Asynchronous Request Interface                                        */
/*----------------------------------------------------------------------------*/

/***    RexxSetHalt - Request Program Halt */

APIRET APIENTRY RexxSetHalt(
         process_id_t,                /* Process Id                  */
         thread_id_t);                /* Thread Id                   */
typedef APIRET (APIENTRY *PFNREXXSETHALT)(process_id_t, thread_id_t);
#define REXXSETHALT  RexxSetHalt


/***    RexxSetTrace - Request Program Trace */

APIRET APIENTRY RexxSetTrace(
         process_id_t,                /* Process Id                  */
         thread_id_t);                /* Thread Id                   */
typedef APIRET (APIENTRY *PFNREXXSETTRACE)(process_id_t, thread_id_t);
#define REXXSETTRACE  RexxSetTrace


/***    RexxResetTrace - Turn Off Program Trace */

APIRET APIENTRY RexxResetTrace(
         process_id_t,                /* Process Id                  */
         thread_id_t);                /* Thread Id                   */
typedef APIRET (APIENTRY *PFNREXXRESETTRACE)(process_id_t, thread_id_t);
#define REXXRESETTRACE  RexxResetTrace


/*----------------------------------------------------------------------------*/
/***    Macro Space Interface                                                 */
/*----------------------------------------------------------------------------*/

/***    RexxAddMacro - Register a function in the Macro Space        */

APIRET APIENTRY RexxAddMacro(
         const char *,                 /* Function to add or change   */
         const char *,                 /* Name of file to get function*/
         size_t);                      /* Flag indicating search pos  */
typedef APIRET (APIENTRY *PFNREXXADDMACRO)(const char *, const char *, size_t);
#define REXXADDMACRO  RexxAddMacro


/***    RexxDropMacro - Remove a function from the Macro Space       */

APIRET APIENTRY RexxDropMacro (
         const char * );                        /* Name of function to remove */
typedef APIRET (APIENTRY *PFNREXXDROPMACRO)(const char *);
#define REXXDROPMACRO  RexxDropMacro


/***    RexxSaveMacroSpace - Save Macro Space functions to a file    */

APIRET APIENTRY RexxSaveMacroSpace (
         size_t,                              /* Argument count (0==save all)*/
         const char * *,                      /* List of funct names to save */
         const char *);                       /* File to save functions in   */
typedef APIRET (APIENTRY * PFNREXXSAVEMACROSPACE)(size_t, const char * *, const char *);
#define REXXSAVEMACROSPACE  RexxSaveMacroSpace


/***    RexxLoadMacroSpace - Load Macro Space functions from a file  */

APIRET APIENTRY RexxLoadMacroSpace (
         size_t,                              /* Argument count (0==load all)*/
         const char * *,                      /* List of funct names to load */
         const char *);                       /* File to load functions from */
typedef APIRET (APIENTRY *PFNREXXLOADMACROSPACE)(size_t, const char * *, const char *);
#define REXXLOADMACROSPACE  RexxLoadMacroSpace


/***    RexxQueryMacro - Find a function's search-order position     */

APIRET APIENTRY RexxQueryMacro (
         const char *,                         /* Function to search for      */
         unsigned short * );                   /* Ptr for position flag return*/
typedef APIRET (APIENTRY *PFNREXXQUERYMACRO)(const char *, unsigned short *);
#define REXXQUERYMACRO  RexxQueryMacro


/***    RexxReorderMacro - Change a function's search-order          */
/***                            position                             */

APIRET APIENTRY RexxReorderMacro(
         const char *,                        /* Name of funct change order  */
         size_t);                             /* New position for function   */
typedef APIRET (APIENTRY *PFNREXXREORDERMACRO)(const char *, size_t);
#define REXXREORDERMACRO  RexxReorderMacro


/***    RexxClearMacroSpace - Remove all functions from a MacroSpace */

APIRET APIENTRY RexxClearMacroSpace(
         void );                      /* No Arguments.               */
typedef APIRET (APIENTRY *PFNREXXCLEARMACROSPACE)(void);
#define REXXCLEARMACROSPACE  RexxClearMacroSpace


/*----------------------------------------------------------------------------*/
/***    Queing Services                                                       */
/*----------------------------------------------------------------------------*/

/***    RexxCreateQueue - Create an External Data Queue */

APIRET APIENTRY RexxCreateQueue (
        char *,                                /* Name of queue created       */
        size_t,                                /* Size of buf for ret name    */
        const char *,                          /* Requested name for queue    */
        size_t *);                             /* Duplicate name flag.        */
typedef APIRET (APIENTRY *PFNREXXCREATEQUEUE)(char *, size_t, const char *, size_t);


/***    RexxDeleteQueue - Delete an External Data Queue */

APIRET APIENTRY RexxDeleteQueue (
        const char * );                         /* Name of queue to be deleted */
typedef APIRET (APIENTRY *PFNREXXDELETEQUEUE)(const char *);


/*** RexxQueryQueue - Query an External Data Queue for number of      */
/***                  entries                                         */

APIRET APIENTRY RexxQueryQueue (
        const char *,                          /* Name of queue to query      */
        size_t *);                             /* Place to put element count  */
typedef APIRET (APIENTRY *PFNREXXQUERYQUEUE)(const char *, size_t *);


/***    RexxAddQueue - Add an entry to an External Data Queue */

APIRET APIENTRY RexxAddQueue (
        const char *,                          /* Name of queue to add to     */
        PCONSTRXSTRING,                        /* Data string to add          */
        size_t);                               /* Queue type (FIFO|LIFO)      */
typedef APIRET (APIENTRY *PFNREXXADDQUEUE)(const char *, PCONSTRXSTRING, size_t);


#include "rexxplatformapis.h"


APIRET APIENTRY RexxShutDownAPI(void);

typedef APIRET (APIENTRY *PFNREXXSHUTDOWNAPI)(void);
#define REXXSHUTDOWNAPI  RexxShutDownAPI


/*----------------------------------------------------------------------------*/
/***    Memory Allocation Services                                            */
/*----------------------------------------------------------------------------*/

/***   RexxAllocateMemory            */

void *APIENTRY RexxAllocateMemory(
                   size_t);                    /* number of bytes to allocate */
typedef void *(APIENTRY *PFNREXXALLOCATEMEMORY)(size_t );


/***   RexxFreeMemory                */

APIRET APIENTRY RexxFreeMemory(
                   void *);  /* pointer to the memory returned by    */
                             /* RexxAllocateMemory                   */
typedef APIRET (APIENTRY *PFNREXXFREEMEMORY)(void *);

int APIENTRY RexxResolveExit(const char *, REXXPFN *);

APIRET APIENTRY RexxCallFunction (
        const char *,                  /* Name of function to call   */
        size_t,                        /* Number of arguments        */
        PCONSTRXSTRING,                /* Array of argument strings  */
        int            *,              /* RC from function called    */
        PRXSTRING,                     /* Storage for returned data  */
        const char *);                 /* Name of active data queue  */

/***   Uppercase Entry Point Name */
#define REXXCALLFUNCTION  RexxCallFunction

#ifdef __cplusplus
}
#endif

#endif /* REXXSAA_INCLUDED */

