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
/*********************************************************************\
*
* Module Name: REXX.H
*
* REXX Common Definitions File
*
* *********************************************************************
*
*   #define:              To include:
*
*   INCL_REXXSAA          Complete Rexx support
*   INCL_RXSUBCOM         Rexx subcommand handler support
*   INCL_RXSHV            Rexx shared variable pool support
*   INCL_RXFUNC           Rexx external function support
*   INCL_RXSYSEXIT        Rexx system exit support
*   INCL_RXMACRO          Rexx macro space support
*   INCL_RXARI            Rexx asynchronous Trace/Halt support
*   INCL_RXQUEUE          Rexx queue interface support
*
\*********************************************************************/

#ifndef REXXSAA_INCLUDED
#define REXXSAA_INCLUDED

/* XLATON */

#include <windows.h>

#define APIRET ULONG                    // API return types
typedef DWORD PID;                      // process id
typedef DWORD TID;                      // thread id (ignored on Windows)

#ifdef __cplusplus
#define PCSZ PCSTR
extern "C" {
#endif
   typedef APIRET (APIENTRY *PFN)();       // PFN pointer to a function
#ifdef __cplusplus
}
#endif

/*********************************************************************/
/*                                                                   */
/*                            Common                                 */
/*                                                                   */
/*********************************************************************/

/* This section defines return codes and constants for REXX calls    */

#ifdef INCL_REXXSAA
#define INCL_RXSUBCOM
#define INCL_RXSHV
#define INCL_RXFUNC
#define INCL_RXSYSEXIT
#define INCL_RXMACRO
#define INCL_RXARI
#define INCL_RXQUEUE
#endif /* INCL_REXXSAA */

/***    Structure for external interface string (RXSTRING) */

typedef struct _RXSTRING {          /* rxstr */
        ULONG  strlength;           /*   length of string         */
        PCH    strptr;              /*   pointer to string        */
}  RXSTRING;



typedef RXSTRING     *PRXSTRING;       /* pointer to a RXSTRING      */

#define RXAUTOBUFLEN         256L


/***    Structure for system exit block (RXSYSEXIT) 32-bit */

typedef struct _RXSYSEXIT {            /* syse */
   PSZ   sysexit_name;                 /* subcom enviro for sysexit  */
   LONG  sysexit_code;                 /* sysexit function code      */
}  RXSYSEXIT;


typedef RXSYSEXIT *PRXSYSEXIT;         /* pointer to a RXSYSEXIT     */


/* XLATOFF */
/***    Macros for RXSTRING manipulation                   */

#define RXNULLSTRING(r)      (!(r).strptr)
#define RXZEROLENSTRING(r)   ((r).strptr && !(r).strlength)
#define RXVALIDSTRING(r)     ((r).strptr && (r).strlength)
#define RXSTRLEN(r)          (RXNULLSTRING(r)?0L:(r).strlength)
#define RXSTRPTR(r)          (r).strptr
#define MAKERXSTRING(r,p,l)  {(r).strptr=(PCH)p;(r).strlength=(ULONG)l;}

/* XLATON */

/*** Call type codes for use on interpreter startup                  */
#define RXCOMMAND       0              /* Program called as Command  */
#define RXSUBROUTINE    1              /* Program called as Subroutin*/
#define RXFUNCTION      2              /* Program called as Function */

/***    Subcommand Interface defines*/
#ifdef INCL_RXSUBCOM


/***    Drop Authority for RXSUBCOM interface */

#define RXSUBCOM_DROPPABLE   0x00     /* handler to be dropped by all*/
#define RXSUBCOM_NONDROP     0x01     /* process with same PID as the*/
                                      /* registrant may drop environ */

/***    Return Codes from RXSUBCOM interface */

#define RXSUBCOM_ISREG       0x01     /* Subcommand is registered    */
#define RXSUBCOM_ERROR       0x01     /* Subcommand Ended in Error   */
#define RXSUBCOM_FAILURE     0x02     /* Subcommand Ended in Failure */
#define RXSUBCOM_BADENTRY    1001     /* Invalid Entry Conditions    */
#define RXSUBCOM_NOEMEM      1002     /* failure in memory manager   */
#define RXSUBCOM_BADTYPE     1003     /* Bad registration type.      */
#define RXSUBCOM_NOTINIT     1004     /* API system not initialized. */
#define RXSUBCOM_OK           0       /* Function Complete           */
#define RXSUBCOM_DUP         10       /* Duplicate Environment Name- */
                                      /* but Registration Completed  */
#define RXSUBCOM_MAXREG      20       /* Cannot register more        */
                                      /* handlers                    */
#define RXSUBCOM_NOTREG      30       /* Name Not Registered         */
#define RXSUBCOM_NOCANDROP   40       /* Name not droppable          */
#define RXSUBCOM_LOADERR     50       /* Could not load function     */
#define RXSUBCOM_NOPROC     127       /* RXSUBCOM routine - not found*/

#endif /* INCL_RXSUBCOM for defines */

/***    Shared Variable Pool Interface defines */
#ifdef INCL_RXSHV

/***    Function Codes for Variable Pool Interface (shvcode) */

#define RXSHV_SET          0x00       /* Set var from given value    */
#define RXSHV_FETCH        0x01       /* Copy value of var to buffer */
#define RXSHV_DROPV        0x02       /* Drop variable               */
#define RXSHV_SYSET        0x03       /* Symbolic name Set variable  */
#define RXSHV_SYFET        0x04       /* Symbolic name Fetch variable*/
#define RXSHV_SYDRO        0x05       /* Symbolic name Drop variable */
#define RXSHV_NEXTV        0x06       /* Fetch "next" variable       */
#define RXSHV_PRIV         0x07       /* Fetch private information   */
#define RXSHV_EXIT         0x08       /* Set function exit value     */

/***    Return Codes for Variable Pool Interface */

#define RXSHV_NOAVL         144       /* Interface not available     */

/***    Return Code Flags for Variable Pool Interface (shvret) */

#define RXSHV_OK           0x00       /* Execution was OK            */
#define RXSHV_NEWV         0x01       /* Variable did not exist      */
#define RXSHV_LVAR         0x02       /* Last var trans via SHVNEXTV */
#define RXSHV_TRUNC        0x04       /* Truncation occurred-Fetch   */
#define RXSHV_BADN         0x08       /* Invalid variable name       */
#define RXSHV_MEMFL        0x10       /* Out of memory failure       */
#define RXSHV_BADF         0x80       /* Invalid funct code (shvcode)*/


/***    Structure of Shared Variable Request Block (SHVBLOCK) */

typedef struct _SHVBLOCK {            /* shvb */
    struct _SHVBLOCK  *shvnext;       /* pointer to the next block   */
    RXSTRING           shvname;       /* Pointer to the name buffer  */
    RXSTRING           shvvalue;      /* Pointer to the value buffer */
    ULONG              shvnamelen;    /* Length of the name value    */
    ULONG              shvvaluelen;   /* Length of the fetch value   */
    UCHAR              shvcode;       /* Function code for this block*/
    UCHAR              shvret;        /* Individual Return Code Flags*/
}   SHVBLOCK;

typedef SHVBLOCK *PSHVBLOCK;

#endif /* INCL_RXSHV for defines */

/***    External Function Interface */
#ifdef INCL_RXFUNC

/***    Registration Type Identifiers for Available Function Table */

#define RXFUNC_DYNALINK       1        /* Function Available in DLL  */
#define RXFUNC_CALLENTRY      2        /* Registered as mem entry pt.*/


/***    Return Codes from RxFunction interface */


#define RXFUNC_OK             0        /* REXX-API Call Successful   */
#define RXFUNC_DEFINED       10        /* Function Defined in AFT    */
#define RXFUNC_NOMEM         20        /* Not Enough Mem to Add      */
#define RXFUNC_NOTREG        30        /* Funct Not Registered in AFT*/
#define RXFUNC_MODNOTFND     40        /* Funct Dll Module Not Found */
#define RXFUNC_ENTNOTFND     50        /* Funct Entry Point Not Found*/
#define RXFUNC_NOTINIT       60        /* API not initialized        */
#define RXFUNC_BADTYPE       70        /* Bad function type          */
#define RXFUNC_NOEMEM      1002        /* failure in memory manager  */
#endif /* INCL_RXFUNC for defines */

/***   System Exits defines */
#ifdef INCL_RXSYSEXIT

/***    Drop Authority for Rexx Exit interface */

#define RXEXIT_DROPPABLE     0x00     /* handler to be dropped by all*/
#define RXEXIT_NONDROP       0x01     /* process with same PID as the*/
                                      /* registrant may drop environ */


/***    Exit return actions */

#define RXEXIT_HANDLED       0        /* Exit handled exit event     */
#define RXEXIT_NOT_HANDLED   1        /* Exit passes on exit event   */
#define RXEXIT_RAISE_ERROR   (-1)     /* Exit handler error occurred */

/***    Return Codes from RXEXIT interface */

#define RXEXIT_ISREG         0x01     /* Exit is registered          */
#define RXEXIT_ERROR         0x01     /* Exit Ended in Error         */
#define RXEXIT_FAILURE       0x02     /* Exit Ended in Failure       */
#define RXEXIT_BADENTRY      1001     /* Invalid Entry Conditions    */
#define RXEXIT_NOEMEM        1002     /* failure in memory manager   */
#define RXEXIT_BADTYPE       1003     /* Bad registration type.      */
#define RXEXIT_NOTINIT       1004     /* API system not initialized. */
#define RXEXIT_OK             0       /* Function Complete           */
#define RXEXIT_DUP           10       /* Duplicate Exit Name-        */
                                      /* but Registration Completed  */
#define RXEXIT_MAXREG        20       /* Cannot register more        */
                                      /* handlers                    */
#define RXEXIT_NOTREG        30       /* Name Not Registered         */
#define RXEXIT_NOCANDROP     40       /* Name not droppable          */
#define RXEXIT_LOADERR       50       /* Could not load function     */
#define RXEXIT_NOPROC       127       /* RXEXIT routine - not found  */



/* System Exit function and sub-function definitions */

#define RXENDLST    0                 /* End of exit list.           */
#define RXFNC    2                    /* Process external functions. */
#define    RXFNCCAL 1                 /* subcode value.              */
#define RXCMD    3                    /* Process host commands.      */
#define    RXCMDHST 1                 /* subcode value.              */
#define RXMSQ    4                    /* Manipulate queue.           */
#define    RXMSQPLL 1                 /* Pull a line from queue      */
#define    RXMSQPSH 2                 /* Place a line on queue       */
#define    RXMSQSIZ 3                 /* Return num of lines on queue*/
#define    RXMSQNAM 20                /* Set active queue name       */
#define RXSIO    5                    /* Session I/O.                */
#define    RXSIOSAY 1                 /* SAY a line to STDOUT        */
#define    RXSIOTRC 2                 /* Trace output                */
#define    RXSIOTRD 3                 /* Read from char stream       */
#define    RXSIODTR 4                 /* DEBUG read from char stream */
#define    RXSIOTLL 5                 /* Return linelength(N/A OS/2) */
#define RXHLT    7                    /* Halt processing.            */
#define    RXHLTCLR 1                 /* Clear HALT indicator        */
#define    RXHLTTST 2                 /* Test HALT indicator         */
#define RXTRC    8                    /* Test ext trace indicator.   */
#define    RXTRCTST 1                 /* subcode value.              */
#define RXINI    9                    /* Initialization processing.  */
#define    RXINIEXT 1                 /* subcode value.              */
#define RXTER   10                    /* Termination processing.     */
#define    RXTEREXT 1                 /* subcode value.              */

#ifdef INCL_RXOBJECT
#define RXDBG    11                   /* Test ext trace indicator before instruction.   */
#define    RXDBGTST  1                /* subcode value.              */
   /* return codes for the debug exit */
#define    RXDBGOFF  0                /* set trace off */
#define    RXDBGSTEPIN  1             /* set trace on (normal)       */
#define    RXDBGSTEPOVER  2           /* set trace on but step over  */
#define    RXDBGSTEPOUT  3            /* set step out                */
#define    RXDBGENDSTEP 4             /* leaving a subroutine        */
#define    RXDBGENTERSUB 5            /* entering a subroutine       */
#define    RXDBGLEAVESUB 6            /* leaving a subroutine        */
#define    RXDBGLOCATELINE 7          /* locate current line         */
#define    RXDBGSTEPAGAIN 8           /* repeat debug pause          */
#define    RXDBGSIGNAL 9              /* processing a signal instr.  */
#define    RXDBGRECURSIVEOFF 10       /* switch dbg off recursively  */
#define    RXNOOFEXITS 12             /* 1 + largest exit number.    */
#else
#define RXNOOFEXITS 11                /* 1 + largest exit number.    */
#endif /* INCL_RXOBJECT */

typedef PUCHAR PEXIT;                 /* ptr to exit parameter block */
#endif /* INCL_RXSYSEXIT for defines */

/***    Asynchronous Request Interface defines */
#ifdef INCL_RXARI

/***    Return Codes from Asynchronous Request interface */

#define RXARI_OK                   0  /* Interface completed         */
#define RXARI_NOT_FOUND            1  /* Target program not found    */
#define RXARI_PROCESSING_ERROR     2  /* Error processing request    */
#endif /* INCL_RXARI for defines */

/***    Macro Space Interface defines */
#ifdef INCL_RXMACRO

/***    Registration Search Order Flags */

#define RXMACRO_SEARCH_BEFORE       1  /* Beginning of search order  */
#define RXMACRO_SEARCH_AFTER        2  /* End of search order        */


/***    Return Codes from RxMacroSpace interface */

#define RXMACRO_OK                 0  /* Macro interface completed   */
#define RXMACRO_NO_STORAGE         1  /* Not Enough Storage Available*/
#define RXMACRO_NOT_FOUND          2  /* Requested function not found*/
#define RXMACRO_EXTENSION_REQUIRED 3  /* File ext required for save  */
#define RXMACRO_ALREADY_EXISTS     4  /* Macro functions exist       */
#define RXMACRO_FILE_ERROR         5  /* File I/O error in save/load */
#define RXMACRO_SIGNATURE_ERROR    6  /* Incorrect format for load   */
#define RXMACRO_SOURCE_NOT_FOUND   7  /* Requested cannot be found   */
#define RXMACRO_INVALID_POSITION   8  /* Invalid search order pos    */
#define RXMACRO_NOT_INIT           9  /* API not initialized         */
#endif /* INCL_RXMACRO for defines */

/*********************************************************************/
/*                                                                   */
/*                            32-bit                                 */
/*                                                                   */
/*********************************************************************/

/* XLATOFF */
/***    Main Entry Point to the REXX Interpreter */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
APIRET APIENTRY RexxStart (
         LONG ,                        /* Num of args passed to rexx */
         PRXSTRING,                    /* Array of args passed to rex */
         PCSZ,                         /* [d:][path] filename[.ext]  */
         PRXSTRING,                    /* Loc of rexx proc in memory */
         PCSZ,                         /* ASCIIZ initial environment.*/
         LONG ,                        /* type (command,subrtn,funct) */
         PRXSYSEXIT,                   /* SysExit env. names &  codes */
         PSHORT,                       /* Ret code from if numeric   */
         PRXSTRING );                  /* Retvalue from the rexx proc */

typedef APIRET (APIENTRY *PFNREXXSTART)(LONG, PRXSTRING, PCSZ, PRXSTRING,
                                        PCSZ, LONG, PRXSYSEXIT, PSHORT,
                                        PRXSTRING);
#else
APIRET APIENTRY RexxStart (
         LONG,                         /* Num of args passed to rexx */
         PRXSTRING,                    /* Array of args passed to rex */
         PSZ,                          /* [d:][path] filename[.ext]  */
         PRXSTRING,                    /* Loc of rexx proc in memory */
         PSZ,                          /* ASCIIZ initial environment.*/
         LONG ,                        /* type (command,subrtn,funct) */
         PRXSYSEXIT,                   /* SysExit env. names &  codes */
         PSHORT,                       /* Ret code from if numeric   */
         PRXSTRING );                  /* Retvalue from the rexx proc */

typedef APIRET (APIENTRY *PFNREXXSTART)(LONG, PRXSTRING, PSZ, PRXSTRING,
                                        PSZ, LONG, PRXSYSEXIT, PSHORT,
                                        PRXSTRING);
#endif


/***   Uppercase Entry Point Name */
#define REXXSTART   RexxStart


void APIENTRY RexxWaitForTermination(void);
typedef void (APIENTRY *PFNREXXWAITFORTERMINATION)(void);
#define REXXWAITFORTERMINATION RexxWaitForTermination

APIRET APIENTRY RexxDidRexxTerminate(void);
typedef APIRET (APIENTRY *PFNREXXDIDREXXTERMINATE)(void);
#define REXXDIDREXXTERMINATE RexxDidRexxTerminate

BOOL APIENTRY RexxSetProcessMessages(BOOL onoff);
typedef BOOL (APIENTRY *PFNREXXSETPROCESSMESSAGES)(BOOL onoff);
#define REXXSETPROCESSMESSAGES RexxSetProcessMessages


/***    Subcommand Interface */
#ifdef INCL_RXSUBCOM

/* XLATOFF */

/* This typedef simplifies coding of a Subcommand handler.           */
typedef APIRET APIENTRY RexxSubcomHandler(PRXSTRING,
                                PUSHORT,
                                PRXSTRING);

/***   RexxRegisterSubcomDll -- Register a DLL entry point           */
/***   as a Subcommand handler */

#ifdef __cplusplus
APIRET APIENTRY RexxRegisterSubcomDll (
         PCSZ,                         /* Name of subcom handler     */
         PCSZ,                         /* Name of DLL                */
         PCSZ,                         /* Name of procedure in DLL   */
         PUCHAR,                       /* User area                  */
         ULONG  );                     /* Drop authority.            */

typedef APIRET (APIENTRY *PFNREXXREGISTERSUBCOMDLL)(PCSZ, PCSZ, PCSZ,
                                                    PUCHAR, ULONG);
#else
APIRET APIENTRY RexxRegisterSubcomDll (
         PSZ,                          /* Name of subcom handler     */
         PSZ,                          /* Name of DLL                */
         PSZ,                          /* Name of procedure in DLL   */
         PUCHAR,                       /* User area                  */
         ULONG  );                     /* Drop authority.            */

typedef APIRET (APIENTRY *PFNREXXREGISTERSUBCOMDLL)(PSZ, PSZ, PSZ,
                                                    PUCHAR, ULONG);
#endif


/***   Uppercase Entry Point Name */
#define REXXREGISTERSUBCOMDLL  RexxRegisterSubcomDll


/***   RexxRegisterSubcomExe -- Register an EXE entry point          */
/***   as a Subcommand handler */

#ifdef __cplusplus
APIRET APIENTRY RexxRegisterSubcomExe (
         PCSZ,                          /* Name of subcom handler     */
         PFN,                          /* address of handler in EXE  */
         PUCHAR);                      /* User area                  */

typedef APIRET (APIENTRY *PFNREXXREGISTERSUBCOMEXE)(PCSZ, PFN, PUCHAR);

#else
APIRET APIENTRY RexxRegisterSubcomExe (
         PSZ,                          /* Name of subcom handler     */
         PFN,                          /* address of handler in EXE  */
         PUCHAR);                      /* User area                  */


typedef APIRET (APIENTRY *PFNREXXREGISTERSUBCOMEXE)(PSZ, PFN, PUCHAR);
#endif

/***   Uppercase Entry Point Name */
#define REXXREGISTERSUBCOMEXE  RexxRegisterSubcomExe


/***    RexxQuerySubcom - Query an environment for Existance */

#ifdef __cplusplus
APIRET APIENTRY RexxQuerySubcom(
         PCSZ,                         /* Name of the Environment    */
         PCSZ,                         /* DLL Module Name            */
         PUSHORT,                      /* Stor for existance code    */
         PUCHAR );                     /* Stor for user word         */

typedef APIRET (APIENTRY *PFNREXXQUERYSUBCOM)(PCSZ, PCSZ, PUSHORT,
                                              PUCHAR);

#else
APIRET APIENTRY RexxQuerySubcom(
         PSZ,                          /* Name of the Environment    */
         PSZ,                          /* DLL Module Name            */
         PUSHORT,                      /* Stor for existance code    */
         PUCHAR );                     /* Stor for user word         */

typedef APIRET (APIENTRY *PFNREXXQUERYSUBCOM)(PSZ, PSZ, PUSHORT,
                                              PUCHAR);
#endif

/***   Uppercase Entry Point Name */
#define REXXQUERYSUBCOM  RexxQuerySubcom


/***    RexxDeregisterSubcom - Drop registration of a Subcommand     */
/***    environment */

#ifdef __cplusplus
APIRET APIENTRY RexxDeregisterSubcom(
         PCSZ,                         /* Name of the Environment    */
         PCSZ );                       /* DLL Module Name            */

typedef APIRET (APIENTRY *PFNREXXDEREGISTERSUBCOM)(PCSZ, PCSZ);

#else
APIRET APIENTRY RexxDeregisterSubcom(
         PSZ,                          /* Name of the Environment    */
         PSZ );                        /* DLL Module Name            */

typedef APIRET (APIENTRY *PFNREXXDEREGISTERSUBCOM)(PSZ, PSZ);
#endif

/***   Uppercase Entry Point Name */
#define REXXDEREGISTERSUBCOM  RexxDeregisterSubcom


/* XLATON */
#endif /* INCL_RXSUBCOM */


/***    Shared Variable Pool Interface */
#ifdef INCL_RXSHV

/* XLATOFF */

/***    RexxVariablePool - Request Variable Pool Service */

APIRET APIENTRY RexxVariablePool(
         PSHVBLOCK);                  /* Pointer to list of SHVBLOCKs*/

typedef APIRET (APIENTRY *PFNREXXVARIABLEPOOL)(PSHVBLOCK);

/***   Uppercase Entry Point Name */
#define REXXVARIABLEPOOL  RexxVariablePool
/* XLATON */

#endif /* INCL_RXSHV */


/***    External Function Interface */
#ifdef INCL_RXFUNC

/* XLATOFF */

/* This typedef simplifies coding of an External Function.           */

typedef ULONG APIENTRY RexxFunctionHandler(PUCHAR,
                                  ULONG,
                                  PRXSTRING,
                                  PSZ,
                                  PRXSTRING);

/***    RexxRegisterFunctionDll - Register a function in the AFT */

#ifdef __cplusplus
APIRET APIENTRY RexxRegisterFunctionDll (
        PCSZ,                          /* Name of function to add    */
        PCSZ,                          /* Dll file name (if in dll)  */
        PCSZ);                         /* Entry in dll               */

typedef APIRET (APIENTRY *PFNREXXREGISTERFUNCTIONDLL)(PCSZ, PCSZ, PCSZ);

#else
APIRET APIENTRY RexxRegisterFunctionDll (
        PSZ,                           /* Name of function to add    */
        PSZ,                           /* Dll file name (if in dll)  */
        PSZ);                          /* Entry in dll               */

typedef APIRET (APIENTRY *PFNREXXREGISTERFUNCTIONDLL)(PSZ, PSZ, PSZ);
#endif


/***   Uppercase Entry Point Name */
#define REXXREGISTERFUNCTIONDLL  RexxRegisterFunctionDll


/***    RexxRegisterFunctionExe - Register a function in the AFT */

#ifdef __cplusplus
APIRET APIENTRY RexxRegisterFunctionExe (
        PCSZ,                          /* Name of function to add    */
        PFN);                          /* Entry point in EXE         */

typedef APIRET (APIENTRY *PFNREXXREGISTERFUNCTIONEXE)(PCSZ, PFN);

#else
APIRET APIENTRY RexxRegisterFunctionExe (
        PSZ,                           /* Name of function to add    */
        PFN);                          /* Entry point in EXE         */

typedef APIRET (APIENTRY *PFNREXXREGISTERFUNCTIONEXE)(PSZ, PFN);
#endif


/***   Uppercase Entry Point Name */
#define REXXREGISTERFUNCTIONEXE  RexxRegisterFunctionExe


/***    RexxDeregisterFunction - Delete a function from the AFT */

#ifdef __cplusplus
APIRET APIENTRY RexxDeregisterFunction (
        PCSZ );                         /* Name of function to remove */

typedef APIRET (APIENTRY *PFNREXXDEREGISTERFUNCTION)(PCSZ);

#else
APIRET APIENTRY RexxDeregisterFunction (
        PSZ );                         /* Name of function to remove */

typedef APIRET (APIENTRY *PFNREXXDEREGISTERFUNCTION)(PSZ);
#endif

/***   Uppercase Entry Point Name */
#define REXXDEREGISTERFUNCTION  RexxDeregisterFunction


/***    RexxQueryFunction - Scan the AFT for a function */

#ifdef __cplusplus
APIRET APIENTRY RexxQueryFunction (
        PCSZ );                         /* Name of function to find   */

typedef APIRET (APIENTRY *PFNREXXQUERYFUNCTION)(PCSZ);

#else
APIRET APIENTRY RexxQueryFunction (
        PSZ );                         /* Name of function to find   */

typedef APIRET (APIENTRY *PFNREXXQUERYFUNCTION)(PSZ);
#endif

/***   Uppercase Entry Point Name */
#define REXXQUERYFUNCTION  RexxQueryFunction


/* XLATON */


#endif /* INCL_RXFUNC */


/***   System Exits */
#ifdef INCL_RXSYSEXIT

/***    Subfunction RXFNCCAL - External Function Calls */

typedef  struct _RXFNC_FLAGS {          /* fl */
   unsigned rxfferr  : 1;              /* Invalid call to routine.   */
   unsigned rxffnfnd : 1;              /* Function not found.        */
   unsigned rxffsub  : 1;              /* Called as a subroutine     */
}  RXFNC_FLAGS ;

typedef  struct _RXFNCCAL_PARM {        /* fnc */
   RXFNC_FLAGS       rxfnc_flags ;     /* function flags             */
   PUCHAR            rxfnc_name;       /* Pointer to function name.  */
   USHORT            rxfnc_namel;      /* Length of function name.   */
   PUCHAR            rxfnc_que;        /* Current queue name.        */
   USHORT            rxfnc_quel;       /* Length of queue name.      */
   USHORT            rxfnc_argc;       /* Number of args in list.    */
   PRXSTRING         rxfnc_argv;       /* Pointer to argument list.  */
   RXSTRING          rxfnc_retc;       /* Return value.              */
}  RXFNCCAL_PARM;


/***    Subfunction RXCMDHST -- Process Host Commands     */

typedef  struct _RXCMD_FLAGS {          /* fl */
   unsigned rxfcfail : 1;              /* Command failed.            */
   unsigned rxfcerr  : 1;              /* Command ERROR occurred.    */
}  RXCMD_FLAGS;

typedef  struct _RXCMDHST_PARM {        /* rx */
   RXCMD_FLAGS       rxcmd_flags;      /* error/failure flags        */
   PUCHAR            rxcmd_address;    /* Pointer to address name.   */
   USHORT            rxcmd_addressl;   /* Length of address name.    */
   PUCHAR            rxcmd_dll;        /* dll name for command.      */
   USHORT            rxcmd_dll_len;    /* Length of dll name.        */
   RXSTRING          rxcmd_command;    /* The command string.        */
   RXSTRING          rxcmd_retc;       /* Pointer to return buffer   */
}  RXCMDHST_PARM;


/***     Subfunction RXMSQPLL -- Pull Entry from Queue */

typedef struct _RXMSQPLL_PARM {        /* pll */
   RXSTRING          rxmsq_retc;       /* Pointer to dequeued entry  */
                                       /* buffer.  User allocated.   */
} RXMSQPLL_PARM;


/***    Subfunction RXMSQPSH -- Push Entry on Queue */
typedef  struct _RXMSQ_FLAGS {          /* fl */
   unsigned rxfmlifo : 1;              /* Stack entry LIFO if set    */
}  RXMSQ_FLAGS;

typedef  struct _RXMSQPSH_PARM {        /* psh */
   RXMSQ_FLAGS       rxmsq_flags;      /* LIFO/FIFO flag             */
   RXSTRING          rxmsq_value;      /* The entry to be pushed.    */
}  RXMSQPSH_PARM;


/***    Subfunction RXMSQSIZ -- Return the Current Queue Size */

typedef struct _RXMSQSIZ_PARM {        /* siz */
   ULONG             rxmsq_size;       /* Number of Lines in Queue   */
}  RXMSQSIZ_PARM;


/***    Subfunction RXMSQNAM -- Set Current Queue Name */

typedef struct _RXMSQNAM_PARM {        /* nam */
   RXSTRING          rxmsq_name;       /* RXSTRING containing        */
                                       /* queue name.                */
}  RXMSQNAM_PARM;


/***    Subfunction RXSIOSAY -- Perform SAY Clause */

typedef struct _RXSIOSAY_PARM {        /* say */
   RXSTRING          rxsio_string;     /* String to display.         */
}  RXSIOSAY_PARM;


/***    Subfunction RXSIOTRC -- Write Trace Output */

typedef struct _RXSIOTRC_PARM { /* trcparm */
   RXSTRING          rxsio_string;     /* Trace line to display.     */
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

#ifdef INCL_RXOBJECT
typedef struct _RXDBG_FLAGS {          /* fl Trace flags             */
   unsigned rxftrace;                  /* Set to run external trace. */
}  RXDBG_FLAGS;

typedef struct _RXDBGTST_PARM {        /* tst */
   RXDBG_FLAGS rxdbg_flags;            /* Set to run external trace before */
   ULONG       rxdbg_line;
   RXSTRING    rxdbg_filename;
   RXSTRING    rxdbg_routine;
}  RXDBGTST_PARM;
#endif /* INCL_RXOBJECT */

/* XLATOFF */

/* This typedef simplifies coding of an Exit handler.                */
typedef LONG APIENTRY RexxExitHandler(LONG,
                             LONG,
                             PEXIT);

/***      RexxRegisterExitDll - Register a system exit. */

#ifdef __cplusplus
APIRET APIENTRY RexxRegisterExitDll (
         PCSZ,                         /* Name of the exit handler   */
         PCSZ,                         /* Name of the DLL            */
         PCSZ,                         /* Name of the procedure      */
         PUCHAR,                       /* User area                  */
         ULONG );                      /* Drop authority             */

typedef APIRET (APIENTRY *PFNREXXREGISTEREXITDLL)(PCSZ, PCSZ, PCSZ,
                                                  PUCHAR, ULONG);
#else
APIRET APIENTRY RexxRegisterExitDll (
         PSZ,                          /* Name of the exit handler   */
         PSZ,                          /* Name of the DLL            */
         PSZ,                          /* Name of the procedure      */
         PUCHAR,                       /* User area                  */
         ULONG );                      /* Drop authority             */

typedef APIRET (APIENTRY *PFNREXXREGISTEREXITDLL)(PSZ, PSZ, PSZ,
                                                  PUCHAR, ULONG);
#endif


/***   Uppercase Entry Point Name */
#define REXXREGISTEREXITDLL  RexxRegisterExitDll


/***      RexxRegisterExitExe - Register a system exit. */

#ifdef __cplusplus
APIRET APIENTRY RexxRegisterExitExe (
         PCSZ,                         /* Name of the exit handler   */
         PFN,                          /* Address of exit handler    */
         PUCHAR);                      /* User area                  */

typedef APIRET (APIENTRY *PFNREXXREGISTEREXITEXE)(PCSZ, PFN, PUCHAR);

#else
APIRET APIENTRY RexxRegisterExitExe (
         PSZ,                          /* Name of the exit handler   */
         PFN,                          /* Address of exit handler    */
         PUCHAR);                      /* User area                  */

typedef APIRET (APIENTRY *PFNREXXREGISTEREXITEXE)(PSZ, PFN, PUCHAR);
#endif

/***   Uppercase Entry Point Name */
#define REXXREGISTEREXITEXE  RexxRegisterExitExe


/***    RexxDeregisterExit - Drop registration of a system exit. */

#ifdef __cplusplus
APIRET APIENTRY RexxDeregisterExit (
         PCSZ,                          /* Exit name                  */
         PCSZ ) ;                       /* DLL module name            */

typedef APIRET (APIENTRY *PFNREXXDEREGISTEREXIT)(PCSZ, PCSZ);

#else
APIRET APIENTRY RexxDeregisterExit (
         PSZ,                          /* Exit name                  */
         PSZ ) ;                       /* DLL module name            */

typedef APIRET (APIENTRY *PFNREXXDEREGISTEREXIT)(PSZ, PSZ);
#endif

/***   Uppercase Entry Point Name */
#define REXXDEREGISTEREXIT  RexxDeregisterExit


/***    RexxQueryExit - Query an exit for existance. */

#ifdef __cplusplus
APIRET APIENTRY RexxQueryExit (
         PCSZ,                         /* Exit name                  */
         PCSZ,                         /* DLL Module name.           */
         PUSHORT,                      /* Existance flag.            */
         PUCHAR );                     /* User data.                 */

typedef APIRET (APIENTRY *PFNREXXQUERYEXIT)(PCSZ, PCSZ, PUSHORT, PUCHAR);

#else
APIRET APIENTRY RexxQueryExit (
         PSZ,                          /* Exit name                  */
         PSZ,                          /* DLL Module name.           */
         PUSHORT,                      /* Existance flag.            */
         PUCHAR );                     /* User data.                 */

typedef APIRET (APIENTRY *PFNREXXQUERYEXIT)(PSZ, PSZ, PUSHORT, PUCHAR);
#endif

/***   Uppercase Entry Point Name */
#define REXXQUERYEXIT  RexxQueryExit
/* XLATON */

#endif /* INCL_RXSYSEXIT */


/***    Asynchronous Request Interface */
#ifdef INCL_RXARI

/* XLATOFF */

/***    RexxSetHalt - Request Program Halt */

APIRET APIENTRY RexxSetHalt(
         PID,                         /* Process Id                  */
         TID);                        /* Thread Id                   */

typedef APIRET (APIENTRY *PFNREXXSETHALT)(PID, TID);

/***   Uppercase Entry Point Name */
#define REXXSETHALT  RexxSetHalt


/***    RexxSetTrace - Request Program Trace */

APIRET APIENTRY RexxSetTrace(
         PID,                         /* Process Id                  */
         TID);                        /* Thread Id                   */

typedef APIRET (APIENTRY *PFNREXXSETTRACE)(PID, TID);

/***   Uppercase Entry Point Name */
#define REXXSETTRACE  RexxSetTrace


/***    RexxResetTrace - Turn Off Program Trace */

APIRET APIENTRY RexxResetTrace(
         PID,                         /* Process Id                  */
         TID);                        /* Thread Id                   */

typedef APIRET (APIENTRY *PFNREXXRESETTRACE)(PID, TID);

/***   Uppercase Entry Point Name */
#define REXXRESETTRACE  RexxResetTrace

/* XLATON */

#endif /* INCL_RXARI */


/***    Macro Space Interface */
#ifdef INCL_RXMACRO

/* XLATOFF */

/***    RexxAddMacro - Register a function in the Macro Space        */

#ifdef __cplusplus
APIRET APIENTRY RexxAddMacro(
         PCSZ,                        /* Function to add or change   */
         PCSZ,                        /* Name of file to get function*/
         ULONG  );                    /* Flag indicating search pos  */

typedef APIRET (APIENTRY *PFNREXXADDMACRO)(PCSZ, PCSZ, ULONG);

#else
APIRET APIENTRY RexxAddMacro(
         PSZ,                         /* Function to add or change   */
         PSZ,                         /* Name of file to get function*/
         ULONG  );                    /* Flag indicating search pos  */

typedef APIRET (APIENTRY *PFNREXXADDMACRO)(PSZ, PSZ, ULONG);
#endif

/***   Uppercase Entry Point Name */
#define REXXADDMACRO  RexxAddMacro


/***    RexxDropMacro - Remove a function from the Macro Space       */

#ifdef __cplusplus
APIRET APIENTRY RexxDropMacro (
         PCSZ );                        /* Name of function to remove */

typedef APIRET (APIENTRY *PFNREXXDROPMACRO)(PCSZ);

#else
APIRET APIENTRY RexxDropMacro (
         PSZ );                        /* Name of function to remove */

typedef APIRET (APIENTRY *PFNREXXDROPMACRO)(PSZ);
#endif

/***   Uppercase Entry Point Name */
#define REXXDROPMACRO  RexxDropMacro


/***    RexxSaveMacroSpace - Save Macro Space functions to a file    */

#ifdef __cplusplus
APIRET APIENTRY RexxSaveMacroSpace (
         ULONG ,                      /* Argument count (0==save all)*/
         PCSZ *,                      /* List of funct names to save */
         PCSZ);                       /* File to save functions in   */

typedef APIRET (APIENTRY * PFNREXXSAVEMACROSPACE)(ULONG, PCSZ *, PCSZ);

#else
APIRET APIENTRY RexxSaveMacroSpace (
         ULONG ,                      /* Argument count (0==save all)*/
         PSZ *,                       /* List of funct names to save */
         PSZ);                        /* File to save functions in   */

typedef APIRET (APIENTRY * PFNREXXSAVEMACROSPACE)(ULONG, PSZ *, PSZ);
#endif

/***   Uppercase Entry Point Name */
#define REXXSAVEMACROSPACE  RexxSaveMacroSpace


/***    RexxLoadMacroSpace - Load Macro Space functions from a file  */

#ifdef __cplusplus
APIRET APIENTRY RexxLoadMacroSpace (
         ULONG ,                      /* Argument count (0==load all)*/
         PCSZ *,                      /* List of funct names to load */
         PCSZ);                       /* File to load functions from */

typedef APIRET (APIENTRY *PFNREXXLOADMACROSPACE)(ULONG, PCSZ *, PCSZ);

#else
APIRET APIENTRY RexxLoadMacroSpace (
         ULONG ,                      /* Argument count (0==load all)*/
         PSZ *,                       /* List of funct names to load */
         PSZ);                        /* File to load functions from */

typedef APIRET (APIENTRY *PFNREXXLOADMACROSPACE)(ULONG, PSZ *, PSZ);
#endif

/***   Uppercase Entry Point Name */
#define REXXLOADMACROSPACE  RexxLoadMacroSpace


/***    RexxQueryMacro - Find a function's search-order position     */

#ifdef __cplusplus
APIRET APIENTRY RexxQueryMacro (
         PCSZ,                        /* Function to search for      */
         PUSHORT );                   /* Ptr for position flag return*/

typedef APIRET (APIENTRY *PFNREXXQUERYMACRO)(PCSZ, PUSHORT);

#else
APIRET APIENTRY RexxQueryMacro (
         PSZ,                         /* Function to search for      */
         PUSHORT );                   /* Ptr for position flag return*/

typedef APIRET (APIENTRY *PFNREXXQUERYMACRO)(PSZ, PUSHORT);
#endif

/***   Uppercase Entry Point Name */
#define REXXQUERYMACRO  RexxQueryMacro


/***    RexxReorderMacro - Change a function's search-order          */
/***                            position                             */

#ifdef __cplusplus
APIRET APIENTRY RexxReorderMacro(
         PCSZ,                        /* Name of funct change order  */
         ULONG  );                    /* New position for function   */

typedef APIRET (APIENTRY *PFNREXXREORDERMACRO)(PCSZ, ULONG);

#else
APIRET APIENTRY RexxReorderMacro(
         PSZ,                         /* Name of funct change order  */
         ULONG  );                    /* New position for function   */

typedef APIRET (APIENTRY *PFNREXXREORDERMACRO)(PSZ, ULONG);
#endif


/***   Uppercase Entry Point Name */
#define REXXREORDERMACRO  RexxReorderMacro


/***    RexxClearMacroSpace - Remove all functions from a MacroSpace */

APIRET APIENTRY RexxClearMacroSpace(
         VOID );                      /* No Arguments.               */

typedef APIRET (APIENTRY *PFNREXXCLEARMACROSPACE)(VOID);

/***   Uppercase Entry Point Name */
#define REXXCLEARMACROSPACE  RexxClearMacroSpace

/* XLATON */

#endif /* INCL_RXMACRO */

/***    Queing Services */
#ifdef INCL_RXQUEUE

/***    Request flags for External Data Queue access */

#define RXQUEUE_FIFO          0    /* Access queue first-in-first-out */
#define RXQUEUE_LIFO          1    /* Access queue last-in-first-out  */

#define RXQUEUE_NOWAIT        0    /* Wait for data if queue empty    */
#define RXQUEUE_WAIT          1    /* Don't wait on an empty queue    */


/***    Return Codes from RxQueue interface */

#define RXQUEUE_OK            0        /* Successful return           */
#define RXQUEUE_NOTINIT       1000     /* Queues not initialized      */

#define RXQUEUE_STORAGE       1        /* Ret info buf not big enough */
#define RXQUEUE_SIZE          2        /* Data size > 64K-64          */
#define RXQUEUE_DUP           3        /* Attempt-duplicate queue name*/
#define RXQUEUE_NOEMEM     1002        /* failure in memory manager   */
#define RXQUEUE_BADQNAME      5        /* Not a valid queue name      */
#define RXQUEUE_PRIORITY      6        /* Not accessed as LIFO|FIFO   */
#define RXQUEUE_BADWAITFLAG   7        /* Not accessed as WAIT|NOWAIT */
#define RXQUEUE_EMPTY         8        /* No data in queue            */
#define RXQUEUE_NOTREG        9        /* Queue does not exist        */
#define RXQUEUE_ACCESS       10        /* Queue busy and wait active  */
#define RXQUEUE_MAXREG       11        /* No memory to create a queue */
#define RXQUEUE_MEMFAIL      12        /* Failure in memory management*/


/***    RexxCreateQueue - Create an External Data Queue */

#ifdef __cplusplus
ULONG  APIENTRY RexxCreateQueue (
        PCSZ,                          /* Name of queue created       */
        ULONG,                         /* Size of buf for ret name    */
        PCSZ,                          /* Requested name for queue    */
        PULONG ) ;                     /* Duplicate name flag.        */

typedef ULONG (APIENTRY *PFNREXXCREATEQUEUE)(PCSZ, ULONG, PCSZ, PULONG);

#else
ULONG  APIENTRY RexxCreateQueue (
        PSZ,                           /* Name of queue created       */
        ULONG,                         /* Size of buf for ret name    */
        PSZ,                           /* Requested name for queue    */
        PULONG ) ;                     /* Duplicate name flag.        */

typedef ULONG (APIENTRY *PFNREXXCREATEQUEUE)(PSZ, ULONG, PSZ, PULONG);
#endif


/***    RexxDeleteQueue - Delete an External Data Queue */

#ifdef __cplusplus
ULONG  APIENTRY RexxDeleteQueue (
        PCSZ );                         /* Name of queue to be deleted */

typedef ULONG (APIENTRY *PFNREXXDELETEQUEUE)(PCSZ);

#else
ULONG  APIENTRY RexxDeleteQueue (
        PSZ );                         /* Name of queue to be deleted */

typedef ULONG (APIENTRY *PFNREXXDELETEQUEUE)(PSZ);
#endif


/*** RexxQueryQueue - Query an External Data Queue for number of      */
/***                  entries                                         */

#ifdef __cplusplus
ULONG  APIENTRY RexxQueryQueue (
        PCSZ,                          /* Name of queue to query      */
        PULONG );                      /* Place to put element count  */

typedef ULONG (APIENTRY *PFNREXXQUERYQUEUE)(PCSZ, PULONG);

#else
ULONG  APIENTRY RexxQueryQueue (
        PSZ,                           /* Name of queue to query      */
        PULONG );                      /* Place to put element count  */

typedef ULONG (APIENTRY *PFNREXXQUERYQUEUE)(PSZ, PULONG);
#endif


/***    RexxAddQueue - Add an entry to an External Data Queue */

#ifdef __cplusplus
ULONG  APIENTRY RexxAddQueue (
        PCSZ,                          /* Name of queue to add to     */
        PRXSTRING,                     /* Data string to add          */
        ULONG );                       /* Queue type (FIFO|LIFO)      */

typedef ULONG (APIENTRY *PFNREXXADDQUEUE)(PCSZ, PRXSTRING, ULONG);

#else
ULONG  APIENTRY RexxAddQueue (
        PSZ,                           /* Name of queue to add to     */
        PRXSTRING,                     /* Data string to add          */
        ULONG );                       /* Queue type (FIFO|LIFO)      */

typedef ULONG (APIENTRY *PFNREXXADDQUEUE)(PSZ, PRXSTRING, ULONG);
#endif


/***    RexxPullQueue - Retrieve data from an External Data Queue */

#ifdef __cplusplus
ULONG  APIENTRY RexxPullQueue (
        PCSZ,                          /* Name of queue to read from  */
        PRXSTRING,                     /* RXSTRING to receive data    */
        SYSTEMTIME * PDATETIME,        /* Stor for data date/time     */
        ULONG );                       /* wait status (WAIT|NOWAIT)   */

typedef ULONG (APIENTRY *PFNREXXPULLQUEUE)(PCSZ, PRXSTRING, SYSTEMTIME *,
                                           ULONG);

#else
ULONG  APIENTRY RexxPullQueue (
        PSZ,                           /* Name of queue to read from  */
        PRXSTRING,                     /* RXSTRING to receive data    */
        SYSTEMTIME * PDATETIME,        /* Stor for data date/time     */
        ULONG );                       /* wait status (WAIT|NOWAIT)   */

typedef ULONG (APIENTRY *PFNREXXPULLQUEUE)(PSZ, PRXSTRING, SYSTEMTIME *,
                                           ULONG);
#endif


/* XLATOFF */

#endif /* INCL_RXQUEUE */


/* BEGIN Common Prototypes */

/* The following prototypes are always included, because they are    */
/* common to all interfaces                                          */

ULONG  APIENTRY RexxShutDownAPI(void);

typedef ULONG (APIENTRY *PFNREXXSHUTDOWNAPI)(void);

/***   Uppercase Entry Point Name */
#define REXXSHUTDOWNAPI  RexxShutDownAPI


/***   RexxAllocateMemory            */

PVOID APIENTRY RexxAllocateMemory(
                   ULONG );           /* number of bytes to allocate */

typedef PVOID (APIENTRY *PFNREXXALLOCATEMEMORY)(ULONG);

/***   Uppercase Entry Point Name */
#define REXXALLOCATEMEMORY RexxAllocateMemory


/***   RexxFreeMemory                */


ULONG APIENTRY RexxFreeMemory(
                   PVOID);   /* pointer to the memory returned by    */
                             /* RexxAllocateMemory                   */

typedef ULONG (APIENTRY *PFNREXXFREEMEMORY)(PVOID);

/***   Uppercase Entry Point Name */
#define REXXFREEMEMORY RexxFreeMemory

/* END Common Prototypes */

#ifdef __cplusplus
}
#endif


#endif /* REXXSAA_INCLUDED */
