/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                                         */
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
/*  File Name:          CALLRXNT.C                                   */
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

#include <rexx.h>                      /* needed for RexxStart()     */
#include <stdio.h>                     /* needed for printf()        */
#include <string.h>                    /* needed for strlen()        */

int main()
{
    CONSTRXSTRING arg;                  /* argument string for REXX  */
    RXSTRING rexxretval;                /* return value from REXX    */

    char     *str = "These words will be swapped"; /* text to swap   */

    RexxReturnCode   rc;                        /* return code from REXX     */
    short    rexxrc = 0;                /* return code from function */

    printf("\nThis program will call the REXX interpreter ");
    printf("to reverse the order of the\n");
    printf("\twords in a string.  ");
    printf("The interpreter is invoked with an initial\n");
    printf("\tenvironment name of 'FNC' ");
    printf("and a file name of 'BACKWARD.FNC'\n\n");

    /* By setting the strlength of the output RXSTRING to zero, we   */
    /* force the interpreter to allocate memory and return it to us. */
    /* We could provide a buffer for the interpreter to use instead. */
    rexxretval.strlength = 0L;          /* initialize return to empty*/

    MAKERXSTRING(arg, str, strlen(str));/* create input argument     */

    /* Here we call the interpreter.                                 */
    rc=RexxStart(1,                         /* number of arguments   */
                 &arg,                      /* array of arguments    */
                 "BACKWARD.FNC",            /* name of REXX file     */
                 0,                         /* No INSTORE used       */
                 "FNC",                     /* Command env. name     */
                 RXSUBROUTINE,              /* Code for how invoked  */
                 0,                         /* No EXITs on this call */
                 &rexxrc,                   /* Rexx program output   */
                 &rexxretval );             /* Rexx program output   */

    printf("Interpreter Return Code: %d\n", rc);
    printf("Function Return Code:    %d\n", (int) rexxrc);
    printf("Original String:         '%s'\n", arg.strptr);
    printf("Backwards String:        '%s'\n", rexxretval.strptr);

    RexxFreeMemory(rexxretval.strptr);      /* Release storage       */
                                            /* given to us by REXX.  */
    system("PAUSE");
    return 0;
}
