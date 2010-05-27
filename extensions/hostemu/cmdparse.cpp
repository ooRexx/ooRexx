#ifndef lint
static const char yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93";
#endif

#include <stdlib.h>
#include <string.h>

#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define YYPATCH 20070509

#define YYEMPTY (-1)
#define yyclearin    (yychar = YYEMPTY)
#define yyerrok      (yyerrflag = 0)
#define YYRECOVERING (yyerrflag != 0)

extern int yyparse(void);

static int yygrowstack(void);
#define YYPREFIX "yy"
#line 2 "cmdparse.ypp"
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


#line 73 "cmdparse.ypp"
typedef union
   {
   int    numval;
   char * strval;
   } YYSTYPE;
#line 100 "cmdparse.cpp"
#define EXECIO 257
#define HI 258
#define TE 259
#define TS 260
#define CONSTANT 261
#define DISKW 262
#define DISKR 263
#define STEM 264
#define FINIS 265
#define LIFO 266
#define FIFO 267
#define SKIP 268
#define YYERRCODE 256
short yylhs[] = {                                        -1,
    0,    0,    0,    0,    0,    1,    1,    1,    3,    3,
    4,    4,    4,    4,    4,    4,    4,    4,    2,    2,
    5,    5,    5,    5,    5,
};
short yylen[] = {                                         2,
    3,    3,    1,    1,    1,    3,    3,    4,    2,    0,
    1,    2,    2,    2,    2,    3,    3,    0,    2,    0,
    1,    2,    3,    3,    0,
};
short yydefred[] = {                                      0,
    0,    3,    4,    5,    0,    0,    0,    0,    0,    1,
    2,    0,    0,    0,    6,    0,    0,    7,    0,    0,
   19,    8,    0,    0,    9,    0,    0,    0,    0,   13,
   12,   14,   23,   24,   16,   17,
};
short yydgoto[] = {                                       5,
   10,   15,   18,   25,   21,
};
short yysindex[] = {                                   -250,
  -41,    0,    0,    0,    0, -251, -251, -258, -248,    0,
    0,  -26,  -40, -249,    0,  -21, -247,    0, -241, -243,
    0,    0, -239, -262,    0, -242, -237, -240, -235,    0,
    0,    0,    0,    0,    0,    0,
};
short yyrindex[] = {                                      0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   27,   28,   29,    0,   28,   30,    0,    0,   31,
    0,    0,    0,   32,    0,   33,    0,   34,    0,    0,
    0,    0,    0,    0,    0,    0,
};
short yygindex[] = {                                      0,
   35,    0,   19,    0,    0,
};
#define YYTABLESIZE 221
short yytable[] = {                                      17,
    7,   29,   12,   30,   31,   32,    1,    2,    3,    4,
    8,    9,   13,   14,   19,   20,   23,   24,   17,   26,
   27,   28,   33,   34,   35,   36,   20,   10,   25,   18,
   21,   11,   22,   15,   22,    0,    0,    0,    0,    0,
    0,   11,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    6,
   16,
};
short yycheck[] = {                                      40,
   42,  264,  261,  266,  267,  268,  257,  258,  259,  260,
  262,  263,  261,   40,  264,  265,  264,  265,   40,  261,
  264,  261,  265,  261,  265,  261,    0,    0,    0,    0,
    0,    0,    0,    0,   16,   -1,   -1,   -1,   -1,   -1,
   -1,    7,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  261,
  261,
};
#define YYFINAL 5
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 268
#if YYDEBUG
char *yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,"'('",0,"'*'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"EXECIO","HI","TE","TS",
"CONSTANT","DISKW","DISKR","STEM","FINIS","LIFO","FIFO","SKIP",
};
char *yyrule[] = {
"$accept : stmt",
"stmt : EXECIO CONSTANT disk_clause",
"stmt : EXECIO '*' disk_clause",
"stmt : HI",
"stmt : TE",
"stmt : TS",
"disk_clause : DISKW CONSTANT diskw_option",
"disk_clause : DISKR CONSTANT diskr_option",
"disk_clause : DISKR CONSTANT CONSTANT diskr_option",
"diskr_option : '(' diskr_options",
"diskr_option :",
"diskr_options : FINIS",
"diskr_options : FINIS FIFO",
"diskr_options : FINIS LIFO",
"diskr_options : FINIS SKIP",
"diskr_options : STEM CONSTANT",
"diskr_options : STEM CONSTANT FINIS",
"diskr_options : FINIS STEM CONSTANT",
"diskr_options :",
"diskw_option : '(' diskw_options",
"diskw_option :",
"diskw_options : FINIS",
"diskw_options : STEM CONSTANT",
"diskw_options : STEM CONSTANT FINIS",
"diskw_options : FINIS STEM CONSTANT",
"diskw_options :",
};
#endif
#if YYDEBUG
#include <stdio.h>
#endif

/* define the initial stack-sizes */
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH  YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 500
#define YYMAXDEPTH  500
#endif
#endif

#define YYINITSTACKSIZE 500

int      yydebug;
int      yynerrs;
int      yyerrflag;
int      yychar;
short   *yyssp;
YYSTYPE *yyvsp;
YYSTYPE  yyval;
YYSTYPE  yylval;

/* variables for the parser stack */
static short   *yyss;
static short   *yysslim;
static YYSTYPE *yyvs;
static int      yystacksize;
#line 211 "cmdparse.ypp"


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

#line 451 "cmdparse.cpp"
/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int yygrowstack(void)
{
    int newsize, i;
    short *newss;
    YYSTYPE *newvs;

    if ((newsize = yystacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return -1;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;

    i = yyssp - yyss;
    newss = (yyss != 0)
          ? (short *)realloc(yyss, newsize * sizeof(*newss))
          : (short *)malloc(newsize * sizeof(*newss));
    if (newss == 0)
        return -1;

    yyss  = newss;
    yyssp = newss + i;
    newvs = (yyvs != 0)
          ? (YYSTYPE *)realloc(yyvs, newsize * sizeof(*newvs))
          : (YYSTYPE *)malloc(newsize * sizeof(*newvs));
    if (newvs == 0)
        return -1;

    yyvs = newvs;
    yyvsp = newvs + i;
    yystacksize = newsize;
    yysslim = yyss + newsize - 1;
    return 0;
}

#define YYABORT goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR goto yyerrlab
int
yyparse(void)
{
    register int yym, yyn, yystate;
#if YYDEBUG
    register const char *yys;

    if ((yys = getenv("YYDEBUG")) != 0)
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = YYEMPTY;

    if (yyss == NULL && yygrowstack()) goto yyoverflow;
    yyssp = yyss;
    yyvsp = yyvs;
    *yyssp = yystate = 0;

yyloop:
    if ((yyn = yydefred[yystate]) != 0) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yyssp >= yysslim && yygrowstack())
        {
            goto yyoverflow;
        }
        *++yyssp = yystate = yytable[yyn];
        *++yyvsp = yylval;
        yychar = YYEMPTY;
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;

    yyerror("syntax error");

#ifdef lint
    goto yyerrlab;
#endif

yyerrlab:
    ++yynerrs;

yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yyssp, yytable[yyn]);
#endif
                if (yyssp >= yysslim && yygrowstack())
                {
                    goto yyoverflow;
                }
                *++yyssp = yystate = yytable[yyn];
                *++yyvsp = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yyssp);
#endif
                if (yyssp <= yyss) goto yyabort;
                --yyssp;
                --yyvsp;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = YYEMPTY;
        goto yyloop;
    }

yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    if (yym)
        yyval = yyvsp[1-yym];
    else
        memset(&yyval, 0, sizeof yyval);
    switch (yyn)
    {
case 1:
#line 96 "cmdparse.ypp"
{
      lStmtType = EXECIO_STMT;
      ExecIO_Options.lStartRcd = 1;
      /* if constant is not a numeric string we have an error */
      if (isnumeric (yyvsp[-1].strval))
         ExecIO_Options.lRcdCnt = atoi (yyvsp[-1].strval);
      else
         return 1;
      }
break;
case 2:
#line 106 "cmdparse.ypp"
{
      lStmtType = EXECIO_STMT;
      ExecIO_Options.lStartRcd = 1;
      ExecIO_Options.lRcdCnt = -1;
      }
break;
case 3:
#line 112 "cmdparse.ypp"
{
      lStmtType = HI_STMT;
      }
break;
case 4:
#line 116 "cmdparse.ypp"
{
      lStmtType = TE_STMT;
      }
break;
case 5:
#line 120 "cmdparse.ypp"
{
      lStmtType = TS_STMT;
      }
break;
case 6:
#line 126 "cmdparse.ypp"
{
      ExecIO_Options.fRW = true;
      strcpy (ExecIO_Options.aFilename, yyvsp[-1].strval);
      }
break;
case 7:
#line 131 "cmdparse.ypp"
{
      ExecIO_Options.fRW = false;
      strcpy (ExecIO_Options.aFilename, yyvsp[-1].strval);
      }
break;
case 8:
#line 136 "cmdparse.ypp"
{
      ExecIO_Options.fRW = false;
      strcpy (ExecIO_Options.aFilename, yyvsp[-2].strval);
      /* if constant is not a numeric string we have an error */
      if (isnumeric (yyvsp[-1].strval))
         ExecIO_Options.lStartRcd = atoi (yyvsp[-1].strval);
      else
         return 1;
      }
break;
case 11:
#line 152 "cmdparse.ypp"
{
      ExecIO_Options.fFinis = true;
      }
break;
case 12:
#line 156 "cmdparse.ypp"
{
      ExecIO_Options.fFinis = false;
      }
break;
case 13:
#line 160 "cmdparse.ypp"
{
      ExecIO_Options.fFinis = true;
      ExecIO_Options.lDirection = 1;
      }
break;
case 14:
#line 165 "cmdparse.ypp"
{
      ExecIO_Options.fFinis = true;
      ExecIO_Options.lDirection = 2;
      }
break;
case 15:
#line 170 "cmdparse.ypp"
{
      strcpy (ExecIO_Options.aStem, yyvsp[0].strval);
      }
break;
case 16:
#line 174 "cmdparse.ypp"
{
      strcpy (ExecIO_Options.aStem, yyvsp[-1].strval);
      ExecIO_Options.fFinis = true;
      }
break;
case 17:
#line 179 "cmdparse.ypp"
{
      strcpy (ExecIO_Options.aStem, yyvsp[0].strval);
      ExecIO_Options.fFinis = true;
      }
break;
case 21:
#line 191 "cmdparse.ypp"
{
      ExecIO_Options.fFinis = true;
      }
break;
case 22:
#line 195 "cmdparse.ypp"
{
      strcpy (ExecIO_Options.aStem, yyvsp[0].strval);
      }
break;
case 23:
#line 199 "cmdparse.ypp"
{
      strcpy (ExecIO_Options.aStem, yyvsp[-1].strval);
      ExecIO_Options.fFinis = true;
      }
break;
case 24:
#line 204 "cmdparse.ypp"
{
      strcpy (ExecIO_Options.aStem, yyvsp[0].strval);
      ExecIO_Options.fFinis = true;
      }
break;
#line 768 "cmdparse.cpp"
    }
    yyssp -= yym;
    yystate = *yyssp;
    yyvsp -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yyssp = YYFINAL;
        *++yyvsp = yyval;
        if (yychar < 0)
        {
            if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yyssp, yystate);
#endif
    if (yyssp >= yysslim && yygrowstack())
    {
        goto yyoverflow;
    }
    *++yyssp = yystate;
    *++yyvsp = yyval;
    goto yyloop;

yyoverflow:
    yyerror("yacc stack overflow");

yyabort:
    return (1);

yyaccept:
    return (0);
}
