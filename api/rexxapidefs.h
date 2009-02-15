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

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Module Name: rexxapidefs.h                                                 */
/*                                                                            */
/* ooRexx Common Definitions File                                             */
/*                                                                            */
/*----------------------------------------------------------------------------*/

#ifndef REXXAPIDEFS_INCLUDED
#define REXXAPIDEFS_INCLUDED

/*----------------------------------------------------------------------------*/
/*                                                                            */
/*                                  Common                                    */
/*                                                                            */
/*----------------------------------------------------------------------------*/

/* This section defines return codes and constants for REXX calls    */

#define RXAUTOBUFLEN         256

#define RXAPI_OK 0
#define RXAPI_MEMFAIL 1002

/*** Call type codes for use on interpreter startup                  */
#define RXCOMMAND       0              /* Program called as Command  */
#define RXSUBROUTINE    1              /* Program called as Subroutin*/
#define RXFUNCTION      2              /* Program called as Function */
#define RXMETHOD        3              /* Program called as Method   */
#define RXSCRIPT        4              /* Program called as Script   */

/***    Drop Authority for RXSUBCOM interface */

#define RXSUBCOM_DROPPABLE   0x00     /* handler to be dropped by all*/
#define RXSUBCOM_NONDROP     0x01     /* process with same PID as the*/
                                      /* registrant may drop environ */

/***    Return Codes from RXSUBCOM interface */

#define RXSUBCOM_ISREG       0x01     /* Subcommand is registered    */
#define RXSUBCOM_ERROR       0x01     /* Subcommand Ended in Error   */
#define RXSUBCOM_FAILURE     0x02     /* Subcommand Ended in Failure */
#define RXSUBCOM_BADENTRY    1001     /* Invalid Entry Conditions    */
#define RXSUBCOM_NOEMEM      RXAPI_MEMFAIL  /* failure in memory manager   */
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

/***    Function Codes for Variable Pool Interface (shvcode) */

#define RXSHV_SET          0x00       /* Set var from given value    */
#define RXSHV_FETCH        0x01       /* Copy value of var to buffer */
#define RXSHV_DROPV        0x02       /* Drop variable               */
#define RXSHV_SYSET        0x03       /* Symbolic name Set variable  */
#define RXSHV_SYFET        0x04       /* Symbolic name Fetch variable*/
#define RXSHV_SYDRO        0x05       /* Symbolic name Drop variable */
#define RXSHV_NEXTV        0x06       /* Fetch "next" variable       */
#define RXSHV_PRIV         0x07       /* Fetch private information   */

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
#define RXFUNC_NOEMEM      RXAPI_MEMFAIL  /* failure in memory manager  */

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
#define RXEXIT_NOEMEM        RXAPI_MEMFAIL  /* failure in memory manager   */
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

#define RXEXF    12                   /* scripting function call     */
#define    RXEXFCAL 1                 /* subcode value.              */
#define RXNOVAL   13                  /* NOVALUE exit                */
#define    RXNOVALCALL 1
#define RXVALUE   14                  /* VALUE function exit         */
#define    RXVALUECALL 1
#define RXOFNC   15                   /* Process external functions using object values. */
#define    RXOFNCCAL 1                /* subcode value.              */


#define    RXNOOFEXITS 16             /* 1 + largest exit number.    */

/***    Asynchronous Request Interface defines */

/***    Return Codes from Asynchronous Request interface */

#define RXARI_OK                   0  /* Interface completed         */
#define RXARI_NOT_FOUND            1  /* Target program not found    */
#define RXARI_PROCESSING_ERROR     2  /* Error processing request    */

/***    Macro Space Interface defines */

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
#define RXQUEUE_NOEMEM     RXAPI_MEMFAIL        /* failure in memory manager   */
#define RXQUEUE_BADQNAME      5        /* Not a valid queue name      */
#define RXQUEUE_PRIORITY      6        /* Not accessed as LIFO|FIFO   */
#define RXQUEUE_BADWAITFLAG   7        /* Not accessed as WAIT|NOWAIT */
#define RXQUEUE_EMPTY         8        /* No data in queue            */
#define RXQUEUE_NOTREG        9        /* Queue does not exist        */
#define RXQUEUE_ACCESS       10        /* Queue busy and wait active  */
#define RXQUEUE_MAXREG       11        /* No memory to create a queue */
#define RXQUEUE_MEMFAIL    RXAPI_MEMFAIL  /* Failure in memory management*/

#endif /* REXXAPIDEFS_INCLUDED */

