/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2021 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                         */
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
/***************************************************************************/
/*                                                                         */
/*  apitest1.rex       Open Object Rexx samples                            */
/*                                                                         */
/* ----------------------------------------------------------------------- */
/*                                                                         */
/*  Description:       script to test the REXX interpreter API             */
/*                                                                         */
/***************************************************************************/

say "1a) 'call RxFuncAdd', expecting '0' as result"
Call RxFuncAdd "ApiLoadFuncs", "rexxapi1", "ApiLoadFuncs"
say "1b) result:" result
say "---"

say "2a) 'call ApiLoadFuncs', null (empty) string expected as result"
Call ApiLoadFuncs
say "2b) result:" result
say "---"

say "3a) 'call Api_Output_From_C', library version '1.0' expected as result"
call Api_Output_From_C
say "3b) result:" result
say "---"

say "4a) 'call Api_Output_FROM_REXX', expecting string as result"
call Api_Output_From_REXX
say "4b) result:" result
say "---"

say "5a) 'res=Api_Exchange_Data(23456,888, a)', expecting '0' as result"
a = 'This is a String'
res=Api_Exchange_Data(23456,888, a)
say "5b) res:" res
say "---"

say "6a) 'call ApiDeregFunc', no result expected"
call ApiDeregFunc
say "6b) result:" result
say "---"

