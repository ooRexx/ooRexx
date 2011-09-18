/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2011-2011 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                                         */
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

/* ooDialog User Guide
   Samples\TestPopups
   TestPopups.rex 						  v01-00 18Sep11

   The four dialogs in this file illustrate how popups can be tested in
   stand-alone mode with a single code base. If a given popup pops up another
   dialog, then this is treated as part of the "stand-alone" oparation of the
   first dialog.

   Associated files: None.

   Invocation:
   A single parameter - "runType" - controls which dialog to surface in
   "standalone" mode as follows:
      "c" for child, or "g" for grandchild, "gg" for greatgrandchild.
   No parameter surfaces the parent dialog - that is, the "application".

------------------------------------------------------------------------------*/

parse arg runtype

if runtype = ""  then .ParentDialog~newInstance
else if runtype = "c" then .ChildDialog~newInstance("SA")
  else if runtype = "g" then .GrandChildDialog~newInstance("SA")
    else if runtype = "gg" then .GreatGrandChildDialog~newInstance("SA")
      else say "Bad parameter - try again..."

::REQUIRES "ooDialog.cls"


/*---------------------------------------------------------------------------*/

::CLASS 'ParentDialog' SUBCLASS UserDialog
  ::METHOD newInstance CLASS
    dlg = self~new
    dlg~activate


  ::METHOD init
    say "Parent-init."
    forward class (super) continue
    self~create(30, 30, 257, 123, "Parent Dialog for Popups", "CENTER")


  ::METHOD defineDialog		-- Invoked automatically by ooDialog.
    say "Parent-defineDialog."
    self~createPushButton(901, 142, 99, 50, 14, "DEFAULT", "Pop Up Child", ok)


  ::METHOD activate UNGUARDED
    say "Parent-activate."
    self~execute("SHOWTOP")


  ::METHOD ok UNGUARDED
    .ChildDialog~newInstance(self)


/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/

::CLASS 'ChildDialog' SUBCLASS UserDialog

  --::ATTRIBUTE standAlone CLASS

  ::METHOD newInstance CLASS
    use arg rootDlg
    dlg = self~new
    dlg~activate(rootDlg)

  ::METHOD init
    say "ChildDialog-Init."
    forward class (super) continue
    self~create(30, 30, 257, 123, "Child Dialog", "CENTER")

  ::METHOD defineDialog		-- Invoked automatically by ooDialog.
    say "ChildDialog-defineDialog."
    self~createPushButton(901, 142, 99, 100, 14, "DEFAULT", "Pop Up Grandchild", ok)

  ::METHOD activate UNGUARDED
    expose rootDlg
    use arg rootDlg
    say "ChildDialog-activate: rootDlg =" rootDlg
    --trace i
    if rootDlg = "SA" then do
      rootDlg = self
      self~execute("SHOWTOP")
    end
    else self~popupAsChild(rootDlg, "SHOWTOP")

  ::METHOD ok UNGUARDED
    expose rootDlg
    say "ChildDialog-ok."
    .GrandChildDialog~newInstance(rootDlg)

/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/

::CLASS 'GrandChildDialog' SUBCLASS UserDialog

  ::METHOD newInstance CLASS
    use arg rootDlg
    say ".GrandchildDialog-newInstance: rootDlg = " rootDlg
    dlg = self~new
    dlg~activate(rootDlg)


  ::METHOD init
    say "GrandchildDialog-init."
    forward class (super) continue
    self~create(30, 30, 257, 123, "Grandchild", "CENTER")


  ::METHOD defineDialog
    self~createPushButton(901, 142, 99, 100, 14, "DEFAULT", "Pop Up GreatGrandchild", ok)


  ::METHOD activate
    expose rootDlg
    use arg rootDlg
    say "GrandchildDialog-activate: rootDlg =" rootDlg
    if rootDlg = "SA" then do
      rootDlg = self
      self~execute("SHOWTOP")
    end
    else self~popupAsChild(rootDlg, "SHOWTOP")


  ::METHOD ok UNGUARDED
    expose rootDlg
    say "GrandChildChildDialog-ok."
    .GreatGrandChildDialog~newInstance(rootDlg)

/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/

::CLASS 'GreatGrandChildDialog' SUBCLASS UserDialog

  ::METHOD newInstance CLASS
    use arg rootDlg
    say ".GreatGrandchildDialog-newInstance: rootDlg = " rootDlg
    dlg = self~new
    dlg~activate(rootDlg)


  ::METHOD init
    say "GreatGrandchildDialog-init."
    forward class (super) continue
    self~create(30, 30, 257, 123, "GreatGrandchild", "CENTER")

  ::METHOD activate
    use arg rootDlg
    say "GreatGrandchildDialog-activate: rootDlg =" rootDlg
    if rootDlg = "SA" then self~execute("SHOWTOP")
    else self~popupAsChild(rootDlg, "SHOWTOP")

/*---------------------------------------------------------------------------*/