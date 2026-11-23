/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2026 Josep Maria Blasco. All rights reserved.                */
/* Copyright (c) 2026 Rexx Language Association. All rights reserved.         */
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
/******************************************************************************/
/*                                                                            */
/* identity.cls -- The identity compiler                                      */
/* =====================================                                      */
/*                                                                            */
/* The identity compiler compiles a program, through the parser's tree, to    */
/* a perfect copy of itself. A program that uses it needs only                */
/*                                                                            */
/*   ::Requires "parser.cls"                                                  */
/*   ::Requires "parser/modules/identity.cls"                                 */
/*                                                                            */
/* and compiles a program with                                                */
/*                                                                            */
/*   parser~package~compile(parser~firstElement, stream, .StringTable~new)    */
/*                                                                            */
/* The compiler is in identity/: compile.cls has the routines that other      */
/* compilers built on it use (Clone and PrepareClause), and the compile       */
/* methods of code bodies and packages; the other files, the compile          */
/* methods of clauses, directives, expressions, instructions, iterations      */
/* and parsing templates.                                                     */
/*                                                                            */
/* This file is part of the Rexx Parser package                               */
/* [See https://rexx.epbcn.com/rexx-parser/]                                  */
/*                                                                            */
/* Version history:                                                           */
/*                                                                            */
/* Date     Version Details                                                   */
/* -------- ------- --------------------------------------------------------- */
/* 20261123    0.7  First version (RFE #878)                                  */
/*                                                                            */
/******************************************************************************/

::Requires "identity/compile.cls"
::Requires "identity/Clauses.cls"
::Requires "identity/Directives.cls"
::Requires "identity/Expressions.cls"
::Requires "identity/Instructions.cls"
::Requires "identity/Iterations.cls"
::Requires "identity/Parsing.cls"
