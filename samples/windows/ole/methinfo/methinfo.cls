/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
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
/*******************************************************************/
/* methodInfo class:                                               */
/* retrieve method information from OLE object and output it       */
/* to stdout.                                                      */
/*******************************************************************/

::class 'MethodInfo' public

::method init
  expose methods.
  use arg oleobj
  methods. = oleobj~GetKnownMethods

::method print
  expose methods.
  if methods. == .nil then return
  do i = 1 to methods.0
    signature = methods.i.!RETTYPE
    signature = signature methods.i.!NAME
    if methods.i.!INVKIND = 4 then
      signature = signature"= ("methods.i.!PARAMS.1.!TYPE "as the argument)"
    else do
      do j = 1 to methods.i.!PARAMS.0
        if j = 1 then signature = signature"("
        signature = signature||methods.i.!PARAMS.j.!FLAGS methods.i.!PARAMS.j.!TYPE methods.i.!PARAMS.j.!NAME
        if j = methods.i.!PARAMS.0 then signature = signature")"
        else signature = signature", "
      end
    end
    say signature
    interpret 'exists = var("methods.'i'.!DOC")'
    if exists = 1 then say methods.i.!DOC
    if i < methods.0 then say "-"~copies(64)
  end
