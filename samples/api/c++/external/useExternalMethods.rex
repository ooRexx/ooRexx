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

o=.test~new          -- create object
o~hello
say "---"
say "o~NoArgMethodReturn123()    :" pp(o~NoArgMethodReturn123)
say
say "invoking a method that returns nothing:"
o~NoArgMethodVoid
say
say "o~OneArgMethodReturnArg(1)  :" pp(o~OneArgMethodReturnArg(1))
say "o~OneArgMethodReturnArg( )  :" pp(o~OneArgMethodReturnArg( ))
say
say "invoking another method that returns nothing:"
o~OneArgMethodVoid(2)
say
say "o~TwoIntArgAdder(4, 5)       :" pp(o~TwoIntArgAdder(4, 5))
say "o~TwoDoubleArgAdder(6  , 8  ):" pp(o~TwoDoubleArgAdder(6  , 8  ))
say "o~TwoDoubleArgAdder(6.7, 8.9):" pp(o~TwoDoubleArgAdder(6.7, 8.9))
say "---"
say "invoking a method that natively will send 'hello' to its object:"
o~OneArgInvokeOrxMethod(-123)
say "---"

say "Invoking an external method with wrong arguments:"
say "o~TwoIntArgAdder(9.8, 7.6)   :" pp(o~TwoIntArgAdder(9.8, 7.6))


::routine pp   -- return string value enclosed in square brackets
  return "["arg(1)"]"

::class test            -- define class and native methods for it
::method NoArgMethodReturn123  external "LIBRARY external_methods NoArgMethodReturn123 "
::method NoArgMethodVoid       external "LIBRARY external_methods NoArgMethodVoid      "
::method OneArgMethodReturnArg external "LIBRARY external_methods OneArgMethodReturnArg"
::method OneArgMethodVoid      external "LIBRARY external_methods OneArgMethodVoid     "
::method TwoIntArgAdder        external "LIBRARY external_methods TwoIntArgAdder       "
::method TwoDoubleArgAdder     external "LIBRARY external_methods TwoDoubleArgAdder    "
::method OneArgInvokeOrxMethod external "LIBRARY external_methods OneArgInvokeOrxMethod"

::method hello
  say "hello from" pp(self) "self~identityHash:" pp(self~identityHash)
