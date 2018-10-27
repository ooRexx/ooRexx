/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2018 Rexx Language Association. All rights reserved.    */
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
/*
 * Note: this program uses the public routine, locate(), to get the full path
 * name to the directory this source code file is located. In places, the
 * variable holding this value has been callously abbreviated to 'sd' which
 * stands for source directory.
 *
 ****************************************************************************/

  srcDir = locate()

  -- Use the global .constDir for symbolic IDs
  .application~useGlobalConstDir('O', srcDir'rc\ldvideo.h')

 logfile = srcDir'oovideo.log'
 a.IDC_EDIT_TAPE_NO = "10000"
 a.IDC_EDIT_TAPE_LABEL = "Actionfilms 1"
 a.IDC_EDIT_FILM1 = "Gone with the Wind"
 a.IDC_EDIT_FILM2 = "True Lies"
 a.IDC_EDIT_FILM3 = "Rambo 4"
 a.IDC_EDIT_FILM4  = ""
 a.IDC_CB_LOCATION = "Unknown"
 a.IDC_LB_LENT_TO = "Maria Shell"
 do i = .constDir[IDC_CK_LONGPLAY] to .constDir[IDC_RB_C240]
    a.i = 0
 end
 a.IDC_RB_C180 = 1
 a.IDC_CK_HIFI = 1

 dlg = .MyDialog~new(srcDir"rc\ldvideo.rc", IDD_VIDEO_DLG, A., , "CONNECTBUTTONS")

 if dlg~initcode > 0 then do
    call errorDialog "Couldn't load the Video dialog"
    return 99
 end
 else if dlg~execute("SHOWTOP") = .MyDialog~IDOK then do
    o=.stream~new(logfile)
    o~open('write replace')
    o~lineout("OOVideo - Data:")
    o~lineout("Nr      :" a.IDC_EDIT_TAPE_NO)
    o~lineout("Titel   :" a.IDC_EDIT_TAPE_LABEL)
    o~lineout("Film 1  :" a.IDC_EDIT_FILM1)
    o~lineout("Film 2  :" a.IDC_EDIT_FILM2)
    o~lineout("Film 3  :" a.IDC_EDIT_FILM3)
    o~lineout("Film 4  :" a.IDC_EDIT_FILM4)
    o~lineout("Lent to :" a.IDC_LB_LENT_TO)
    o~lineout("Location:" a.IDC_CB_LOCATION)
    o~lineout("Longplay:" a.IDC_CK_LONGPLAY)
    o~lineout("Hifi    :" a.IDC_CK_HIFI)
    o~lineout("Protect :" a.IDC_CK_WRITEPROTECT)
    if (a.IDC_RB_C120 = 1) then o~lineout("Tape    : C120")
    if (a.IDC_RB_C180 = 1) then o~lineout("Tape    : C180")
    if (a.IDC_RB_C240 = 1) then o~lineout("Tape    : C240")
    o~close()
    'type "'logfile'"'
 end

 return

/*--------------------------------- requires -------------------------*/

::requires "ooDialog.cls"
::requires "samplesSetup.rex"

/*--------------------------------- dialog class --------------------*/

::class 'MyDialog' subclass RcDialog

::method initDialog
   expose sd

   sd = locate()

   cb = self~newComboBox(IDC_CB_LOCATION)
   cb~add("Drawer 1")
   cb~add("Drawer 2")
   cb~add("Behind the door")
   cb~add("Under the bed")
   cb~add("On the floor")
   cb~add("Unknown")

   lb = self~newListBox(IDC_LB_LENT_TO)
   lb~add("Hans-Maria Baier")
   lb~add("John Smith")
   lb~add("Peter Fonda")
   lb~add("Maria Shell")
   lb~add("Mike Miller")

   return 0

::method validate
   expose sd
   tst = self~newEdit(IDC_EDIT_TAPE_NO)~getText
   if tst <> "" then do
      call Play sd"wav\take.wav"
      return .true
   end
   else do
      msg = "The tape number can not be blank.  Use cancel to quit, if needed."
      title = 'Video Archive Database Warning'
      ret = MessageDialog(msg, self~hwnd, title, 'OK', 'WARNING')
   end

  return .false

::method cancel unguarded
   expose sd

   ret = Play(sd"wav\cancel.wav", yes)

   msg   = "Do you really want to cancel?       "
   title = 'Exiting Video Database Application'

   ret = MessageDialog(msg, self~hwnd, title, 'YESNO', 'QUESTION')

   if ret  == self~IDYES then return self~cancel:super
   else return 0

::method search unguarded

   msg = "Entry not found in Video Archive database."
   title = 'Video Archive Database Warning'
   ret = MessageDialog(msg, self~hwnd, title, 'OK', 'WARNING')

   return 0

