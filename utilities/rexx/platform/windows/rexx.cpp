/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2026 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                                        */
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
/*  File Name:          REXX.CPP                                     */
/*                                                                   */
/*  Description:        Call the REXX interpreter using the command  */
/*                      line arguments.                              */
/*                                                                   */
/*  Entry Points:       main - main entry point                      */
/*                                                                   */
/*********************************************************************/


#include <windows.h>
#include <oorexxapi.h>                          /* needed for rexx stuff      */

#include <iostream>
#include <string>
using namespace std;

#include "ArgumentParser.h"         /* defines getArguments and freeArguments */

//
//  MAIN program
//
int main(int argc, char **argv) {
    short    rexxrc = 0;                 /* return code from rexx             */
    int      argix = argc;               /* argv index of 1st argument (none) */
    int   i;                             /* loop counter                      */
    int  rc = 0;                         /* actually running program RC       */
    const char *program_name = NULL;     /* name to run                       */
    const char *cp;                      /* option character pointer          */
    CONSTRXSTRING arguments;             /* rexxstart argument                */
    size_t argcount;
    bool from_string = false;            /* running from command line string? */
    bool real_argument = true;           /* running from command line string? */
    RXSTRING instore[2];

    RexxInstance        *pgmInst;
    RexxThreadContext   *pgmThrdInst;
    RexxArrayObject      rxargs, rxcargs;
    RexxDirectoryObject  dir;
    RexxObjectPtr        result;

    string optStr;                       // will hold options override string
    wholenumber_t overrideCount = 0;     // +1 one time (switch -o),
                                         // -1 always=global (switch -og)

    /*
     * Convert the input array into a single string for the Object REXX
     * argument string. Initialize the RXSTRING variable to point to this
     * string. Keep the string null terminated so we can print it for debug.
     * First argument is name of the REXX program
     * Next argument(s) are parameters to be passed
    */

    for (i = 1; i < argc; i++) {         /* loop through the arguments        */
        /* is this an option switch?         */
        if ((*(cp=*(argv+i)) == '-' || *cp == '/')) {
            switch (*++cp) {
                case 'e':
                case 'E':                /* execute from string               */
                    if (from_string == false) {  /* only treat 1st -e differently */
                        from_string = true;
                        if ( argc == i+1 ) {
                            break;
                        }
                        program_name = "INSTORE";
                        instore[0].strptr = argv[i+1];
                        instore[0].strlength = strlen(instore[0].strptr);
                        instore[1].strptr = NULL;
                        instore[1].strlength = 0;
                        real_argument = false;
                    }
                    break;
                case 'o':     // override package options, cf. Package' globalOptions documentation
                case 'O': {
                    if (argc == i+1) {    // no options string (o is last arg)
                        break;
                    }
                    overrideCount = 1;  // default to a one time override (for program_name only)
                    char c = *(cp+1);   // get second character
                    if ( c=='d' || c=='D' ) {   // override package default options, unless explicitly defined in package
                        overrideCount = -1;     // override globally
                    }
                    // advance to next: must start with "some option settings"
                    i++;    // position on options string argument  */
                    int k=0;
                    while (argv[i][k++] == ' ');   // skip over leading blanks
                    // NB k is one PAST the first non-blank! (could be after \0)
                    string optPrefix("::OPTIONS ");
                    optStr = argv[i];
                    if (optStr.empty() || optStr.substr(--k, 2) != "::") {
                        optStr = optPrefix + optStr;
                    }
                    break;
                }
                case 'v':
                case 'V': {                                /* version display */
                    char *ptr = RexxGetVersionInformation();
                    if (ptr) {
                        cout << ptr << endl;
                        RexxFreeMemory(ptr);
                    }
                    return 0;
                }
                default:                       /* ignore other switches       */
                    break;
            }
        } else {                         /* convert into an argument string   */
            if (program_name == NULL) {     /* no name yet?                   */
                program_name = argv[i];     /* program is first non-option    */
                argix = i + 1;          /* remember the index of the first arg*/
                break;    /* end parsing after program_name has been resolved */
            } else if ( !(real_argument) )  {  /* not part of the arguments   */
                break;
            }
        }
    }

    if (program_name == NULL) {
        /* give a simple error message       */
        cerr << endl
             << "Syntax is \"rexx [-o[d] \"options\"] filename [arguments]\"\n"
             << "or        \"rexx -e program_string [arguments]\"\n"
             << "or        \"rexx -v\".\n";
        return -1;
    } else {                            /* real program execution             */
        getArguments(NULL, GetCommandLine(), &argcount, &arguments);

        if (from_string) {
            /* Here we call the interpreter.  We don't really need to use     */
            /* all the casts in this call; they just help illustrate          */
            /* the data types used.                                           */
            rc=REXXSTART(argcount,                    /* number of arguments  */
                         &arguments,                  /* array of arguments   */
                         program_name,                /* name of REXX file    */
                         instore,               /* rexx code from command line*/
                         "CMD",                       /* Command env. name    */
                         RXCOMMAND,                   /* Code for how invoked */
                         NULL,
                         &rexxrc,                     /* Rexx program output  */
                         NULL);                       /* REXX program output  */
        } else {
            RexxCreateInterpreter(&pgmInst, &pgmThrdInst, NULL);
            // configure the traditional single argument string

            if ( arguments.strptr != NULL ) {
                rxargs = pgmThrdInst->NewArray(1);
                pgmThrdInst->ArrayPut(rxargs, pgmThrdInst->String(arguments.strptr), 1);
            } else {
                rxargs = pgmThrdInst->NewArray(0);
            }

            // set up the C args into the .local environment
            dir = (RexxDirectoryObject)pgmThrdInst->GetLocalEnvironment();
            if ( argc > argix ) {
                rxcargs = pgmThrdInst->NewArray(argc - argix);
            } else {
                rxcargs = pgmThrdInst->NewArray(0);
            }
            for (int j = argix, i = 1; j < argc; j++, i++)
            {
                pgmThrdInst->ArrayPut(rxcargs,
                                      pgmThrdInst->NewStringFromAsciiz(argv[j]),
                                      i);
            }
            pgmThrdInst->DirectoryPut(dir, rxcargs, "SYSCARGS");

            // call the interpreter
            if (overrideCount != 0) {   // set override global package options
                // if something goes wrong a condition will be set
                RexxObjectPtr clzPackage = pgmThrdInst->FindClass("PACKAGE");
                // set override options
                pgmThrdInst->SendMessage2(clzPackage,
                                          "DEFAULTOPTIONS",
                                          // DefineDefaultOptions
                                          pgmThrdInst->CString("D"),
                                          pgmThrdInst->NewStringFromAsciiz(
                                              optStr.c_str()));
                pgmThrdInst->SendMessage2(clzPackage,
                                          "DEFAULTOPTIONS",
                                          // CountOverrides
                                          pgmThrdInst->CString("C"),
                                          pgmThrdInst->Int64ToObject(
                                              overrideCount));
            }

            if (!pgmThrdInst->CheckCondition()) {   // condition raised? if no,
                // call the interpreter
                result = pgmThrdInst->CallProgram(program_name, rxargs);
            }
            // display any error message if there is a condition.
            // if there was an error, then that will be our return code.
            // Although the return is a wholenumber_t we know there is no error
            // code too big to fit in an int.
            rc = (int)pgmThrdInst->DisplayCondition();
            if (rc != 0) {
                pgmInst->Terminate();
                return -rc;   // well, the negation of the error number is the return code
            }
            // now handle any potential return value
            if (result != NULL) {
                pgmThrdInst->ObjectToInt32(result, &rc);
            }

            pgmInst->Terminate();

            return rc;
        }
    }
    return rc ? rc : rexxrc;                    // rexx program return cd
}
