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
/*****************************************************************************/
/*                                                                           */
/*  Module Name: rexx.h                                                      */
/*                                                                           */
/*  REXX Common Definitions File                                             */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************
|   #define:              To include:
|
|   INCL_REXXSAA          Complete Rexx support
|   INCL_RXSUBCOM         Rexx subcommand handler support
|   INCL_RXSHV            Rexx shared variable pool support
|   INCL_RXFUNC           Rexx external function support
|   INCL_RXSYSEXIT        Rexx system exit support
|   INCL_RXARI            Rexx asynchronous Trace/Halt support
|
******************************************************************************/

#ifndef REXXSAA_INCLUDED
#define REXXSAA_INCLUDED

/*****************************************************************************/
/* Definitions of return codes and constants that are used in all REXX calls */
/*****************************************************************************/

#ifdef INCL_REXXSAA      /* INCL_REXXSAA */
#define INCL_RXSUBCOM
#define INCL_RXSHV
#define INCL_RXFUNC
#define INCL_RXSYSEXIT
#define INCL_RXARI
#endif                   /* INCL_REXXSAA */

/*
 * Some platforms do not typedef pthread_t without explicitly #including pthread.h
 */
#include <pthread.h>

/* Definitions maybe provided ---------------------------------------------- */

#if !defined(AIX) && !defined(LINUX)
#define AIX                           /* Default definition for AIX          */
#endif

#ifndef TRUE
#define TRUE            1
#endif

#ifndef FALSE
#define FALSE           0
#endif

#ifndef HQUEUE
#define HQUEUE          unsigned long
#endif

#ifndef ULONG
#define ULONG           unsigned long
#endif

#ifndef PULONG
#define PULONG          ULONG *
#endif

#ifndef PVOID
#define PVOID           void *
#endif

#ifndef PPVOID
#define PPVOID          void **
#endif

#ifndef SHORT
#define SHORT           short
#endif

#ifndef LONG
#define LONG            long
#endif

#ifndef PLONG
#define PLONG           LONG *
#endif

#ifndef USHORT
#define USHORT          unsigned short
#endif

#ifndef PSHORT
#define PSHORT          SHORT *
#endif

#ifndef PUSHORT
#define PUSHORT         USHORT *
#endif

#ifndef UCHAR
#define UCHAR           unsigned char
#endif

#ifndef PUCHAR
#define PUCHAR          UCHAR *
#endif

#ifndef CHAR
#define CHAR            char
#endif

#ifndef PCHAR
#define PCHAR           CHAR *
#endif

#ifndef INT
#define INT             int
#endif

#ifndef UINT
#define UINT            unsigned int
#endif

#ifndef PUINT
#define PUINT           unsigned int *
#endif

#ifndef PINT
#define PINT            int *
#endif

#ifndef PCH
#define PCH             PCHAR
#endif

#ifndef PSZ
#define PSZ             PCHAR
#endif

#ifndef APIENTRY
#define APIENTRY
#endif

#ifndef APIRET
#define APIRET          ULONG
#endif

#ifndef CONST
#define CONST           const
#endif

#ifndef LPCTSTR
#define LPCTSTR         LPCSTR
#endif

#ifndef BYTE
#define BYTE            unsigned char
#endif

#ifndef BOOL
#define BOOL            unsigned long
#endif

#ifndef UBYTE
#define UBYTE           unsigned char
#endif

#ifndef TID
#ifndef LINUX
#define TID             tid_t
#else
#define TID             pthread_t
#endif
#endif

#ifndef PID
#define PID             pid_t
#endif

#ifndef VOID
#define VOID            void
#endif

#ifndef near
#define near
#endif

#ifndef far
#define far
#endif

#ifndef _loadds
#define _loadds
#endif

#ifndef PFN
#define PFN             void *
#endif

/*
#ifndef FNONBLOCK
#define FNONBLOCK       O_NONBLOCK
#endif
*/

/* Structure for external interface string (RXSTRING)  -------------- */
typedef struct _RXSTRING {             /* rxstring                    */
        ULONG  strlength;              /*   length of string          */
        PCH    strptr;                 /*   pointer to string         */
}  RXSTRING;

/* DATETIME structure ----------------------------------------------- */
#ifndef _REXXDATETIME                 /* prevent muliple decls        */
#define _REXXDATETIME
typedef struct _REXXDATETIME {        /* REXX time stamp format       */
  unsigned short hours;               /* hour of the day (24-hour)    */
  unsigned short minutes;             /* minute of the hour           */
  unsigned short seconds;             /* second of the minute         */
  unsigned short hundredths;          /* hundredths of a second       */
  unsigned short day;                 /* day of the month             */
  unsigned short month;               /* month of the year            */
  unsigned short year;                /* current year                 */
  unsigned short weekday;             /* day of the week              */
  unsigned long  microseconds;        /* microseconds                 */
  unsigned long  yearday;             /* day number within the year   */
  unsigned short valid;               /* valid time stamp marker      */
} REXXDATETIME;
                                      /* ---------------------------- */
#endif                                /* _REXXDATETIME                */

#define DATETIME REXXDATETIME
typedef REXXDATETIME *PDATETIME;
typedef RXSTRING     *PRXSTRING;      /* pointer to a RXSTRING        */

#define RXAUTOBUFLEN         256L


/* Structure for system exit block (RXSYSEXIT)  --------------------- */
typedef struct _RXSYSEXIT {            /* syseexit                    */
   PSZ   sysexit_name;                 /*   exit handler name         */
   LONG  sysexit_code;                 /*   exit function code        */
}  RXSYSEXIT;

typedef RXSYSEXIT *PRXSYSEXIT;         /* pointer to a RXSYSEXIT      */

/* Macros for RXSTRING manipulation --------------------------------- */
#define RXNULLSTRING(r)      (!(r).strptr)
#define RXZEROLENSTRING(r)   ((r).strptr && !(r).strlength)
#define RXVALIDSTRING(r)     ((r).strptr && (r).strlength)
#define RXSTRLEN(r)          (RXNULLSTRING(r)?0L:(r).strlength)
#define RXSTRPTR(r)          (r).strptr
#define MAKERXSTRING(r,p,l)  {(r).strptr=(PCH)p;(r).strlength=(ULONG)l;}

/* Call type codes for use on interpreter startup ------------------- */
#define RXCOMMAND       0              /* Program called as Command   */
#define RXSUBROUTINE    1              /* Program called as Subroutin */
#define RXFUNCTION      2              /* Program called as Function  */

/**********************************************************************/
/***    Subcommand Interface defines                                ***/
/**********************************************************************/
#ifdef INCL_RXSUBCOM

/* Drop Authority for RXSUBCOM interface ---------------------------- */
#define RXSUBCOM_DROPPABLE   0x00     /* handler to be dropped by all */
#define RXSUBCOM_NONDROP     0x01     /* process with same PID as the */
                                      /* registrant may drop environ  */

/* Return Codes from RXSUBCOM interface ----------------------------- */
#define RXSUBCOM_ISREG       0x01     /* Subcommand is registered     */
#define RXSUBCOM_ERROR       0x01     /* Subcommand Ended in Error    */
#define RXSUBCOM_FAILURE     0x02     /* Subcommand Ended in Failure  */
#define RXSUBCOM_BADENTRY    1001     /* Invalid Entry Conditions     */
#define RXSUBCOM_NOEMEM      1002     /* Insuff stor to complete req  */
#define RXSUBCOM_BADTYPE     1003     /* Bad registration type.       */
#define RXSUBCOM_NOTINIT     1004     /* API system not initialized.  */
#define RXSUBCOM_OK           0       /* Function Complete            */
#define RXSUBCOM_DUP         10       /* Duplicate Environment Name-  */
                                      /* but Registration Completed   */
#define RXSUBCOM_PARTOFPKG   20       /* dropped package was part of  */
                                      /* a package, but was dropped   */
                                      /* changed for aix port...      */
/*#define RXSUBCOM_MAXREG      20*/   /* Cannot register more         */
                                      /* handlers                     */
#define RXSUBCOM_NOTREG      30       /* Name Not Registered          */
#define RXSUBCOM_NOCANDROP   40       /* Name not droppable           */
#define RXSUBCOM_LOADERR     50       /* Could not load function      */
#define RXSUBCOM_NOPROC     127       /* RXSUBCOM routine - not found */
#define RXSUBCOM_NOMEM     1001       /* Insuff stor to complete req  */

/* Type definition for REXX subcommand handler function pointer ----- */
typedef LONG (*PRXSUBCOM)(
                   PRXSTRING,          /* Command string from REXX    */
                   PUSHORT,            /* Return flag (ERROR/FAILURE) */
                   PRXSTRING);         /* RC variable return value    */

/* ------------------------------------------------------------------ */
/* This structure defines one element of an array of blocks whose     */
/* anchor is to be returned by the initial load function of a         */
/* subcommand package.  It identifies the functions in the package:   */
/* names, and addresses.  The last element of the array should        */
/* contain NULL pointers.                                             */
typedef struct _RXSUBCOMBLOCK {
  PSZ name;                            /* Name of function            */
  PRXSUBCOM subcom;                    /* Pointer to subcommand hdlr  */
  PUCHAR userarea;                     /* Pointer to 8-byte user area */
} RXSUBCOMBLOCK;

/* ------------------------------------------------------------------ */
/* Type definition of the initialization routine's pointer in a       */
/* REXX External Function Library.                                    */
typedef USHORT (*PRXINITSUBCOMPKG)(RXSUBCOMBLOCK**);
                                      /* ---------------------------- */
#endif                                /* INCL_RXSUBCOM                */

/**********************************************************************/
/***    Shared Variable Pool Interface defines                      ***/
/**********************************************************************/
#ifdef INCL_RXSHV

/* Function Codes for Variable Pool Interface (shvcode) ------------- */
#define RXSHV_SET          0x00       /* Set var from given value     */
#define RXSHV_FETCH        0x01       /* Copy value of var to buffer  */
#define RXSHV_DROPV        0x02       /* Drop variable                */
#define RXSHV_SYSET        0x03       /* Symbolic name Set variable   */
#define RXSHV_SYFET        0x04       /* Symbolic name Fetch variable */
#define RXSHV_SYDRO        0x05       /* Symbolic name Drop variable  */
#define RXSHV_NEXTV        0x06       /* Fetch "next" variable        */
#define RXSHV_PRIV         0x07       /* Fetch private information    */
#define RXSHV_EXIT         0x08       /* Set function exit value      */

/* Return Codes for Variable Pool Interface ------------------------- */
#define RXSHV_NOAVL         144       /* Interface not available      */

/* Return Code Flags for Variable Pool Interface (shvret) ----------- */
#define RXSHV_OK           0x00       /* Execution was OK             */
#define RXSHV_NEWV         0x01       /* Variable did not exist       */
#define RXSHV_LVAR         0x02       /* Last var trans via SHVNEXTV  */
#define RXSHV_TRUNC        0x04       /* Truncation occurred-Fetch    */
#define RXSHV_BADN         0x08       /* Invalid variable name        */
#define RXSHV_MEMFL        0x10       /* Out of memory failure        */
#define RXSHV_BADF         0x80       /* Invalid funct code (shvcode) */

/* Structure of Shared Variable Request Block (SHVBLOCK) ------------ */
typedef struct _SHVBLOCK {            /* shvb */
    struct _SHVBLOCK  *shvnext;       /* pointer to the next block    */
    RXSTRING           shvname;       /* Pointer to the name buffer   */
    RXSTRING           shvvalue;      /* Pointer to the value buffer  */
    ULONG              shvnamelen;    /* Length of the name value     */
    ULONG              shvvaluelen;   /* Length of the fetch value    */
    UCHAR              shvcode;       /* Function code for this block */
    UCHAR              shvret;        /* Individual Return Code Flags */
}   SHVBLOCK;

typedef SHVBLOCK *PSHVBLOCK;
                                      /* ---------------------------- */
#endif                                /* INCL_RXSHV                   */

/**********************************************************************/
/***    External Function Interface                                 ***/
/**********************************************************************/
#ifdef INCL_RXFUNC

/* Registration Type Identifiers for Available Function Table ------- */
#define RXFUNC_DYNALINK       1        /* Function Available in LIB   */
#define RXFUNC_CALLENTRY      2        /* Registered as mem entry pt. */

/* Return Codes from RxFunction interface --------------------------- */
#define RXFUNC_OK             0        /* REXX-API Call Successful    */
#define RXFUNC_DEFINED       10        /* function is                 */
                                       /* already  registered         */
#define RXFUNC_PARTOFPKG     20        /* Dropped handler was part of */
                                       /* a package, but WAS dropped  */
#define RXFUNC_NOTREG        30        /* Funct Not Registered in AFT */
#define RXFUNC_MODNOTFND     40        /* Funct LIB Module Not Found  */
#define RXFUNC_ENTNOTFND     50        /* Funct Entry Point Not Found */
#define RXFUNC_NOTINIT       60        /* API not initialized         */
#define RXFUNC_BADTYPE       70        /* Bad function type           */
#define RXFUNC_NOMEM       1001        /* No memory to register func  */
                                       /* --------------------------- */
#endif                                 /* INCL_RXFUNC                 */

/**********************************************************************/
/***   System Exits defines                                         ***/
/**********************************************************************/
#ifdef INCL_RXSYSEXIT

/* Drop Authority for Rexx Exit interface --------------------------- */
#define RXEXIT_DROPPABLE     0x00     /* handler to be dropped by all */
#define RXEXIT_NONDROP       0x01     /* process with same PID as the */
                                      /* registrant may drop environ  */

/* Exit return actions ---------------------------------------------- */
#define RXEXIT_HANDLED       0        /* Exit handled exit event      */
#define RXEXIT_NOT_HANDLED   1        /* Exit passes on exit event    */
#define RXEXIT_RAISE_ERROR   (-1)     /* Exit handler error occurred  */

/* Return Codes from RXEXIT interface ------------------------------- */
#define RXEXIT_ISREG         0x01     /* Exit is registered           */
#define RXEXIT_ERROR         0x01     /* Exit Ended in Error          */
#define RXEXIT_FAILURE       0x02     /* Exit Ended in Failure        */
#define RXEXIT_BADENTRY    1001       /* Invalid Entry Conditions     */
#define RXEXIT_NOEMEM      1002       /* Insuff stor to complete req  */
#define RXEXIT_BADTYPE     1003       /* Bad registration type.       */
#define RXEXIT_NOTINIT     1004       /* API system not initialized.  */
#define RXEXIT_OK             0       /* Function Complete            */
#define RXEXIT_DUP           10       /* Duplicate Exit Name-         */
                                      /* but Registration Completed   */
#define RXEXIT_MAXREG        20       /* Cannot register more         */
                                      /* handlers                     */
#define RXEXIT_NOTREG        30       /* Name Not Registered          */
#define RXEXIT_NOCANDROP     40       /* Name not droppable           */
#define RXEXIT_LOADERR       50       /* Could not load function      */
#define RXEXIT_NOPROC       127       /* RXEXIT routine - not found   */

/* System Exit function and sub-function definitions ---------------- */
#define RXENDLST    0                 /* End of exit list.            */
#define RXFNC    2                    /* Process external functions.  */
#define    RXFNCCAL 1                 /* subcode value.               */
#define RXCMD    3                    /* Process host commands.       */
#define    RXCMDHST 1                 /* subcode value.               */
#define RXMSQ    4                    /* Manipulate queue.            */
#define    RXMSQPLL 1                 /* Pull a line from queue       */
#define    RXMSQPSH 2                 /* Place a line on queue        */
#define    RXMSQSIZ 3                 /* Return num of lines on queue */
#define    RXMSQNAM 20                /* Set active queue name        */
#define RXSIO    5                    /* Session I/O.                 */
#define    RXSIOSAY 1                 /* SAY a line to STDOUT         */
#define    RXSIOTRC 2                 /* Trace output                 */
#define    RXSIOTRD 3                 /* Read from char stream        */
#define    RXSIODTR 4                 /* DEBUG read from char stream  */
#define    RXSIOTLL 5                 /* Return linelength(N/A OS/2)  */
#define RXHLT    7                    /* Halt processing.             */
#define    RXHLTCLR 1                 /* Clear HALT indicator         */
#define    RXHLTTST 2                 /* Test HALT indicator          */
#define RXTRC    8                    /* Test exit trace indicator.   */
#define    RXTRCTST 1                 /* subcode value.               */
#define RXINI    9                    /* Initialization processing.   */
#define    RXINIEXT 1                 /* subcode value.               */
#define RXTER   10                    /* Termination processing.      */
#define    RXTEREXT 1                 /* subcode value.               */

#ifdef INCL_RXOBJECT                  /* ---------------------------- */
#define RXDBG    11                   /* Test exit trace indicator    */
                                      /* before instruction.          */
#define    RXDBGTST  1                /* subcode value.               */
/* Return codes for the debug exit ---------------------------------- */
#define    RXDBGOFF  0                /* set trace off                */
#define    RXDBGSTEPIN   1            /* set trace on (normal)        */
#define    RXDBGSTEPOVER 2            /* set trace on but step over   */
#define    RXDBGSTEPOUT  3            /* set step out                 */
#define    RXDBGENDSTEP 4             /* leaving a subroutine         */
#define    RXDBGENTERSUB 5            /* entering a subroutine        */
#define    RXDBGLEAVESUB 6            /* leaving a subroutine         */
#define    RXDBGLOCATELINE 7          /* locate current line          */
#define    RXDBGSTEPAGAIN 8           /* repeat debug pause           */
#define    RXDBGSIGNAL 9              /* processing a signal instr.   */
#define    RXDBGRECURSIVEOFF 10       /* switch dbg off recursively  */
#define RXNOOFEXITS 12                /* 1 + largest exit number.     */
#else                                 /* ---------------------------- */
#define RXNOOFEXITS 11                /* 1 + largest exit number.     */
                                      /* ---------------------------- */
#endif                                /* INCL_RXOBJECT                */

typedef PUCHAR PEXIT;                 /* ptr to exit parameter block  */

/* Type definition for REXX exit handler function pointer ----------- */
typedef LONG (*PRXEXIT)(
                   LONG,
                   LONG,
                   PEXIT);
                                      /* ---------------------------- */
#endif                                /* INCL_RXSYSEXIT               */

/**********************************************************************/
/***    Asynchronous Request Interface defines                      ***/
/**********************************************************************/
#ifdef INCL_RXARI

/* Return Codes from Asynchronous Request interface ----------------- */
#define RXARI_OK                   0  /* Interface completed          */
#define RXARI_NOT_FOUND            1  /* Target program not found     */
#define RXARI_PROCESSING_ERROR     2  /* Error processing request     */
                                      /* ---------------------------- */
#endif                                /* INCL_RXARI                   */

/**********************************************************************/
/***    Macro Space Interface defines                                 */
/**********************************************************************/
#ifdef INCL_RXMACRO

/* Registration Search Order Flags ---------------------------------- */
#define RXMACRO_SEARCH_BEFORE       1  /* Beginning of search order   */
#define RXMACRO_SEARCH_AFTER        2  /* End of search order         */

/* Return Codes from RxMacroSpace interface ------------------------- */
#define RXMACRO_OK                 0  /* Macro interface completed    */
#define RXMACRO_NO_STORAGE         1  /* Not Enough Storage Available */
#define RXMACRO_NOT_FOUND          2  /* Requested function not found */
#define RXMACRO_EXTENSION_REQUIRED 3  /* File ext required for save   */
#define RXMACRO_ALREADY_EXISTS     4  /* Macro functions exist        */
#define RXMACRO_FILE_ERROR         5  /* File I/O error in save/load  */
#define RXMACRO_SIGNATURE_ERROR    6  /* Incorrect format for load    */
#define RXMACRO_SOURCE_NOT_FOUND   7  /* Requested cannot be found    */
#define RXMACRO_INVALID_POSITION   8  /* Invalid search order pos     */
#define RXMACRO_NOT_INIT           9  /* API not initialized          */
                                      /* ---------------------------- */
#endif                                /* INCL_RXMACRO                 */


/**********************************************************************/
/***                                                                ***/
/***           Main Entry Points to the REXX Interpreter            ***/
/***                                                                ***/
/**********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/* Type definition for RexxStart function pointer (dynamic linking)-- */
typedef APIRET  (*PRXSTART)(
                   LONG,
                   PRXSTRING,
                   PSZ,
                   PRXSTRING,
                   PSZ,
                   LONG,
                   PRXSYSEXIT,
                   PSHORT,
                   PRXSTRING );

/* Function Prototypes ---------------------------------------------- */
LONG   APIENTRY RexxStart(
         LONG ,                        /* Num of args passed to rexx  */
         PRXSTRING,                    /* Array of args passed to rex */
         PSZ,                          /* [d:][path] filename[.ext]   */
         PRXSTRING,                    /* Loc of rexx proc in memory  */
         PSZ,                          /* ASCIIZ initial environment. */
         LONG ,                        /* type (command,subrtn,funct) */
         PRXSYSEXIT,                   /* SysExit env. names &  codes */
         PSHORT,                       /* Ret code from if numeric    */
         PRXSTRING );                  /* Retvalue from the rexx proc */

typedef  void (*PRXWAITFORTERMINATION)(void);

void   APIENTRY RexxWaitForTermination(void);

APIRET APIENTRY RexxDidRexxTerminate(void);

/* Uppercase Entry Point Names -------------------------------------- */
#define REXXSTART   RexxStart

#define REXXWAITFORTERMINATION RexxWaitForTermination

#define REXXDIDREXXTERMINATE   RexxDidRexxTermiate

#ifdef __cplusplus
}
#endif

/**********************************************************************/
/***    Subcommand Interface                                        ***/
/**********************************************************************/
#ifdef INCL_RXSUBCOM

/* Type definition to simplify coding of a Subcommand handler ------- */

typedef ULONG RexxSubcomHandler(PRXSTRING,
                                PUSHORT,
                                PRXSTRING);


/* RexxRegisterSubcomDll -- Register a library entry point ---------- */
/* as a Subcommand handler                                            */
#ifdef __cplusplus
extern "C" {
#endif

APIRET APIENTRY RexxRegisterSubcomDll (
         PSZ,                          /* Name of subcom handler      */
         PSZ,                          /* Name of library             */
         PSZ,                          /* Name of procedure in library*/
         PUCHAR,                       /* User area                   */
         ULONG  );                     /* Drop authority.             */

/* Uppercase Entry Point Name --------------------------------------- */
#define REXXREGISTERSUBCOMDLL  RexxRegisterSubcomDll

/* RexxRegisterSubcomExe -- Register an EXE entry point ------------- */
/* as a Subcommand handler                                            */
APIRET APIENTRY RexxRegisterSubcomExe (
         PSZ,                          /* Name of subcom handler      */
         PFN,                          /* address of handler in EXE   */
         PUCHAR);                      /* User area                   */

/* Uppercase Entry Point Name --------------------------------------- */
#define REXXREGISTERSUBCOMEXE  RexxRegisterSubcomExe

/* RexxQuerySubcom - Query an environment for Existence ------------- */
APIRET APIENTRY RexxQuerySubcom(
         PSZ,                          /* Name of the Environment     */
         PSZ,                          /* Library Module Name         */
         PUSHORT,                      /* Stor for existence code     */
         PUCHAR );                     /* Stor for user word          */


/* Uppercase Entry Point Name --------------------------------------- */
#define REXXQUERYSUBCOM  RexxQuerySubcom

/* RexxDeregisterSubcom - Drop registration of a Subcommand --------- */
/* environment                                                        */
APIRET APIENTRY RexxDeregisterSubcom(
         PSZ,                          /* Name of the Environment     */
         PSZ );                        /* Library Module Name         */

/* Uppercase Entry Point Name --------------------------------------- */
#define REXXDEREGISTERSUBCOM  RexxDeregisterSubcom
                                      /* ---------------------------- */
#ifdef __cplusplus
}
#endif
                                      /* ---------------------------- */
#endif                                /* INCL_RXSUBCOM                */



/**********************************************************************/
/***                Shared Variable Pool Interface                  ***/
/**********************************************************************/
#ifdef INCL_RXSHV

/* RexxVariablePool - Request Variable Pool Service ----------------- */
#ifdef __cplusplus
extern "C" {
#endif

APIRET APIENTRY RexxVariablePool(
         PSHVBLOCK);                  /* Pointer to list of SHVBLOCKs */

/* Uppercase Entry Point Name --------------------------------------- */
#define REXXVARIABLEPOOL  RexxVariablePool
                                      /* ---------------------------- */
#ifdef __cplusplus
}
#endif
                                      /* ---------------------------- */
#endif                                /* INCL_RXSHV                   */

/**********************************************************************/
/***                  External Function Interface                   ***/
/**********************************************************************/
#ifdef INCL_RXFUNC

/* Type definition for REXX external function pointer --------------- */
typedef ULONG (*PRXFUNC)(
                   PUCHAR,             /* Name of the function        */
                   ULONG,              /* Number of arguments         */
                   PRXSTRING,          /* Array of argument strings   */
                   PSZ,                /* Current queue name          */
                   PRXSTRING);         /* Returned result string      */
/* ------------------------------------------------------------------ */
/* This structure defines one element of an array of blocks whose     */
/* anchor is to be returned by the initial load function of a         */
/* function package.  It identifies the functions in the package:     */
/* names, and addresses.  The last element of the array should        */
/* contain NULL pointers.                                             */

typedef struct _RXFUNCBLOCK {
  PSZ name;                            /* Name of function            */
  PRXFUNC function;                    /* Pointer to function         */
  PUCHAR userarea;                     /* pointer to 8-byte user data */
} RXFUNCBLOCK;

/* ------------------------------------------------------------------ */
/* Type definition of the initialization routine's pointer in a       */
/* REXX External Function Library.                                    */
typedef USHORT (*PRXINITFUNCPKG)(RXFUNCBLOCK**);


/* This typedef simplifies coding of an External Function ----------- */

typedef ULONG RexxFunctionHandler(PUCHAR,
                                  ULONG,
                                  PRXSTRING,
                                  PSZ,
                                  PRXSTRING);

/* RexxRegisterFunctionDll - Register a package of external --------- */
/* function handlers                                                  */
#ifdef __cplusplus
extern "C" {
#endif

APIRET APIENTRY RexxRegisterFunctionDll (
        PSZ,                          /* Name of function to add      */
        PSZ,                          /* Name of library              */
        PSZ);                         /* Entry in library             */

/* Uppercase Entry Point Name --------------------------------------- */
#define REXXREGISTERFUNCTIONDLL  RexxRegisterFunctionDll


/* RexxRegisterFunctionExe - Register a function in the AFT --------- */
APIRET APIENTRY RexxRegisterFunctionExe (
        PSZ,                          /* Name of function to add      */
        PFN);                         /* Entry point in EXE           */

/* Uppercase Entry Point Name --------------------------------------- */
#define REXXREGISTERFUNCTIONEXE  RexxRegisterFunctionExe

/* RexxDeregisterFunction - Delete a function from the AFT ---------- */
APIRET APIENTRY RexxDeregisterFunction (
        PSZ );                        /* Name of function to remove   */

/* Uppercase Entry Point Name --------------------------------------- */
#define REXXDEREGISTERFUNCTION  RexxDeregisterFunction

/* RexxQueryFunction - Scan the AFT for a function ------------------ */
APIRET APIENTRY RexxQueryFunction (
        PSZ);                         /* Name of function to find     */

/* Uppercase Entry Point Name --------------------------------------- */
#define REXXQUERYFUNCTION  RexxQueryFunction

/* RexxShutDownAPI - Remove user specific shared memory ------------- */
APIRET APIENTRY RexxShutDownAPI(VOID);

/* Uppercase Entry Point Name --------------------------------------- */
#define REXXSHUTDOWNAPI  RexxShutDownAPI
                                      /* ---------------------------- */
#ifdef __cplusplus
}
#endif
                                      /* ---------------------------- */
#endif                                /* INCL_RXFUNC                  */


/**********************************************************************/
/***   System Exits                                                 ***/
/**********************************************************************/
#ifdef INCL_RXSYSEXIT

/* Subfunction RXFNCCAL - External Function Calls ------------------- */


typedef struct _RXFNC_FLAGS {          /* fl */
   unsigned rxfferr  : 1;              /* Invalid call to routine.    */
   unsigned rxffnfnd : 1;              /* Function not found.         */
   unsigned rxffsub  : 1;              /* Called as a subroutine      */
}  RXFNC_FLAGS ;




typedef struct _RXFNCCAL_PARM {        /* fnc */
   RXFNC_FLAGS       rxfnc_flags ;     /* function flags              */
   PUCHAR            rxfnc_name;       /* Pointer to function name.   */
   USHORT            rxfnc_namel;      /* Length of function name.    */
   PUCHAR            rxfnc_que;        /* Current queue name.         */
   USHORT            rxfnc_quel;       /* Length of queue name.       */
   USHORT            rxfnc_argc;       /* Number of args in list.     */
   PRXSTRING         rxfnc_argv;       /* Pointer to argument list.   */
   RXSTRING          rxfnc_retc;       /* Return value.               */
}  RXFNCCAL_PARM;


/* Subfunction RXCMDHST -- Process Host Commands -------------------- */


typedef struct _RXCMD_FLAGS {          /* fl */
   unsigned rxfcfail : 1;              /* Command failed.             */
   unsigned rxfcerr  : 1;              /* Command ERROR occurred.     */
}  RXCMD_FLAGS;


typedef struct _RXCMDHST_PARM {        /* rx */
   RXCMD_FLAGS       rxcmd_flags;      /* error/failure flags         */
   PUCHAR            rxcmd_address;    /* Pointer to address name     */
   USHORT            rxcmd_addressl;   /* Length of address name      */
   PUCHAR            rxcmd_dll;        /* Library name for command    */
   USHORT            rxcmd_dll_len;    /* Length of library name      */
   RXSTRING          rxcmd_command;    /* The command string          */
   RXSTRING          rxcmd_retc;       /* Pointer to return buffer    */
}  RXCMDHST_PARM;

/* Subfunction RXMSQPLL -- Pull Entry from Queue -------------------- */

typedef struct _RXMSQPLL_PARM {        /* pll */
   RXSTRING          rxmsq_retc;       /* Pointer to dequeued entry   */
                                       /* buffer.  User allocated.    */
} RXMSQPLL_PARM;

/* Subfunction RXMSQPSH -- Push Entry on Queue ---------------------- */

typedef struct _RXMSQ_FLAGS {          /* fl */
   unsigned rxfmlifo : 1;              /* Stack entry LIFO if set     */
}  RXMSQ_FLAGS;

typedef struct _RXMSQPSH_PARM {        /* psh */
   RXMSQ_FLAGS       rxmsq_flags;      /* LIFO/FIFO flag              */
   RXSTRING          rxmsq_value;      /* The entry to be pushed.     */
}  RXMSQPSH_PARM;

/* Subfunction RXMSQSIZ -- Return the Current Queue Size ------------ */

typedef struct _RXMSQSIZ_PARM {        /* siz */
   ULONG             rxmsq_size;       /* Number of Lines in Queue    */
}  RXMSQSIZ_PARM;

/* Subfunction RXMSQNAM -- Set Current Queue Name ------------------- */
typedef struct _RXMSQNAM_PARM {        /* nam */
   RXSTRING          rxmsq_name;       /* RXSTRING containing         */
                                       /* queue name.                 */
}  RXMSQNAM_PARM;

/* Subfunction RXSIOSAY -- Perform SAY Clause ----------------------- */
typedef struct _RXSIOSAY_PARM {        /* say */
   RXSTRING          rxsio_string;     /* String to display.          */
}  RXSIOSAY_PARM;

/* Subfunction RXSIOTRC -- Write Trace Output ----------------------- */
typedef struct _RXSIOTRC_PARM {        /* trcparm */
   RXSTRING          rxsio_string;     /* Trace line to display.      */
}  RXSIOTRC_PARM;

/* Subfunction RXSIOTRD -- Read Input from the Terminal ------------- */
typedef struct _RXSIOTRD_PARM {        /* trd */
   RXSTRING          rxsiotrd_retc;    /* RXSTRING for output.        */
}  RXSIOTRD_PARM;

/* Subfunction RXSIODTR -- Read Debug Input from the Terminal ------- */
typedef struct _RXSIODTR_PARM {        /* dtr */
   RXSTRING          rxsiodtr_retc;    /* RXSTRING for output.        */
}  RXSIODTR_PARM;


/* Subfunction RXHSTTST -- Test for HALT Condition ------------------ */
typedef struct _RXHLT_FLAGS {          /* fl Halt flag                */
   unsigned rxfhhalt : 1;              /* Set if HALT occurred.       */
}  RXHLT_FLAGS;

typedef struct _RXHLTTST_PARM {        /* tst */
   RXHLT_FLAGS rxhlt_flags;            /* Set if HALT occurred        */
}  RXHLTTST_PARM;

/* Subfunction RXTRCTST -- Test for TRACE Condition ----------------- */
typedef struct _RXTRC_FLAGS {          /* fl Trace flags              */
   unsigned rxftrace : 1;              /* Set to run external trace.  */
}  RXTRC_FLAGS;

typedef struct _RXTRCTST_PARM {        /* tst */
   RXTRC_FLAGS rxtrc_flags;            /* Set to run external trace   */
}  RXTRCTST_PARM;


#ifdef INCL_RXOBJECT
typedef struct _RXDBG_FLAGS {          /* fl Trace flags              */
   unsigned rxftrace;                  /* Set to run external trace.  */
}  RXDBG_FLAGS;

typedef struct _RXDBGTST_PARM {   /* tst */
   RXDBG_FLAGS rxdbg_flags;       /* Set to run external trace before */
   ULONG       rxdbg_line;
   RXSTRING    rxdbg_filename;
   RXSTRING    rxdbg_routine;
}  RXDBGTST_PARM;                      /* --------------------------- */
#endif                                 /* INCL_RXOBJECT               */

/* XLATOFF */

/* This typedef simplifies coding of an Exit handler ---------------- */
typedef LONG RexxExitHandler(LONG,
                             LONG,
                             PEXIT);

/* RexxRegisterExitDll - Register a system exit --------------------- */
#ifdef __cplusplus
extern "C" {
#endif

APIRET APIENTRY RexxRegisterExitDll (
         PSZ,                          /* Name of the exit handler    */
         PSZ,                          /* Name of the library         */
         PSZ,                          /* Name of the procedure       */
         PUCHAR,                       /* User area                   */
         ULONG );                      /* Drop authority              */

/* Uppercase Entry Point Name --------------------------------------- */
#define REXXREGISTEREXITDLL  RexxRegisterExitDll


/* RexxRegisterExitExe - Register a system exit --------------------- */
APIRET APIENTRY RexxRegisterExitExe (
         PSZ,                          /* Name of the exit handler    */
         PFN,                          /* Address of exit handler     */
         PUCHAR);                      /* User area                   */

/* Uppercase Entry Point Name --------------------------------------- */
#define REXXREGISTEREXITEXE  RexxRegisterExitExe

/* RexxDeregisterExit - Drop registration of a system exit ---------- */
APIRET APIENTRY RexxDeregisterExit (
         PSZ,                          /* Exit name                   */
         PSZ ) ;                       /* Library module name         */

/* Uppercase Entry Point Name --------------------------------------- */
#define REXXDEREGISTEREXIT  RexxDeregisterExit

/* RexxQueryExit - Query an exit for existence ---------------------- */
APIRET APIENTRY RexxQueryExit (
         PSZ,                          /* Exit name                   */
         PSZ,                          /* Library module name         */
         PUSHORT,                      /* Existence flag              */
         PUCHAR );                     /* User data                   */

/* Uppercase Entry Point Name --------------------------------------- */
#define REXXQUERYEXIT  RexxQueryExit
                                       /* --------------------------- */
#ifdef __cplusplus
}
#endif
/* XLATON */
                                       /* --------------------------- */
#endif                                 /* INCL_RXSYSEXIT              */

/**********************************************************************/
/***    Asynchronous Request Interface                              ***/
/**********************************************************************/
#ifdef INCL_RXARI

/* XLATOFF */

#ifdef __cplusplus
extern "C" {
#endif

/* RexxSetHalt - Request Program Halt ------------------------------- */

APIRET APIENTRY RexxSetHalt(
         PID,                         /* Process Id                   */
         TID);                        /* Thread Id                    */

/* Uppercase Entry Point Name --------------------------------------- */
#define REXXSETHALT  RexxSetHalt


/* RexxSetTrace - Request Program Trace ----------------------------- */

APIRET APIENTRY RexxSetTrace(
         PID,                         /* Process Id                   */
         TID);                        /* Thread Id                    */

/* Uppercase Entry Point Name --------------------------------------- */
#define REXXSETTRACE  RexxSetTrace


/* RexxResetTrace - Turn Off Program Trace -------------------------- */

APIRET APIENTRY RexxResetTrace(
         PID,                         /* Process Id                   */
         TID);                        /* Thread Id                    */

/* Uppercase Entry Point Name --------------------------------------- */
#define REXXRESETTRACE  RexxResetTrace


#ifdef __cplusplus
}
#endif
/* XLATON */
                                      /* ---------------------------- */
#endif                                /* INCL_RXARI                   */

/**********************************************************************/
/*                      Queueing Services                             */
/**********************************************************************/
#ifdef INCL_RXQUEUE

/* Request flags for External Data Queue access --------------------- */
#define RXQUEUE_FIFO          0    /* Access queue first-in-first-out */
#define RXQUEUE_LIFO          1    /* Access queue last-in-first-out  */

#define RXQUEUE_NOWAIT        0    /* Wait for data if queue empty    */
#define RXQUEUE_WAIT          1    /* Don't wait on an empty queue    */

/* Return Codes from RxQueue interface ------------------------------ */
#define RXQUEUE_OK            0        /* Successful return           */
#define RXQUEUE_NOTINIT    1000        /* Queues not initialized      */

#define RXQUEUE_STORAGE       1        /* Ret info buf not big enough */
#define RXQUEUE_SIZE          2        /* Data size > 64K-64          */
#define RXQUEUE_DUP           3        /* Attempt-duplicate queue name*/
#define RXQUEUE_NOEMEM        4        /* Not enough available memory */
#define RXQUEUE_BADQNAME      5        /* Not a valid queue name      */
#define RXQUEUE_PRIORITY      6        /* Not accessed as LIFO|FIFO   */
#define RXQUEUE_BADWAITFLAG   7        /* Not accessed as WAIT|NOWAIT */
#define RXQUEUE_EMPTY         8        /* No data in queue            */
#define RXQUEUE_NOTREG        9        /* Queue does not exist        */
#define RXQUEUE_ACCESS       10        /* Queue busy and wait active  */
#define RXQUEUE_MAXREG       11        /* No memory to create a queue */
#define RXQUEUE_MEMFAIL      12        /* Failure in memory management*/
                                       /* --------------------------- */
#ifdef __cplusplus
extern "C" {
#endif


/* Type definition for RexxCreateQueue function pointer (dynamic linking)-- */

/* RexxCreateQueue - Create an External Data Queue ------------------ */
ULONG  APIENTRY RexxCreateQueue (
        PSZ,                           /* Name of queue created       */
        ULONG,                         /* Size of buf for ret name    */
        PSZ,                           /* Requested name for queue    */
        PULONG ) ;                     /* Duplicate name flag.        */

/* Type definition for RexxCreateQueue function pointer (dynamic linking)-- */

typedef ULONG (*APIENTRY PFNREXXCREATEQUEUE)(PSZ, ULONG, PSZ, PULONG);

/* RexxDeleteQueue - Delete an External Data Queue ------------------ */
ULONG  APIENTRY RexxDeleteQueue (
        PSZ );                         /* Name of queue to be deleted */

/* Type definition for RexxDeleteQueue function pointer (dynamic linking)-- */

typedef ULONG (*APIENTRY PFNREXXDELETEQUEUE)(PSZ);

/* RexxQueryQueue - Query an External Data Queue for number of entries*/
ULONG  APIENTRY RexxQueryQueue (
        PSZ,                           /* Name of queue to query      */
        PULONG );                      /* Place to put element count  */

/* Type definition for RexxQueryQueue function pointer (dynamic linking)-- */

typedef ULONG (*APIENTRY PFNREXXQUERYQUEUE)(PSZ, PULONG);

/* RexxAddQueue - Add an entry to an External Data Queue ------------ */
ULONG  APIENTRY RexxAddQueue (
        PSZ,                           /* Name of queue to add to     */
        PRXSTRING,                     /* Data string to add          */
        ULONG );                       /* Queue type (FIFO|LIFO)      */

/* Type definition for RexxAddQueue function pointer (dynamic linking)-- */

typedef ULONG (*APIENTRY PFNREXXADDQUEUE)(PSZ, PRXSTRING, ULONG);

/* RexxPullQueue - Retrieve data from an External Data Queue -------- */
ULONG  APIENTRY RexxPullQueue (
        PSZ,                           /* Name of queue to read from  */
        PRXSTRING,                     /* RXSTRING to receive data    */
        PDATETIME,                     /* Stor for data date/time     */
        ULONG );                       /* wait status (WAIT|NOWAIT)   */
                                       /* --------------------------- */

/* Type definition for RexxPullQueue function pointer (dynamic linking)-- */

typedef ULONG (*APIENTRY PFNREXXPULLQUEUE)(PSZ, PRXSTRING, PDATETIME,
                                           ULONG);
#ifdef __cplusplus
}
#endif
                                       /* --------------------------- */
#endif                                 /* INCL_RXQUEUE                */
                                       /* --------------------------- */
#endif                                 /* REXXSAA_INCLUDED            */

/**********************************************************************/
/***    Macro Space Interface                                       ***/
/**********************************************************************/

#ifdef INCL_RXMACRO

/* XLATOFF */

/* RexxAddMacro - Register a function in the Macro Space ------------ */
#ifdef __cplusplus
extern "C" {
#endif

APIRET APIENTRY RexxAddMacro (
         PSZ,                         /* Function to add or change    */
         PSZ,                         /* Name of file to get function */
         ULONG  );                    /* Flag indicating search pos   */

/* Uppercase Entry Point Name --------------------------------------- */
#define REXXADDMACRO  RexxAddMacro

/* RexxDropMacro - Remove a function from the Macro Space ----------- */
APIRET APIENTRY RexxDropMacro (
         PSZ );                        /* Name of function to remove  */

/* Uppercase Entry Point Name --------------------------------------- */
#define REXXDROPMACRO  RexxDropMacro

/* RexxSaveMacroSpace - Save Macro Space functions to a file -------- */
APIRET APIENTRY RexxSaveMacroSpace (
         ULONG ,                      /* Argument count (0==save all) */
         PSZ *,                       /* List of funct names to save  */
         PSZ);                        /* File to save functions in    */

/* Uppercase Entry Point Name --------------------------------------- */
#define REXXSAVEMACROSPACE  RexxSaveMacroSpace

/* RexxLoadMacroSpace - Load Macro Space functions from a file ------ */
APIRET APIENTRY RexxLoadMacroSpace (
         ULONG ,                      /* Argument count (0==load all) */
         PSZ *,                       /* List of funct names to load  */
         PSZ);                        /* File to load functions from  */

/* Uppercase Entry Point Name --------------------------------------- */
#define REXXLOADMACROSPACE  RexxLoadMacroSpace


/* RexxQueryMacro - Find a function's search-order position --------- */
APIRET APIENTRY RexxQueryMacro (
         PSZ,                         /* Function to search for       */
         PUSHORT );                   /* Ptr for position flag return */

/* Uppercase Entry Point Name --------------------------------------- */
#define REXXQUERYMACRO  RexxQueryMacro


/* RexxReorderMacro - Change a function's search-order position ----- */
APIRET APIENTRY RexxReorderMacro(
         PSZ,                         /* Name of funct change order   */
         ULONG  );                    /* New position for function    */

/* Uppercase Entry Point Name --------------------------------------- */
#define REXXREORDERMACRO  RexxReorderMacro

/* RexxClearMacroSpace - Remove all functions from a MacroSpace ----- */
APIRET APIENTRY RexxClearMacroSpace(
         VOID );                      /* No Arguments.                */

/* Uppercase Entry Point Name --------------------------------------- */
#define REXXCLEARMACROSPACE  RexxClearMacroSpace
                                      /* ---------------------------- */
#ifdef __cplusplus
}
#endif
/* XLATON */
                                      /* ---------------------------- */
#endif                                /* INCL_RXMACRO                 */
/* END -------------------------------------------------------------- */


/**********************************************************************/
/***    Allocte/Free Rexx Memory                                    ***/
/**********************************************************************/

/* #ifdef INCL_RXALLOCATEMEM */

/* XLATOFF */

/* RexxAddMacro - Register a function in the Macro Space ------------ */
#ifdef __cplusplus
extern "C" {
#endif

APIRET APIENTRY RexxFreeMemory (
         PVOID );

/* Uppercase Entry Point Name --------------------------------------- */
#define REXXFREEMEMORY RexxFreeMemory

PVOID  APIENTRY RexxAllocateMemory (
         ULONG );

/* Uppercase Entry Point Name --------------------------------------- */
#define REXXALLOCATEMEMORY RexxAllocateMemory

#ifdef __cplusplus
}
#endif
                                      /* ---------------------------- */
/*#endif                                INCL_RXALLOCATEMEM           */
/* END -------------------------------------------------------------- */

