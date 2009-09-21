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
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* OODialog\Samples\oostddlg.rex   Standard Dialog demonstration            */
/*                                                                          */
/*--------------------------------------------------------------------------*/


say
say 'Starting standard dialog demonstration...'

/********************************************************************/
/* How to use "TimedMessage" class                                  */
/********************************************************************/

say 'Timed message box'
d = .TimedMessage~new("This is a timed message, maybe you will need it sometime",,
                      "Hello !", 3000)
d~execute

/********************************************************************/
/* How to use "InputBox" class                                      */
/********************************************************************/
d = .InputBox~new("This is an input dialog, please enter some data","InputBox")
say "Your InputBox data:" d~execute

/********************************************************************/
/* How to use "IntegerBox" class                                    */
/********************************************************************/
d = .IntegerBox~new("This is an integer dialog, please enter numerical data","IntegerBox")
say "Your IntegerBox data:" d~execute

/********************************************************************/
/* How to use "PasswordBox" class                                   */
/********************************************************************/
d = .PasswordBox~new("Please enter your password","Security")
say "Your PasswordBox data:" d~execute

/********************************************************************/
/* How to use "MultipleInputBox" class                              */
/********************************************************************/
lab.1 = "First name: "
lab.2 = "Last name: "
lab.3 = "Street and City: "
lab.4 = "Profession:"

addr.101 = "John" ; addr.102 = "Smith" ; addr.103 = ""
addr.104 = "Software Engineer"

d = .MultiInputBox~new("This is a multi input dialog, please enter the address","Your Address",lab., addr.)
if d~execute = 1 then
do
   say "Your MultiInputBox data:"
   say "->" d~FirstName     ; say "->" d~LastName
   say "->" d~StreetandCity ; say "->" d~Profession
end

/********************************************************************/
/* How to use "CheckList" class                                     */
/********************************************************************/
lst.1  = "Monday" ; lst.2  = "Tuesday" ; lst.3  = "Wednesday" ; lst.4  = "Thursday"
lst.5  = "Friday" ; lst.6  = "Saturday"; lst.7  = "Sunday"

do i = 101 to 107
   chk.i = 0
end

d = .CheckList~new("This is a checklist dialog","Checklist",lst., chk.)
if d~execute = 1 then do
   say "Your CheckList data:"
     do i = 101 to 107
        a = i-100
        if chk.i = 1 then say "->" lst.a
     end
   end

/********************************************************************/
/* How to use "SingleSelection" class                               */
/********************************************************************/
mon.1 = "January" ; mon.2 = "February" ; mon.3 = "March"   ; mon.4 = "April"
mon.5 = "May"      ;mon.6 = "June"     ; mon.7 = "July"    ; mon.8 = "August"
mon.9 = "September";mon.10= "October"  ; mon.11= "November"; mon.12= "December"

d = .SingleSelection~new("This is a single selection dialog","Single Selection",mon.,6,,6)
d~focusItem(106)
s = d~execute
if s>0 then say "Your SingleSelection data:" mon.s

/********************************************************************/
/* How to use "ListChoice" class                                    */
/********************************************************************/
lst.1  = "Monday" ; lst.2  = "Tuesday" ; lst.3  = "Wednesday" ; lst.4  = "Thursday"
lst.5  = "Friday" ; lst.6  = "Saturday"; lst.7  = "Sunday"

d = .ListChoice~new("This is a listchoice dialog, please select the day","ListChoice",lst.)
s = d~execute
say "Your ListChoice data:" s

/********************************************************************/
/* How to use "MultipleListChoice" class                            */
/********************************************************************/
lst.1  = "Monday" ; lst.2  = "Tuesday" ; lst.3  = "Wednesday" ; lst.4  = "Thursday"
lst.5  = "Friday" ; lst.6  = "Saturday"; lst.7  = "Sunday"

d = .MultiListChoice~new("This is a multiple list choice dialog, please select the days",,
                         "MultipleListChoice",lst.)
s = d~execute
say "Your MultiListChoice data lines:" s
if s <> 0 then do while s <> ""
  parse var s res s
  if res <> "" then say "->" lst.res
end

say 'End of standard dialog demonstration...'

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

::requires "OODPLAIN.cls"

