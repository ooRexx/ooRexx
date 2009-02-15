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
/*********************************************************************/
/*                                                                   */
/*  File Name:          REXXEXIT.C                                   */
/*                                                                   */
/*  Description:        Provides a sample call to the REXX           */
/*                      interpreter, passing in an environment name, */
/*                      a file name, and a single argument string.   */
/*                                                                   */
/*  Entry Points:       main - main entry point                      */
/*                                                                   */
/*  Input:              None                                         */
/*                                                                   */
/*  Output:             returns 0 in all cases.                      */
/*                                                                   */
/*********************************************************************/
#include <windows.h>
#include <rexx.h>                           /* needed for RexxStart()     */
#include <stdio.h>                          /* needed for printf()        */
#include <string.h>                         /* needed for strlen()        */

//
//  Prototypes
//
int __cdecl main(int argc, char *argv[]);  /* main entry point           */
RexxReturnCode REXXENTRY MY_IOEXIT( int ExitNumber, int Subfunction, PEXIT ParmBlock);

//
//  MAIN program
//
int __cdecl main(int argc, char *argv[])
{
    RXSYSEXIT exit_list[9];              /* Exit list array                   */
    short    rexxrc = 0;                 /* return code from rexx             */
    int   rc;                            /* actually running program RC       */
    CONSTRXSTRING argument;              /* rexxstart argument                */
    RXSTRING rxretbuf;                   // program return buffer

    rc = 0;                              /* set default return                */

    /* just one argument is accepted by this program */
    if ((argc < 2) || (argc > 3))
    {
        printf("Wrong arguments: REXXEXIT program [argument]\n");
        exit(-1);
    }

    /*
     * Convert the input array into a single string for the Object REXX
     * argument string. Initialize the RXSTRING variable to point to this
     * string. Keep the string null terminated so we can print it for debug.
     * First argument is name of the REXX program
     * Next argument(s) are parameters to be passed
    */

    /* By setting the strlength of the output RXSTRING to zero, we   */
    /* force the interpreter to allocate memory and return it to us. */
    /* We could provide a buffer for the interpreter to use instead. */
    rxretbuf.strlength = 0L;          /* initialize return to empty*/

    if (argc == 3)
    {
        MAKERXSTRING(argument, argv[2], strlen(argv[2]));/* create input argument     */
    }
    else
        MAKERXSTRING(argument, "", 0);/* create blank argument     */

    // register IO exit
    rc = RexxRegisterExitExe("MY_IOC", (REXXPFN)&MY_IOEXIT, NULL);

    /* run this via RexxStart            */
    exit_list[0].sysexit_name = "MY_IOC";
    exit_list[0].sysexit_code = RXSIO;
    exit_list[1].sysexit_code = RXENDLST;

    /* Here we call the interpreter.                                  */
    rc=REXXSTART(1,              /* number of arguments   */
                 &argument,      /* array of arguments    */
                 argv[1],        /* name of REXX file     */
                 NULL,           /* No INSTORE used       */
                 "CMD",          /* Command env. name     */
                 RXCOMMAND,      /* Code for how invoked  */
                 exit_list,      /* exits for this call   */
                 &rexxrc,        /* Rexx program output   */
                 &rxretbuf );    /* Rexx program output   */

    /* free memory allocated for the return result */
    if (rc==0)
    {
        RexxFreeMemory(rxretbuf.strptr);
    }
    RexxDeregisterExit("MY_IOC",NULL);     // remove the exit in exe exit list
    // return interpeter or
    return rc ? rc : rexxrc;                    // rexx program return cd
}


RexxReturnCode REXXENTRY MY_IOEXIT(int ExitNumber, int Subfunction, PEXIT parmblock)
{
    RXSIOSAY_PARM *sparm;
    RXSIOTRC_PARM *tparm;
    RXSIOTRD_PARM *rparm;
    RXSIODTR_PARM *dparm;

    switch (Subfunction)
    {
        case RXSIOSAY:
            sparm = ( RXSIOSAY_PARM * )parmblock ;
            printf("%s\n",sparm->rxsio_string.strptr);
            break;
        case RXSIOTRC:
            tparm = ( RXSIOTRC_PARM * )parmblock ;
            printf("%s\n",tparm->rxsio_string.strptr);
            break;
        case RXSIOTRD:
            rparm = (RXSIOTRD_PARM * )parmblock ;
            gets(rparm->rxsiotrd_retc.strptr);
            rparm->rxsiotrd_retc.strlength=strlen(rparm->rxsiotrd_retc.strptr);
            break;
        case RXSIODTR:
            dparm = (RXSIODTR_PARM * )parmblock ;
            gets(dparm->rxsiodtr_retc.strptr);
            dparm->rxsiodtr_retc.strlength=strlen(dparm->rxsiodtr_retc.strptr);
            break;
        default:
            break;
    }

    return RXEXIT_HANDLED;
}


