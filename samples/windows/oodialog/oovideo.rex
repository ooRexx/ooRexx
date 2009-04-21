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
/****************************************************************************/
/* Name: OOVIDEO.REX                                                        */
/* Type: Object REXX Script                                                 */
/*                                                                          */
/* Description: Video archive                                               */
/*                                                                          */
/****************************************************************************/

 curdir = directory()
 parse source . . me
 mydir = me~left(me~lastpos('\')-1)              /* where is code     */
 mydir = directory(mydir)                        /* current is "my"   */

 logfile = 'oovideo.log'
 a.1001 = "10000"
 a.1002 = "Actionfilms 1"
 a.1003 = "Gone with the Wind"
 a.1004 = "True Lies"
 a.1005 = "Rambo 4"
 a.1006  = ""
 a.1007 = "Unknown"
 a.1008 = "Maria Shell"
 do i = 1009 to 1014
    a.i = 0
 end
 a.1013 = 1
 a.1010 = 1

 dlg = .mydialog~new(A.)
 if dlg~initcode > 0 then call ErrorMessage "Couldn't load the Video dialog"
 else if dlg~execute("SHOWTOP") = 1 then
 do
    o=.stream~new(logfile)
    o~open('write replace')
    o~lineout("OOVideo - Data:")
    o~lineout("Nr      :" a.1001)
    o~lineout("Titel   :" a.1002)
    o~lineout("Film 1  :" a.1003)
    o~lineout("Film 2  :" a.1004)
    o~lineout("Film 3  :" a.1005)
    o~lineout("Film 4  :" a.1006)
    o~lineout("Lent to :" a.1007)
    o~lineout("Location:" a.1008)
    o~lineout("Longplay:" a.1009)
    o~lineout("Hifi    :" a.1010)
    o~lineout("Protect :" a.1011)
    if (a.1012 = 1) then o~lineout("Tape    : C120")
    if (a.1013 = 1) then o~lineout("Tape    : C180")
    if (a.1014 = 1) then o~lineout("Tape    : C240")
    o~close()
    "type" logfile
 end

 dlg~deinstall
 ret = directory(curdir)
 return

/*--------------------------------- requires -------------------------*/

::requires "oodplain.cls"

/*--------------------------------- dialog class --------------------*/

::class mydialog subclass PlainUserDialog

::method init
   use arg st.
   self~init:super(st.)
   self~InitCode = self~load("rc\ldvideo.rc", 101, "CONNECTBUTTONS")

::method InitDialog
   self~InitDialog:super
   self~AddComboEntry(1007,"Drawer 1")
   self~AddComboEntry(1007,"Drawer 2")
   self~AddComboEntry(1007,"Behind the door")
   self~AddComboEntry(1007,"Under the bed")
   self~AddComboEntry(1007,"On the floor")
   self~AddComboEntry(1007,"Unknown")

   self~AddListEntry(1008,"Hans-Maria Baier")
   self~AddListEntry(1008,"John Smith")
   self~AddListEntry(1008,"Peter Fonda")
   self~AddListEntry(1008,"Maria Shell")
   self~AddListEntry(1008,"Mike Miller")

   return 0

::method Validate
   tst = self~GetValue(1001)
   if tst <> "" then do
      call Play "wav\take.wav"
      return 1
      end
   else do
          ret = ErrorMessage("The number can't be blank (use cancel)!")
          return 0
        end

::method CANCEL
   ret = Play("wav\cancel.wav", yes)
   self~finished = YesNoMessage("Do you really want to cancel")
   return self~finished

::method Search
   ret = ErrorMessage("Entry not found in database")
   return 1


