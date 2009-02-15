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
/*  File Name:          CALLREXX2.C                                  */
/*                                                                   */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Description:        Samples of how to invoke the Open Object Rexx*/
/*                      interpreter. It loads the Rexx library at    */
/*                      runtime.                                     */
/*                                                                   */
/*  Entry Points:       main - main entry point                      */
/*                                                                   */
/*  Input:              None                                         */
/*                                                                   */
/*  Output:             returns from the Open Object Rexx programs   */
/*                                                                   */
/*********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <errno.h>

#include "rexx.h"

char   *pcharTemp;

int main(int argc, char **argv)
{
    CONSTRXSTRING arg[4];                   /* argument string for Rexx  */
    RXSTRING rexxretval;                    /* return value from Rexx    */
    RXSTRING instore[2];                    /* in storage parms          */
    char     *pszTemp;
    PFNREXXSTART FuncAddress;
    void    *pLibHandle = NULL;             /* Library handle             */
    RexxReturnCode   rc = 0;                        /* return code from Rexx      */
    short    rexxrc = 0;                    /* return code from function  */
    const char *pszLibraryName = "librexx.so"; /* define the library name    */

    char    val;
    const char *str1 = "Arg number one";                   /* text to swap   */
    const char *str2 = "Arg number two";                   /* text to swap   */
    const char *str3 = "Arg number three";                 /* text to swap   */
    const char *str4 = "Arg number four";                  /* text to swap   */

    const char *sync_tst = "call time 'Reset';" \
                        "object1 = .example~new;" \
                        "object2 = .example~new;" \
                        "object3 = .example~new;" \
                        "a.1 = object1~start('REPEAT', 4 , 'Object 1 running');" \
                        "say a.1~result;" \
                        "say 'The result method waits until the START message has completed:';" \
                        "a.2 = object2~start('REPEAT', 2, 'Object 2 running');" \
                        "a.3 = object3~start('REPEAT', 2, 'Object 3 running');" \
                        "say a.2~result;" \
                        "say a.3~result;" \
                        "say 'main ended';" \
                        "say 'Elapsed time: ' time('E');" \
                        "exit;" \
                        "::REQUIRES 'example.rex'";

    if (!(pLibHandle = dlopen(pszLibraryName, RTLD_LAZY )))
    {                            /* Load and resolve symbols immediately  */
      fprintf(stderr, " *** Unable to load library %s !\nError message: %s\n",
               pszLibraryName, dlerror());
      return 99;
    }

    if(!(FuncAddress = (PFNREXXSTART) dlsym(pLibHandle, "RexxStart")))
    {
      rc = 1;                               /* could not resolve          */
      fprintf(stderr, " *** Unable to load function %s !\nError message: %s\n",
                                            "RexxStart", dlerror());
      return 99;
    }


    /* By setting the strlength of the Rexx output to zero, the           */
    /* interpreter allocates memory.                                      */
    /* We can provide a buffer for the interpreter to use instead.        */
    /* If the returned value does not fit into the buffer,                */
    /* Open Object Rexx creates a new one.                                */

    system("clear");

    rexxretval.strptr = NULL;       /* initialize return-pointer to empty */
    rexxretval.strlength = 0;       /* initialize return-length to zero   */

    printf("This is an easy sample of how to invoke the Rexx interpreter. \n");
    printf("The Rexx commandfile which is started is named: startrx1.rex\n");

    printf("Press Enter to continue\n");
    scanf("%c", &val);


    /* This is the interpreter invocation. ------------------------------ */

    rc =    (*FuncAddress)(
            0,                       /* number of arguments   */
             NULL,                   /* array of arguments    */
             "startrx1.rex",         /* name of Rexx file     */
             NULL,                   /* No INSTORE used       */
             "ksh",                  /* Command env. name     */
             RXCOMMAND,              /* Code for how invoked  */
             NULL,                   /* No EXITs on this call */
             &rexxrc,                /* Rexx program output   */
             &rexxretval );          /* Rexx program output   */

    printf("CALLREXX2 - Back from REXXSTART:  Return Code: %d\n", rc);
    printf("CALLREXX2 - RESULT-LENGTH:           %d\n", rexxretval.strlength);
    printf("CALLREXX2 - RESULT-Value:            %s\n", rexxretval.strptr);

    RexxFreeMemory(rexxretval.strptr);

    printf("Press Enter to continue\n");
    scanf("%c", &val);

    system("clear");

    printf("In this case a previously defined Resultstring is \n");
    printf("delivered to Open Object Rexx, which is large enough to \n");
    printf("hold the Return Value of the Rexx commandfile.\n");

    printf("Press Enter to continue\n");
    scanf("%c", &val);

    rexxretval.strptr = (char *) malloc(100 * sizeof(char));
    rexxretval.strlength = 100;

    rc = (*FuncAddress)(
             0,                      /* number of arguments   */
             NULL,                   /* array of arguments    */
             "startrx1.rex",         /* name of Rexx file     */
             NULL,                   /* No INSTORE used       */
             "ksh",                  /* Command env. name     */
             RXCOMMAND,              /* Code for how invoked  */
             NULL,                   /* No EXITs on this call */
             &rexxrc,                /* Rexx program output   */
             &rexxretval );          /* Rexx program output   */

    printf("CALLREXX2 - Back from REXXSTART:  Return Code: %d\n", rc);
    printf("rexxretval.strptr contains %s\n", rexxretval.strptr);
    printf("CALLREXX2 - RESULT-LENGTH:           %d\n", rexxretval.strlength);
    printf("CALLREXX2 - RESULT-Value:            %s\n", rexxretval.strptr);

    free(rexxretval.strptr);

    printf("Press Enter to continue\n");
    scanf("%c", &val);

    system("clear");

    printf("In this case a previously defined Resultstring is \n");
    printf("delivered to Open Object Rexx, which is too small to\n");
    printf("hold the Return Value of the Rexx commandfile.\n");
    printf("Rexx reallocates the buffer which needs to be freed.\n");
    printf("in the calling program\n");

    printf("Press Enter to continue\n");
    scanf("%c", &val);

    rexxretval.strptr = (char *) malloc(2 * sizeof(char));
    pszTemp = rexxretval.strptr;
    rexxretval.strlength = 2;

    printf("The length of the Resultstring is %d\n", rexxretval.strlength);

    rc = (*FuncAddress)(
             0,                      /* number of arguments   */
             NULL,                   /* array of arguments    */
             "startrx1.rex",         /* name of Rexx file     */
             NULL,                   /* No INSTORE used       */
             "ksh",                  /* Command env. name     */
             RXCOMMAND,              /* Code for how invoked  */
             NULL,                   /* No EXITs on this call */
             &rexxrc,                /* Rexx program output   */
             &rexxretval );          /* Rexx program output   */

    printf("CALLREXX2 - Back from REXXSTART:  Return Code: %d\n", rc);
    printf("The ResultString contains %s after call\n", rexxretval.strptr);
    printf("The length is now %d\n", rexxretval.strlength);

    free(rexxretval.strptr);

    printf("Press Enter to continue\n");
    scanf("%c", &val);

    system("clear");

    rexxretval.strptr = NULL;       /* initialize return-pointer to empty */
    rexxretval.strlength = 0;       /* initialize return-length to zero   */

    printf("This is a sample with 4 arguments delivered to  \n");
    printf("REXXSTART\n");
    printf("The Rexx commandfile which is started is named: startrx2.rex\n");

    printf("Press Enter to continue\n");
    scanf("%c", &val);

    MAKERXSTRING(arg[0], str1, strlen(str1));  /* create input argument 1 */
    MAKERXSTRING(arg[1], str2, strlen(str2));  /* create input argument 2 */
    MAKERXSTRING(arg[2], str3, strlen(str3));  /* create input argument 3 */
    MAKERXSTRING(arg[3], str4, strlen(str4));  /* create input argument 4 */

    rc = (*FuncAddress)(
             4,                      /* number of arguments   */
             arg,                    /* array of arguments    */
             "startrx2.rex",         /* name of Rexx file     */
             NULL,                   /* No INSTORE used       */
             "ksh",                  /* Command env. name     */
             RXCOMMAND,              /* Code for how invoked  */
             NULL,                   /* No EXITs on this call */
             &rexxrc,                /* Rexx program output   */
             &rexxretval );          /* Rexx program output   */

    printf("CALLREXX2 - Back from REXXSTART:  Return Code: %d\n", rc);
    printf("CALLREXX2 - RESULT-LENGTH:           %d\n", rexxretval.strlength);
    printf("CALLREXX2 - RESULT-Value:            %s\n", rexxretval.strptr);

    free(rexxretval.strptr);

    printf("Press Enter to continue\n");
    scanf("%c", &val);

    system("clear");

    printf("This is a sample with 2 arguments delivered to  \n");
    printf("REXXSTART\n");
    printf("The Rexx commandfile which is started is named: startrx2.rex\n");

    printf("Press Enter to continue\n");
    scanf("%c", &val);

    rexxretval.strptr = NULL;       /* initialize return-pointer to empty */
    rexxretval.strlength = 0;       /* initialize return-length to zero   */

    rc = (*FuncAddress)(
             2,                      /* number of arguments   */
             arg,                    /* array of arguments    */
             "startrx2.rex",         /* name of Rexx file     */
             NULL,                   /* No INSTORE used       */
             "ksh",                  /* Command env. name     */
             RXCOMMAND,              /* Code for how invoked  */
             NULL,                   /* No EXITs on this call */
             &rexxrc,                /* Rexx program output   */
             &rexxretval );          /* Rexx program output   */

    printf("CALLREXX2 - Back from REXXSTART:  Return Code: %d\n", rc);
    printf("CALLREXX2 - RESULT-LENGTH:           %d\n", rexxretval.strlength);
    printf("CALLREXX2 - RESULT-Value:            %s\n", rexxretval.strptr);

    free(rexxretval.strptr);

    printf("Press Enter to continue\n");
    scanf("%c", &val);

    system("clear");

    printf("This is a sample where the directory listing of the   \n");
    printf("actual directory is returned by the Rexx program. The \n");
    printf("returned ResultString is displayed\n");
    printf("The Rexx commandfile which is started is named: startrx3.rex\n");

    printf("Press Enter to continue\n");
    scanf("%c", &val);

    rexxretval.strptr = NULL;       /* initialize return-pointer to empty */
    rexxretval.strlength = 0;       /* initialize return-length to zero   */

    rc = (*FuncAddress)(
             0,                      /* number of arguments   */
             NULL,                   /* array of arguments    */
             "startrx3.rex",         /* name of Rexx file     */
             NULL,                   /* No INSTORE used       */
             "ksh",                  /* Command env. name     */
             RXCOMMAND,              /* Code for how invoked  */
             NULL,                   /* No EXITs on this call */
             &rexxrc,                /* Rexx program output   */
             &rexxretval );          /* Rexx program output   */

    printf("CALLREXX2 - Back from REXXSTART:  Return Code: %d\n", rc);
    printf("CALLREXX2 - RESULT-LENGTH:           %d\n", rexxretval.strlength);
    printf("CALLREXX2 - RESULT-Value:            %s\n", rexxretval.strptr);

    free(rexxretval.strptr);

    printf("Press Enter to continue\n");
    scanf("%c", &val);

    system("clear");

    printf("This is a sample where the instore parameter [0] is \n");
    printf("tested. Instore parameter [0] is loaded with \n");
    printf("a small Open Object Rexx script showing the concurrency feature.\n");

    printf("Press Enter to continue\n");
    scanf("%c", &val);

    instore[0].strptr = (const char *)sync_tst;
    instore[0].strlength = strlen(instore[0].strptr);
    instore[1].strptr = NULL;
    instore[1].strlength = 0;

    rc = (*FuncAddress)(
             0,                       /* number of arguments   */
             NULL,                   /* array of arguments    */
             NULL,                   /* no name for Rexx file */
             instore,                /* INSTORE used          */
             "ksh",                  /* Command env. name     */
             RXCOMMAND,              /* Code for how invoked  */
             NULL,                   /* No EXITs on this call */
             &rexxrc,                /* Rexx program output   */
             &rexxretval );          /* Rexx program output   */

    printf("CALLREXX2 - Back from REXXSTART:  Return Code: %d\n", rc);
    printf("CALLREXX2 - RESULT-LENGTH:           %d\n", rexxretval.strlength);
    printf("CALLREXX2 - RESULT-Value:            %s\n", rexxretval.strptr);

    free(rexxretval.strptr);

    printf("Press Enter to continue\n");
    scanf("%c", &val);

    system("clear");

    printf("Now instore[1] is loaded with the content of instore[0]. \n");
    printf("It can be used on subsequent calls. instore[0] is set to NULL \n");

    printf("Press Enter to continue\n");
    scanf("%c", &val);

    instore[0].strptr = NULL;
    instore[0].strlength = 0;

    rc = (*FuncAddress)(
             0,                      /* number of arguments   */
             NULL,                   /* array of arguments    */
             NULL,                   /* no name for Rexx file */
             instore,                /* INSTORE used          */
             "ksh",                  /* Command env. name     */
             RXCOMMAND,              /* Code for how invoked  */
             NULL,                   /* No EXITs on this call */
             &rexxrc,                /* Rexx program output   */
             &rexxretval );          /* Rexx program output   */

    printf("CALLREXX2 - Back from REXXSTART:  Return Code: %d\n", rc);
    printf("CALLREXX2 - RESULT-LENGTH:           %d\n", rexxretval.strlength);
    printf("CALLREXX2 - RESULT-Value:            %s\n", rexxretval.strptr);

    free(rexxretval.strptr);
    free(instore[1].strptr);

    printf("Press Enter to continue\n");
    scanf("%c", &val);

    system("clear");

    printf("This is a sample to show how to use the Rexx MacroSpace facility. \n");
    printf("First of all load_macro.rex is called to load \n");
    printf("the Rexx script macros.rex into Macrospace. The Macrospace- \n");
    printf("name is upload.rex. \n");

    printf("Press Enter to continue\n");
    scanf("%c", &val);

    rexxretval.strptr = NULL;       /* initialize return-pointer to empty */
    rexxretval.strlength = 0;       /* initialize return-length to zero   */

    rc = (*FuncAddress)(
             0,                      /* number of arguments   */
             NULL,                   /* array of arguments    */
             "load_macro.rex", /* name for Rexx macrospacefile*/
             NULL,                   /* INSTORE not used      */
             "ksh",                  /* Command env. name     */
             RXCOMMAND,              /* Code for how invoked  */
             NULL,                   /* No EXITs on this call */
             &rexxrc,                /* Rexx program output   */
             &rexxretval );          /* Rexx program output   */

    printf("CALLREXX2 - Back from REXXSTART:  Return Code: %d\n", rc);
    printf("CALLREXX2 - RESULT-LENGTH:           %d\n", rexxretval.strlength);
    printf("CALLREXX2 - RESULT-Value:            %s\n", rexxretval.strptr);

    free(rexxretval.strptr);

    printf("Press Enter to continue\n");
    scanf("%c", &val);

    system("clear");

    printf("Now the Open Object Rexx script macros.rex (named upload.rex) has been loaded\n");
    printf("into Macrospace. It is now used in the name option of\n");
    printf("the REXXSTART command. \n");
    printf("It is very important that instore paramenter [0] and [1] are\n");
    printf("initialized to NULL rsp. 0 and used as REXXSTART parameters\n");

    printf("Press Enter to continue\n");
    scanf("%c", &val);

    rexxretval.strptr = NULL;       /* initialize return-pointer to empty */
    rexxretval.strlength = 0;       /* initialize return-length to zero   */

    instore[1].strptr = NULL;
    instore[1].strlength = 0;
    instore[0].strptr = NULL;
    instore[0].strlength = 0;

    rc = (*FuncAddress)(
             0,                      /* number of arguments   */
             NULL,                   /* array of arguments    */
             "upload.rex",           /* name for Rexx macrospacefile */
             instore,                /* INSTORE used          */
             "ksh",                  /* Command env. name     */
             RXCOMMAND,              /* Code for how invoked  */
             NULL,                   /* No EXITs on this call */
             &rexxrc,                /* Rexx program output   */
             &rexxretval );          /* Rexx program output   */

    printf("CALLREXX2 - Back from REXXSTART:  Return Code: %d\n", rc);
    printf("CALLREXX2 - RESULT-LENGTH:           %d\n", rexxretval.strlength);
    printf("CALLREXX2 - RESULT-Value:            %s\n", rexxretval.strptr);

    free(rexxretval.strptr);

    printf("Press Enter to continue\n");
    scanf("%c", &val);

    system("clear");

    printf("Finally del_macro.rex is called to delete macros.rex (named upload.rex)\n");
    printf("out of the Open Object Rexx Macrospace.\n");

    printf("Press Enter to continue\n");
    scanf("%c", &val);

    rc = (*FuncAddress)(
             0,                      /* number of arguments   */
             NULL,                   /* array of arguments    */
             "del_macro.rex",        /* name for Rexx macrospacefile */
             NULL,                   /* INSTORE not used      */
             "ksh",                  /* Command env. name     */
             RXCOMMAND,              /* Code for how invoked  */
             NULL,                   /* No EXITs on this call */
             &rexxrc,                /* Rexx program output   */
             &rexxretval );          /* Rexx program output   */

    printf("CALLREXX2 - Back from REXXSTART:  Return Code: %d\n", rc);
    printf("CALLREXX2 - RESULT-LENGTH:           %d\n", rexxretval.strlength);
    printf("CALLREXX2 - RESULT-Value:            %s\n", rexxretval.strptr);

    free(rexxretval.strptr);

    printf("Press Enter to continue\n");
    scanf("%c", &val);

    system("clear");
    return 0;
}

