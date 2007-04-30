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
/*********************************************************************/
/*                                                                   */
/*     Program Name:   AIXRXCMD.C                                    */
/*                                                                   */
/* Descriptive Name:   AIX/Linux Command Line Program for Subcommand */
/*                     Interface.                                    */
/*                                                                   */
/*********************************************************************/
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#define INCL_RXSUBCOM
#define INCL_RXSYSEXIT

#if defined( HAVE_NL_TYPES_H )
# include <nl_types.h>
#endif

#include <limits.h>
#include <string.h>                    /* Include string functions   */
#include <stdio.h>

#include "rexx.h"                      /* for REXXSAA functionality  */
#include "SubcommandAPI.h"             /* get RexxLoadSubcom calls   */
#include "RexxAPIManager.h"
#include "PlatformDefinitions.h"       /* error constants            */
                                       /* old msg constants replaced!*/
#include "RexxErrorCodes.h"
#include "RexxMessageNumbers.h"

#define BUFFERLEN         256          /* Length of message bufs used*/
                                       /* Macro for argv[1] compares */
#define CASE(x) if(!rxstricmp(x,argv[1]))

#define CCHMAXPATH PATH_MAX+1
#ifdef LINUX                   /*  AIX already defined               */
#define SECOND_PARAMETER 1             /* different sign. Lin-AIX    */
#define CATD_ERR -1
#else
#define SECOND_PARAMETER 0             /* 0 for no  NL_CAT_LOCALE    */
#endif

void parmerr(ULONG);

int main(                              /* Program Begins             */
int     argc,                          /* Argument count             */
char    **argv,                        /* Ptr to array of arguments  */
char    **envp )                       /* Ptr to array of env strings*/
{                                      /*                            */
   ULONG        userdata[2] = {0,0};   /* Used during registeration  */
   USHORT       i;                     /* General var for dummy args */
   PSZ          scbname;               /* registration name          */
   PSZ          scbdll_name;           /* DLL file name              */
   PSZ          scbdll_proc;           /* DLL procedure name         */

                                       /* Must be at lease 1 argument*/
   if(argc<2)parmerr(Error_RXSUBC_general_msg);
   CASE("REGISTER"){                   /* Registration check         */
     if(argc<5)parmerr(Error_RXSUBC_register_msg);/* requires 4 parameters */
     scbname=argv[2];                  /* Should be Environment Name */
     scbdll_name=argv[3];              /* Should be Dll Name         */
     scbdll_proc=argv[4];              /* Should be Function Name    */
                                       /* Go register the environment*/
     return(RexxRegisterSubcomDll(scbname,
                                  scbdll_name,
                                  scbdll_proc,
                                  0,
                                  RXSUBCOM_DROPPABLE));
     }                                 /*                            */
   CASE("QUERY"){                      /* Query Check                */
     if(argc<3)parmerr(Error_RXSUBC_query_msg);   /* requires 3 parameters */
     if(argc<4)argv[3]="";             /* if only 3 passed, dummy 4  */
     return(                           /* Call query and return      */
        RexxQuerySubcom(               /* code.                      */
                 argv[2],              /* Should be Dll Name         */
                 argv[3],              /* Should be Function Name    */
                 &i,                   /* Ptr to storage for existnce*/
                 (PUCHAR)userdata      /* Ptr to storage for userdata*/
                ));                    /*                            */
     }                                 /*                            */
   CASE("DROP"){                       /* Drop Check                 */
     if(argc<3)parmerr(Error_RXSUBC_drop_msg);/* Must pass at least 3 args */
     if(argc<4)argv[3]="";             /* if only 3 passed, dummy 4  */
     return(                           /* Call Drop and return       */
        RexxDeregisterSubcom(          /* code                       */
                 argv[2],              /* Should be Dll Name         */
                 argv[3]));            /* Should be Function Name    */
                                       /*                            */
     }                                 /*                            */
   CASE("LOAD"){                       /* Load Check                 */
     if(argc<3)parmerr(Error_RXSUBC_load_msg);/* Must pass at least 3 args */
     if(argc<4)argv[3]="";             /* if only 3 passed, dummy 4  */
     return(                           /* Call the load function     */
        RexxLoadSubcom(                /* and return its return code */
                 argv[2],              /* Should be Dll Name         */
                 argv[3]));            /* Should be Function Name    */
                                       /*                            */
     }
   parmerr(Error_RXSUBC_general_msg);  /* Otherwise, must be an error*/
}


void parmerr( ULONG msgid )            /* removed useless code       */
{
#if defined( HAVE_NL_TYPES_H )
 nl_catd        catd;                  /* catalog descriptor from catopen() */
#endif
 int            set_num = 1;           /* message set 1 from catalog */
 PSZ            message;               /* message pointer            */
 CHAR           DataArea[BUFFERLEN];   /* buf to return message      */

#if defined( HAVE_CATOPEN )
                                       /* open message catalog in NLSPATH   */
    if ((catd = catopen(REXXMESSAGEFILE, SECOND_PARAMETER)) == (nl_catd)CATD_ERR)
    {
       sprintf(DataArea, "%s/%s", ORX_CATDIR, REXXMESSAGEFILE);
       if ((catd = catopen(DataArea, SECOND_PARAMETER)) == (nl_catd)CATD_ERR)
       {
         fprintf(stderr, "\nCannot open REXX message catalog %s.\nNot in NLSPATH or %s.\n",
                         REXXMESSAGEFILE, ORX_CATDIR);
       }
    }                                    /* retrieve message from repository  */
    if (catd != (nl_catd)CATD_ERR)
    {
       message = catgets(catd, set_num, msgid, NULL);

       if(!message)                      /* got a message ?                   */
# if defined(OPSYS_LINUX) && !defined(OPSYS_SUN)
       {
         sprintf(DataArea, "%s/%s", ORX_CATDIR, REXXMESSAGEFILE);
         if ((catd = catopen(DataArea, SECOND_PARAMETER)) == (nl_catd)CATD_ERR)
         {
           printf("\nCannot open REXX message catalog %s.\nNot in NLSPATH or %s.\n",
                  REXXMESSAGEFILE, ORX_CATDIR);
         }
         else
         {
           message = catgets(catd, set_num, msgid, NULL);
           if(!message)                  /* got a message ?                   */
             printf("\n Error message not found!\n");
           else
             printf("\n%s\n", message);  /* print the message                 */
         }
       }
# else
         printf("\n Error message not found!\n");
       else
         printf("\n%s\n", message);      /* print the message                 */
# endif
       catclose(catd);                   /* close the catalog                 */
    }
#else
  printf("\n Cannot get description for error %d!\n",msgid);
#endif

  exit(-1);
}
