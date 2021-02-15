#!@OOREXX_SHEBANG_PROGRAM@
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
/******************************************************************************/
/*  pipe.rex            Open Object Rexx Samples                              */
/*                                                                            */
/*  Show samples uses of the pipe implementation in pipe.rex                  */
/*                                                                            */
/*                                                                            */
/* -------------------------------------------------------------------------- */
/*                                                                            */
/*  Description:                                                              */
/*  This program demonstrates how one could use the pipes implemented in the  */
/*  pipe sample.                                                              */
/******************************************************************************/

info = .array~of('Tom','Mike','Rick','Steve')  /* Create an array to use      */
                                               /* stages on (any collection   */
                                               /* would work).                */

pipe = .sort|.reverse|.displayer               /* Pipe to sort, reverse       */
                                               /* elements and display.       */

pipe~go(info)                                  /* Run it                      */

say '-----------------------------------'

pipe = .all['e']|.displayer                    /* Pipe to select elements with*/
                                               /* 'e' and display.            */

pipe~go(info)                                  /* Run it                      */
say '-----------------------------------'
array1 = .array~new
array2 = .array~new

-- this hooks stagess up to two different output streams
-- the >> secondary stage must preceed the primary hooked up to the same
-- stage
pipe = .all['e'] >> .arraycollector[array2] > .arraycollector[array1]


pipe~go(info)                                  /* Run it                      */


say 'Items selected by all are'

do item over array1
   say item
end

say
say 'Items not selected by all are'

do item over array2
   say item
end

say '-----------------------------------'

pipe = .sort|.stemcollector[a.]                /* Pipe to sort, put in a.     */

pipe~go(info)                                  /* Run it                      */

Do i = 1 To a.0                                /* Show stem values            */
  Say a.i
End

say '-----------------------------------'

array1 = .array~new
array2 = .array~new

pivot = 'S'

pipe = .pivot[pivot, .arraycollector[array1], .arraycollector[array2]]

pipe~go(info)                                  /* Run it                      */

say 'Items less than pivot' pivot 'are'

do item over array1
   say item
end

say
say 'Items greater than or equal to pivot' pivot 'are'

do item over array2
   say item
end


::REQUIRES 'pipe.cls'
