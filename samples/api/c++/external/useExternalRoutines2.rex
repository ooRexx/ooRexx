/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2021 Rexx Language Association. All rights reserved.         */
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
/* THIS SOFTWARE IS PROVIDED BY THE COPYright HOLDERS AND CONTRIBUTORS        */
/* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT          */
/* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS          */
/* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYright   */
/* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,      */
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,        */
/* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY     */
/* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING    */
/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         */
/* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               */
/*                                                                            */
/*----------------------------------------------------------------------------*/

say "NoArgRoutineReturn123()    :" pp(NoArgRoutineReturn123())
say
say "calling a routine that returns nothing:"
call NoArgRoutineVoid
say
say "OneArgRoutineReturnArg(1)  :" pp(OneArgRoutineReturnArg(1))
say "OneArgRoutineReturnArg( )  :" pp(OneArgRoutineReturnArg())
say
say "calling another routine that returns nothing:"
call OneArgRoutineVoid 2
say
say "TwoIntArgAdder(4, 5)       :" pp(TwoIntArgAdder(4, 5))
say "TwoDoubleArgAdder(6  , 8  ):" pp(TwoDoubleArgAdder(6  , 8  ))
say "TwoDoubleArgAdder(6.7, 8.9):" pp(TwoDoubleArgAdder(6.7, 8.9))
say "---"

say "Invoking an external routine with wrong arguments:"
say "TwoIntArgAdder(9.8, 7.6)   :" pp(TwoIntArgAdder(9.8, 7.6))

::requires "external_routines" library

::routine pp   -- return string value enclosed in square brackets
  return "["arg(1)"]"
