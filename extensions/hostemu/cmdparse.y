%{
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2009-2010 Rexx Language Association. All rights reserved.    */
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
/* Authors;                                                                   */
/*       W. David Ashley <dashley@us.ibm.com>                                 */
/*                                                                            */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* !!!CAUTION!!!                                                              */
/* Do not edit the cmdparse.cpp file! This file is produced by yacc! You      */
/* should edit the cmdparse.y file and regen the changes via make!            */
/*----------------------------------------------------------------------------*/


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <rexx.h>

#include "hostemu.h"

/*--------------------------------------------------------------------*/
/*  Local function prototypes                                         */
/*--------------------------------------------------------------------*/

int yylex (
   void);                        /* no arguments                      */
void yyerror (
   char * token);                /* token string                      */
int kwsearch (
   char * token);                /* token string                      */
bool isnumeric (
   char * token);                /* token string                      */


%}
%union
   {
   int    numval;
   char * strval;
   }
%token <strval> EXECIO
%token <strval> HI
%token <strval> TE
%token <strval> TS
%token <strval> CONSTANT
%token <strval> DISKW
%token <strval> DISKR
%token <strval> STEM
%token <strval> FINIS
%token <strval> LIFO
%token <strval> FIFO
%token <strval> SKIP

%start stmt

%%

stmt : EXECIO CONSTANT disk_clause
      {
      lStmtType = EXECIO_STMT;
      ExecIO_Options.lStartRcd = 1;
      /* if constant is not a numeric string we have an error */
      if (isnumeric ($2))
         ExecIO_Options.lRcdCnt = atoi ($2);
      else
         return 1;
      }
   | EXECIO '*' disk_clause
      {
      lStmtType = EXECIO_STMT;
      ExecIO_Options.lStartRcd = 1;
      ExecIO_Options.lRcdCnt = -1;
      }
   | HI
      {
      lStmtType = HI_STMT;
      }
   | TE
      {
      lStmtType = TE_STMT;
      }
   | TS
      {
      lStmtType = TS_STMT;
      }
   ;

disk_clause: DISKW CONSTANT diskw_option
      {
      ExecIO_Options.fRW = true;
      strcpy (ExecIO_Options.aFilename, $2);
      }
   | DISKR CONSTANT diskr_option
      {
      ExecIO_Options.fRW = false;
      strcpy (ExecIO_Options.aFilename, $2);
      }
   | DISKR CONSTANT CONSTANT diskr_option
      {
      ExecIO_Options.fRW = false;
      strcpy (ExecIO_Options.aFilename, $2);
      /* if constant is not a numeric string we have an error */
      if (isnumeric ($3))
         ExecIO_Options.lStartRcd = atoi ($3);
      else
         return 1;
      }
   ;

diskr_option : '(' diskr_options
   |
   ;

diskr_options : FINIS
      {
      ExecIO_Options.fFinis = true;
      }                                  
   |  FINIS FIFO
      {
      ExecIO_Options.fFinis = false;
      }
   |  FINIS LIFO
      {
      ExecIO_Options.fFinis = true;
      ExecIO_Options.lDirection = 1;
      }
   |  FINIS SKIP
      {
      ExecIO_Options.fFinis = true;
      ExecIO_Options.lDirection = 2;
      }
   |  STEM CONSTANT
      {
      strcpy (ExecIO_Options.aStem, $2);
      }
   |  STEM CONSTANT FINIS
      {
      strcpy (ExecIO_Options.aStem, $2);
      ExecIO_Options.fFinis = true;
      }
   |  FINIS STEM CONSTANT
      {
      strcpy (ExecIO_Options.aStem, $3);
      ExecIO_Options.fFinis = true;
      }
   |
   ;

diskw_option : '(' diskw_options
   |
   ;

diskw_options : FINIS
      {
      ExecIO_Options.fFinis = true;
      }
   |  STEM CONSTANT
      {
      strcpy (ExecIO_Options.aStem, $2);
      }
   |  STEM CONSTANT FINIS
      {
      strcpy (ExecIO_Options.aStem, $2);
      ExecIO_Options.fFinis = true;
      }
   |  FINIS STEM CONSTANT
      {
      strcpy (ExecIO_Options.aStem, $3);
      ExecIO_Options.fFinis = true;
      }
   |
   ;
%%


/*--------------------------------------------------------------------*/
/*                                                                    */
/*  yylex () - lexical analyzer.                                      */
/*                                                                    */
/*--------------------------------------------------------------------*/

int yylex (
   void)                         /* no arguments                      */
   {

   /* local function variables */
   int tktype, idx = 0;
   static char token [1024];

   while (*(prxCmd -> strptr + lCmdPtr) == ' ' ||
    *(prxCmd -> strptr + lCmdPtr) == '\t')
      lCmdPtr++;
   if (*(prxCmd -> strptr + lCmdPtr) == '\0')
      return 0;
   if (*(prxCmd -> strptr + lCmdPtr) == '*')
      {
      lCmdPtr++;
      return '*';
      }
   else if (*(prxCmd -> strptr + lCmdPtr) == '(')
      {
      lCmdPtr++;
      return '(';
      }
   else if (*(prxCmd -> strptr + lCmdPtr) == '\"')
      {
      lCmdPtr++;
      while (*(prxCmd -> strptr + lCmdPtr) != '\"')
         {
         if (*(prxCmd -> strptr + lCmdPtr) == '\0')
            return 0;
         if (idx == 1024)
            return 0;
         token[idx] = *(prxCmd -> strptr + lCmdPtr);
         lCmdPtr++;
         idx++;
         }
      token[idx] = '\0';
      lCmdPtr++;
      /* insert it into our symbol table */
      if (ulNumSym == SYMTABLESIZE)
         return 0;
      pszSymbol[ulNumSym] = (char *)malloc(strlen (token) + 1);
      strcpy (pszSymbol[ulNumSym], token);
      yylval.strval = pszSymbol[ulNumSym];
      ulNumSym++;
      return CONSTANT;
      }
   else if (isalpha (*(prxCmd -> strptr + lCmdPtr)) ||
    isdigit (*(prxCmd -> strptr + lCmdPtr)) ||
    *(prxCmd -> strptr + lCmdPtr) == '\\')
      {
      while (*(prxCmd -> strptr + lCmdPtr) != ' ' &&
       *(prxCmd -> strptr + lCmdPtr) != '\0')
         {
         if (idx == 1024)
            return 0;
         token[idx] = *(prxCmd -> strptr + lCmdPtr);
         lCmdPtr++;
         idx++;
         }
      token[idx] = '\0';
      /* insert it into our symbol table */
      if (ulNumSym == SYMTABLESIZE)
         return 0;
      pszSymbol[ulNumSym] = (char *)malloc(strlen (token) + 1);
      strcpy (pszSymbol[ulNumSym], token);
      yylval.strval = pszSymbol[ulNumSym];
      ulNumSym++;
      return kwsearch (token);
      }
   }


/*--------------------------------------------------------------------*/
/*                                                                    */
/*  yyerror () - error handler.                                       */
/*                                                                    */
/*--------------------------------------------------------------------*/

void yyerror (
   char * token)                 /* token string                      */
   {

   fprintf (stderr, "Token \"%s\"\n", token);
   return;
   }


/*--------------------------------------------------------------------*/
/*                                                                    */
/*  Keyword table                                                     */
/*                                                                    */
/*--------------------------------------------------------------------*/

static struct
   {
   const char * kw;
   int          type;
   } kwtable [] = {
                  "HI",     HI,
                  "TE",     TE,
                  "TS",     TS,
                  "EXECIO", EXECIO,
                  "DISKW",  DISKW,
                  "DISKR",  DISKR,
                  "STEM",   STEM,
                  "FINIS",  FINIS,
                  "FIFO",   FIFO,
                  "LIFO",   LIFO,
                  "SKIP",   SKIP,
                  "eot",    EOF
                  };


/*--------------------------------------------------------------------*/
/*                                                                    */
/*  kwsearch () - search for keywords.                                */
/*                                                                    */
/*--------------------------------------------------------------------*/

int kwsearch (
   char * token)                 /* token string                      */
   {

   /* local function variables */
   int i;
   char *utoken, *uutoken;

   utoken = strdup(token);
   uutoken = utoken;
   while (*uutoken != '\0')
   {
      *uutoken = toupper(*uutoken);
      uutoken++;
   }
   for (i = 0; kwtable[i].type != EOF; i++)
      if (strcmp(utoken, kwtable[i].kw) == 0)
      {
         free(utoken);
         return kwtable[i].type;
      }
   free(utoken);
   return CONSTANT;
   }


/*--------------------------------------------------------------------*/
/*                                                                    */
/*  isnumeric () - is a string numeric?                               */
/*                                                                    */
/*--------------------------------------------------------------------*/

bool isnumeric (
   char * token)                 /* token string                      */
   {

   /* local function variables */
   int i;

   for (i = 0; i < strlen (token); i++)
      if (!isdigit(*(token + i)))
         return false;
   return true;
   }

