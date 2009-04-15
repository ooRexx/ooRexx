/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
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
/* ADSI Sample 7:                                                  */
/*                                                                 */
/* Create a group and several users in it.                         */
/*                                                                 */
/*******************************************************************/

ComputerName = value("COMPUTERNAME",,"ENVIRONMENT")  -- get ComputerName

computer = .OLEObject~GetObject("WinNT://"||ComputerName)

/* create a new group */
newGroup = computer~Create("group", "REXX-TestGroup")
newGroup~Description = "A test group created with REXX"
newGroup~SetInfo

/* make sure the information in the object cache is up-to-date */
newGroup~GetInfo

say "Created new group" newGroup~Name
say "Description:" newGroup~Description

say
say "Creating 15 users in this group:"
say "User01..User15 with passwords demo01..demo15"
/* create several new users */
do i = 1 to 15
  /* create name and other information */
  userName = "User"right(i,2,'0')
  userFullName = "Demo User Number" i
  userDescription = "A demo user that was created with REXX"
  userPassword = "demo"right(i,2,'0')

  newUser = computer~Create("user", userName)
  newUser~FullName = userFullName
  newUser~Description = userDescription
  newUser~SetPassword(userPassword)
  newUser~SetInfo
  newGroup~Add(newUser~ADsPath)
end

say "done"

return 0
