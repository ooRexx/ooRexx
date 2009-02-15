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
/******************************************************************************/
/* Oryx Kernel                                                  rexx.c        */
/*                                                                            */
/* translate a program and save to an output file                             */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "windows.h"
#include "rexx.h"
#include "PlatformDefinitions.h"
#include "RexxErrorCodes.h"
#include "RexxInternalApis.h"

#define DLLNAME "rexx.dll"

#define BUFFERLEN         256          /* Length of message bufs used       */

// TODO:  Add these to the official API list

void DisplayError(HINSTANCE hDll, int err_no)
{
   char str[BUFFERLEN];
   if (LoadString(hDll, err_no, str, BUFFERLEN))
       printf("\n%s", str);
   else
       printf("\nError in service program but no error message found!");
}


int SysCall main(int argc, char **argv)
{
  char fn[2][BUFFERLEN];
  int  silent = 0;
  int  j = 0;

  HINSTANCE hDll=NULL;

  hDll = LoadLibrary(DLLNAME);

  for (j=1; j<argc; j++)
  {
      if (((argv[j][0] == '/') || (argv[j][0] == '-'))
      && ((argv[j][1] == 's') || (argv[j][1] == 'S'))) silent = j;
  }
  if (!silent)
  {
      char *ptr = RexxGetVersionInformation();
      if (ptr) {
          printf(ptr, "Tokenizer");
          printf("\n");
          GlobalFree(ptr);
      }
  }

  /* check arguments: at least 1 argument, max. 2, /s must be last */
  if ((argc < 2) || (argc > 4) ||           /* invalid no. of args */
      (silent && (argc == 2)) ||            /* only /s             */
      (silent && (silent+1 != argc)) ||     /* /s not last arg     */
      (!silent && (argc == 4)))             /* 3 args, no /s       */
  {
      if (argc > 2) {
      DisplayError(hDll, Error_REXXC_cmd_parm_incorrect);
      }
      DisplayError(hDll, Error_REXXC_wrongNrArg);
      DisplayError(hDll, Error_REXXC_SynCheckInfo);
      if (hDll) FreeLibrary(hDll);
      exit(-1);
  }

  strcpy(fn[0], argv[1]);
  if (argc >= 3) strcpy(fn[1], argv[2]);

  if ( ((argc>3) || ((argc==3) && !silent)) &&
       (strcmp(strupr(fn[0]), strupr(fn[1])) == 0))
  {
      DisplayError(hDll, Error_REXXC_outDifferent);
      if (hDll) FreeLibrary(hDll);
      exit(-2);
  }

  if (hDll) FreeLibrary(hDll);

  if ((argc == 2) || ((argc==3) && silent) )  /* just doing a syntax check? */
                                       /* go perform the translation        */
    return RexxTranslateProgram(argv[1], NULL, NULL);
  else                                 /* translate and save the output     */
    return RexxTranslateProgram(argv[1], argv[2], NULL);
}
