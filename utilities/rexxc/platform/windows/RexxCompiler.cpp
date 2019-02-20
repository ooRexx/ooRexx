/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2019 Rexx Language Association. All rights reserved.    */
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

void DisplayError(int msgid)
{
    // retrieve the message from the central catalog
    const char *message = RexxGetErrorMessage(msgid);

    printf("%s\n", message);    /* print the message                 */
}

int SysCall main(int argc, char **argv)
{
  int  silent = 0;
  int  j = 0;

  for (j = 1; j < argc; j++)
  {
      if (((argv[j][0] == '/') || (argv[j][0] == '-'))
      && ((argv[j][1] == 's') || (argv[j][1] == 'S'))) silent = j;
  }
  if (!silent)
  {
      char *ptr = RexxGetVersionInformation();
      printf("%s\n", ptr);
      RexxFreeMemory(ptr);
  }

  /* check arguments: at least 1 argument, max. 2, /s must be last */
  if ((argc < 2) || (argc > 4) ||           /* invalid no. of args */
      (silent && (argc == 2)) ||            /* only /s             */
      (silent && (silent+1 != argc)) ||     /* /s not last arg     */
      (!silent && (argc == 4)))             /* 3 args, no /s       */
  {
      if (argc > 2)
      {
         DisplayError(Error_REXXC_cmd_parm_incorrect);
      }
      DisplayError(Error_REXXC_wrongNrArg);
      DisplayError(Error_REXXC_SynCheckInfo);
      exit(-1);
  }

  if ( ((argc>3) || ((argc==3) && !silent)) &&
       (stricmp(argv[1], argv[2]) == 0))
  {
      DisplayError(Error_REXXC_outDifferent);
      exit(-2);
  }

  if ((argc == 2) || ((argc==3) && silent) )
  {
                                       /* go perform the translation        */
      return RexxTranslateProgram(argv[1], NULL, NULL);
  }
  else                                 /* translate and save the output     */
  {
      return RexxTranslateProgram(argv[1], argv[2], NULL);
  }
}
