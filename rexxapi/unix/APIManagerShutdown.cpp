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
/* REXX AIX Support                                            aixmain.c     */
/*                                                                           */
/* Main interface to delete the REXX interpreter''s IPCs                     */
/*                                                                           */
/*****************************************************************************/

#include <stdlib.h>
#include <stdio.h>

#define INCL_RXFUNC
#include "rexx.h"

extern char achRexxHomeDir[];

int main(
          int argc,
          char **argv[] )
{
  int  iRC = 0;
  char * pcharHome;                 /* Pointer to environment var HOME          */
  char * pchArg;
  char charCntl = 'N';

  if (!(pcharHome = getenv("RXHOME")))     /* Get pointer to home var          */
  {
    if (!(pcharHome = getenv("HOME")))     /* Get pointer to home var          */
    {
      fprintf(stderr," *E* ERROR: No RXHOME or HOME for Open Object Rexx!\n\n");
      return(-1);                            /* all done ERROR end            */
    }
  }
  if ( argc ==  2 )
  {
    pchArg =  (char *)argv[1];
    if ( *pchArg == '-' && (*(pchArg+1) == 'f' || *(pchArg+1) == 'F') )
    {
      iRC = (int)RexxShutDownAPI();
      return(iRC);
    }
    else
    if ( *pchArg == '-' && (*(pchArg+1) == 'h' || *(pchArg+1) == 'H') )
    {

      printf("\n ======================================================================\n");
      printf(" Open Object Rexx on the UNIX platforms uses shared memory: \n");
      printf("   o  To keep the registration of functions from a library \n");
      printf("   o  To store data of the REXX queues \n");
      printf("   o  To store the macro space \n");
      printf("   o  For internal control purposes \n\n");
      printf(" In rare cases, when a  \"kill -9  pid\" command is used to instantly \n");
      printf(" stop a Rexx procedure, the shared memory can get damaged.  Open Object\n");
      printf(" Rexx tries to get the shared memory in order again, but if it fails,\n");
      printf(" you need to use the \"rxdelipc\" procedure to delete the shared memory.\n\n");
      printf(" This procedure can also be used to free memory from stored queue data\n");
      printf(" that never gets fetched.\n\n");
      printf(" The command \"rxdelipc -f\" forces the deletion only if no other\n");
      printf(" Open Object Rexx procedure holds that shared memory.\n\n");
      printf(" The default file name of the shared memory anchor point is the entry\n");
      printf(" \"..OOREXXV#R#M#F#$USERID\" in the $HOME directory. If this directory\n");
      printf(" cannot be used, the /tmp/ directory is used.\n");
      printf(" ------------------------------------------------------------------------\n");
      printf(" The environment variable $RXHOME can be set to use a different directory\n");
      printf(" and anchor point file name. The $RXHOME variable must be \"exported\"\n");
      printf(" with the fully qualified path and file name. If the $RXHOME variable\n");
      printf(" is set, the \"rxdelipc\" procedure also uses this definition to delete\n");
      printf(" the shared memory of Open Object Rexx.\n");
      printf(" ========================================================================\n\n");
      return(0);
    }
    else
    {
      printf("\n  *E*  No approproate option found.\n\n");
      return(1);
    }
  }
  if ( argc > 2 )
  {
    printf("\n  *E*  More than one argument defined.\n\n");
    return(1);
  }
  else
  {
     printf("\n Usage:  rxdelipc   or   rxdelipc -f   will force the deletion\n");
     printf("                    or   rxdelipc -h   for help.\n");
     printf("\n ARE YOU SURE YOU WANT TO DELETE THE SHARED MEMORY OF YOUR REXX\n");
     printf("              PROCEDURE AND REXX IS NOT RUNNING?\n");
     printf("\n      Enter > y < for YES or press the <ENTER> key for NO!\n\n");

     charCntl = getchar();
     if ( (charCntl != 'y') && (charCntl != 'Y') )
     {
        printf(" *I* Nothing has been deleted!\n\n");
        return(0);                                      /* END here */
     }

     iRC = (int)RexxShutDownAPI();
  }

  if ( iRC )
  {
    printf("\n  *I*  NO shared memory connected to:\n");
    printf("      \"%s\"  could be deleted.\n\n", achRexxHomeDir );
  }
  else
  {

    printf("\n  *I*  The shared memory of REXX connected to:\n");
    printf("      \"%s\"  has been deleted.\n\n", achRexxHomeDir );
  }
  return(iRC);
}

