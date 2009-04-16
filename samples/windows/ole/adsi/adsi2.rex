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
/* ADSI Sample 2:                                                  */
/*                                                                 */
/* Get a users' full name and change it.                           */
/*                                                                 */
/*******************************************************************/

ComputerName = value("COMPUTERNAME",,"ENVIRONMENT")     -- get ComputerName
UserID       = value("USERNAME",,"ENVIRONMENT" )        -- get UserName

userObject = .OLEObject~GetObject("WinNT://"||ComputerName||"/"||UserID||",user")

/* using the object property */
say "The full name for" UserID "is" userObject~FullName

/* using the standard get method for ADSI objects */
say "The full name for" UserID "is" userObject~Get("FullName")

say "Would you like to rename the full name (y/n)?"
pull answer

if answer = "Y" then do
  say "New full name:"
  parse pull answer

  /* set the property                                                      */
  /* as an alternative, the property can also be set with the standard put */
  /* method of ADSI objects:                                               */
  /* userObject~Put("FullName",answer)                                     */
  userObject~FullName=answer

  /* because properties are cached to avoid network calls, changing the    */
  /* properties of an object will only affect the cache at first.          */
  /* the object gets updated with the SetInfo method:                      */
  userObject~SetInfo

  say "updated the full name for" UserID
end

return 0
