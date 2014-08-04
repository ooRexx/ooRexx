/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
/******************************************************************************/
/* REXX Kernel                                              GlobalNames.h     */
/*                                                                            */
/* Definitions of all name objects created at startup time.  All these        */
/* Name objects are addressible via OREF_ global names.  Changes to this file */
/* also require recompilation of GlobalNames.cpp and GlobalData.cpp for things*/
/* to link correctly.                                                         */
/*                                                                            */
/* NOTE:  The string values of these constants do not need to be a symbolic   */
/* CHAR_* name.  These values can be directly coded as a literal string.  If  */
/* an existing CHAR_* constant is available for the value to be defined, use  */
/* the symbol.  Otherwise, it is perfectly acceptable to just use a literal   */
/* string value here.                                                         */
/*                                                                            */
/******************************************************************************/

  GLOBAL_NAME(ACTIVATE, "ACTIVATE")
  GLOBAL_NAME(ADDITIONAL, "ADDITIONAL")
  GLOBAL_NAME(ADDRESS, "ADDRESS")
  GLOBAL_NAME(AND, "*")
//GLOBAL_NAME(ASSIGNMENT_AND, CHAR_ASSIGNMENT_AND)
  GLOBAL_NAME(ANY, "ANY")
  GLOBAL_NAME(ARGUMENTS, "ARGUMENTS")
  GLOBAL_NAME(ARRAY, "ARRAY")
  GLOBAL_NAME(BACKSLASH, "\\")
  GLOBAL_NAME(BACKSLASH_EQUAL, "\\=")
  GLOBAL_NAME(BACKSLASH_GREATERTHAN, "\\>")
  GLOBAL_NAME(BACKSLASH_LESSTHAN, "\\<")
  GLOBAL_NAME(BLANK, " ")
//GLOBAL_NAME(BRACKETS, CHAR_BRACKETS)
  GLOBAL_NAME(CALL, "CALL")
//GLOBAL_NAME(CHARIN, CHAR_CHARIN)
//GLOBAL_NAME(CHAROUT, CHAR_CHAROUT)
//GLOBAL_NAME(CHARS, CHAR_CHARS)
//GLOBAL_NAME(CLASSSYM, CHAR_CLASS)
  GLOBAL_NAME(CLOSE, "CLOSE")
//GLOBAL_NAME(CODE, CHAR_CODE)
  GLOBAL_NAME(COMMAND, "COMMAND")
  GLOBAL_NAME(COMPARE, "COMPARE")
  GLOBAL_NAME(COMPARETO, "COMPARETO")
  GLOBAL_NAME(CONCATENATE, "||")
//GLOBAL_NAME(ASSIGNMENT_CONCATENATE, CHAR_ASSIGNMENT_CONCATENATE)
  GLOBAL_NAME(CONDITION, "CONDITION")
  GLOBAL_NAME(CSELF, "CSELF")
  GLOBAL_NAME(DEBUGINPUT, "DEBUGINPUT")
  GLOBAL_NAME(DEFAULTNAME, "DEFAULTNAME")
  GLOBAL_NAME(DELAY, "DELAY")
  GLOBAL_NAME(DESCRIPTION, "DESCRIPTION")
  GLOBAL_NAME(DIVIDE, "/")
//GLOBAL_NAME(ASSIGNMENT_DIVIDE, CHAR_ASSIGNMENT_DIVIDE)
  GLOBAL_NAME(ENGINEERING, "ENGINEERING")
  GLOBAL_NAME(ENVIRONMENT, "ENVIRONMENT")
  GLOBAL_NAME(EQUAL, "=")
  GLOBAL_NAME(ERRORNAME, "ERROR")
//GLOBAL_NAME(ERRORTEXT, CHAR_ERRORTEXT)
//GLOBAL_NAME(EXCEPTION, CHAR_EXCEPTION)
  GLOBAL_NAME(FAILURE, "FAILURE")
//GLOBAL_NAME(FILE, CHAR_FILE)
//GLOBAL_NAME(FILESYSTEM, CHAR_FILESYSTEM)
  GLOBAL_NAME(FUNCTION, "FUNCTION")
//GLOBAL_NAME(GET, CHAR_GET)
  GLOBAL_NAME(GREATERTHAN, ">")
  GLOBAL_NAME(GREATERTHAN_EQUAL, ">=")
  GLOBAL_NAME(GREATERTHAN_LESSTHAN, "><")
  GLOBAL_NAME(HALT, "HALT")
//GLOBAL_NAME(HASMETHOD, CHAR_HASMETHOD)
  GLOBAL_NAME(HASHCODE, "HASHCODE")
//GLOBAL_NAME(INHERIT, CHAR_INHERIT)
  GLOBAL_NAME(INIT, "INIT")
//GLOBAL_NAME(INITINSTANCE, CHAR_INITINSTANCE)
//GLOBAL_NAME(INITIALADDRESS, CHAR_INITIALADDRESS)
  GLOBAL_NAME(INPUT, "INPUT")
//GLOBAL_NAME(INSERT, CHAR_INSERT)
  GLOBAL_NAME(INSTRUCTION, "INSTRUCTION")
  GLOBAL_NAME(INTDIV, "%")
//GLOBAL_NAME(ASSIGNMENT_INTDIV, CHAR_ASSIGNMENT_INTDIV)
  GLOBAL_NAME(LESSTHAN, "<")
  GLOBAL_NAME(LESSTHAN_EQUAL, "<=")
  GLOBAL_NAME(LESSTHAN_GREATERTHAN, "<>")
//GLOBAL_NAME(LINEIN, CHAR_LINEIN)
//GLOBAL_NAME(LINEOUT, CHAR_LINEOUT)
//GLOBAL_NAME(LINES, CHAR_LINES)
  GLOBAL_NAME(LOCAL, "LOCAL")
  GLOBAL_NAME(LOSTDIGITS, "LOSTDIGITS")
  GLOBAL_NAME(MAKEARRAY, "MAKEARRAY")
  GLOBAL_NAME(MAKESTRING, "MAKESTRING")
  GLOBAL_NAME(METHOD, "METHOD")
//GLOBAL_NAME(METHODS, CHAR_METHODS)
  GLOBAL_NAME(MULTIPLY, "*")
//GLOBAL_NAME(ASSIGNMENT_MULTIPLY, CHAR_ASSIGNMENT_MULTIPLY)
  GLOBAL_NAME(NAME, "NAME")
//GLOBAL_NAME(NAME_MESSAGE, CHAR_MESSAGE)
  GLOBAL_NAME(NEW, "NEW")
  GLOBAL_NAME(NOMETHOD, "NOMETHOD")
  GLOBAL_NAME(NONE, "NONE")
//GLOBAL_NAME(NORMAL, CHAR_NORMAL)
  GLOBAL_NAME(NOSTRING, "NOSTRING")
  GLOBAL_NAME(NOVALUE, "NOVALUE")
  GLOBAL_NAME(NULLSTRING, "")
  GLOBAL_NAME(OBJECT, "OBJECT")
  GLOBAL_NAME(OBJECTNAME, "OBJECTNAME")
  GLOBAL_NAME(OFF, "OFF")
  GLOBAL_NAME(ON, "ON")
  GLOBAL_NAME(OR, "|")
//GLOBAL_NAME(ASSIGNMENT_OR, CHAR_ASSIGNMENT_OR)
  GLOBAL_NAME(OUTPUT, "OUTPUT")
//GLOBAL_NAME(PERIOD, CHAR_PERIOD)
  GLOBAL_NAME(PLUS, "+")
//GLOBAL_NAME(ASSIGNMENT_PLUS, CHAR_ASSIGNMENT_PLUS)
//GLOBAL_NAME(POSITION, CHAR_POSITION)
  GLOBAL_NAME(POWER, "**")
//GLOBAL_NAME(ASSIGNMENT_POWER, CHAR_ASSIGNMENT_POWER)
  GLOBAL_NAME(PROGRAM, "PROGRAM")
//GLOBAL_NAME(PACKAGE, CHAR_PACKAGE)
  GLOBAL_NAME(PROPAGATE, "PROPAGATE")
  GLOBAL_NAME(PROPAGATED, "PROPAGATED")
//GLOBAL_NAME(PULL, CHAR_PULL)
//GLOBAL_NAME(PUSH, CHAR_PUSH)
//GLOBAL_NAME(PUT, CHAR_PUT)
//GLOBAL_NAME(QUEUED, CHAR_QUEUED)
//GLOBAL_NAME(QUEUENAME, CHAR_QUEUE)
//GLOBAL_NAME(QUERY, CHAR_QUERY)
  GLOBAL_NAME(RC, "RC")
  GLOBAL_NAME(REMAINDER, "//")
  GLOBAL_NAME(REXX, "REXX")
//GLOBAL_NAME(ASSIGNMENT_REMAINDER, CHAR_ASSIGNMENT_REMAINDER)
  GLOBAL_NAME(REQUEST, "REQUEST")
  GLOBAL_NAME(REQUIRES, "REQUIRES")
  GLOBAL_NAME(RESULT, "RESULT")
  GLOBAL_NAME(REXXQUEUE, "REXXQUEUE")
//GLOBAL_NAME(REXXUTIL, CHAR_REXXUTIL)
  GLOBAL_NAME(ROUTINE, "ROUTINE")
  GLOBAL_NAME(RUN, "RUN")
//GLOBAL_NAME(SAY, CHAR_SAY)
  GLOBAL_NAME(SCIENTIFIC, "SCIENTIFIC")
//GLOBAL_NAME(SCRIPT, CHAR_SCRIPT)
  GLOBAL_NAME(SECURITYMANAGER, "SECURITYMANAGER")
  GLOBAL_NAME(SELF, "SELF")
//GLOBAL_NAME(SEND, CHAR_SEND)
//GLOBAL_NAME(SERVER, CHAR_SERVER)
//GLOBAL_NAME(SESSION, CHAR_SESSION)
//GLOBAL_NAME(SET, CHAR_SET)
  GLOBAL_NAME(SIGL, "SIGL")
  GLOBAL_NAME(SIGNAL, "SIGNAL")
//GLOBAL_NAME(SOURCENAME, CHAR_SOURCE)
//GLOBAL_NAME(STDERR, CHAR_STDERR)
//GLOBAL_NAME(STDIN,  CHAR_STDIN)
//GLOBAL_NAME(STDOUT, CHAR_STDOUT)
//GLOBAL_NAME(CSTDERR, CHAR_CSTDERR)/* standard streams with colon */
//GLOBAL_NAME(CSTDIN,  CHAR_CSTDIN) /* standard streams with colon */
//GLOBAL_NAME(CSTDOUT, CHAR_CSTDOUT)/* standard streams with colon */
  GLOBAL_NAME(STREAM, "STREAM")
//GLOBAL_NAME(STREAMS, CHAR_STREAMS)
//GLOBAL_NAME(STATE, CHAR_STATE)
  GLOBAL_NAME(STRICT_BACKSLASH_EQUAL, "\\==")
  GLOBAL_NAME(STRICT_BACKSLASH_GREATERTHAN, "\\>>")
  GLOBAL_NAME(STRICT_BACKSLASH_LESSTHAN, "\\<<")
  GLOBAL_NAME(STRICT_EQUAL, "==")
  GLOBAL_NAME(STRICT_GREATERTHAN, ">>")
  GLOBAL_NAME(STRICT_GREATERTHAN_EQUAL, ">>=")
  GLOBAL_NAME(STRICT_LESSTHAN, "<<")
  GLOBAL_NAME(STRICT_LESSTHAN_EQUAL, "<<=")
  GLOBAL_NAME(STRING, "STRING")
//GLOBAL_NAME(SUBROUTINE, CHAR_SUBROUTINE)
  GLOBAL_NAME(SUBTRACT, "-")
//GLOBAL_NAME(ASSIGNMENT_SUBTRACT, CHAR_ASSIGNMENT_SUBTRACT)
  GLOBAL_NAME(SUPER, "SUPER")
  GLOBAL_NAME(SUPPLIER, "SUPPLIER")
  GLOBAL_NAME(SYNTAX, "SYNTAX")
//GLOBAL_NAME(TOKENIZE_ONLY, CHAR_TOKENIZE_ONLY)
//GLOBAL_NAME(TRACEBACK, CHAR_TRACEBACK)
//GLOBAL_NAME(TRACEOUTPUT, CHAR_TRACEOUTPUT)
//GLOBAL_NAME(STACKFRAMES, CHAR_STACKFRAMES)
  GLOBAL_NAME(UNINIT, "UNINIT")
  GLOBAL_NAME(UNKNOWN, "UNKNOWN")
//GLOBAL_NAME(VALUE, CHAR_VALUE)
//GLOBAL_NAME(VERSION, CHAR_VERSION)
  GLOBAL_NAME(XOR, "&&")
//GLOBAL_NAME(ASSIGNMENT_XOR, CHAR_ASSIGNMENT_XOR)
  GLOBAL_NAME(ZERO, "0")
//
//GLOBAL_NAME(SIGTERM_STRING, CHAR_SIGTERM)
//GLOBAL_NAME(SIGINT_STRING, CHAR_SIGINT)
//GLOBAL_NAME(SIGHUP_STRING, CHAR_SIGHUP)

