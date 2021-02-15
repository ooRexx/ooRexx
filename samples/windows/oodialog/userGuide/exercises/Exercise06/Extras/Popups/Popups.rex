/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2011-2014 Rexx Language Association. All rights reserved.    */
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
   Samples\Popups
   Popups.rex 							  v01-00 07Jun12

   The four dialogs in this file illustrate how popups can be tested in
   stand-alone mode with a single code base. If a given dialog pops up another
   dialog, then this is part of the "stand-alone" operation of the
   first dialog.

   Associated files: None.

   Invocation:  popups [runType]

   "runType" controls which dialog to surface in "standalone" mode as follows:
      "p" for parent, "c" for child, or "g" for grandchild, "gg" for
      greatgrandchild. Of course "standalone mode" has no meaning for the
      parent dialog, as when run it's always the root.

   The default is runtype = Parent.

   Changes:
     v01-00 07Jun12: First Version

------------------------------------------------------------------------------*/

parse upper arg runtype
progName = "Popups"
if runtype = "?" then do
  say
  say "+-----------------------------------------------------------------------+"
  say "| Demonstration of Popups, four generations, where any generation       |"
  say "| can be run 'standalone' - that is, as the 'parent' of other 'younger' |"
  say "| dialogs.                                                              |"
  say "|                                                                       |"
  say "| Syntax: 'Popups [runType]'                                            |"
  say "|                                                                       |"
  say "| - runType: Defines the 'root' dialog as follows:                      |"
  say "|   'p/P' or null for Parent,  'c/C'   for Child,                       |"
  say "|   'g/G' for GrandChild,      'gg/GG' for GreatGrandChild.             |"
  say "|   Default is 'P'.                                                     |"
  say "+-----------------------------------------------------------------------+"
  exit
end

-- Work out which of the four possible formats of parameters applies:
if runtype  = ""  then runtype = "P"

-- Launch First Dialog:
select
  when runtype = "P"  then .ParentDialog~newInstance(offset)
  when runtype = "C" then .ChildDialog~newInstance("SA",offset)
  when runtype = "G" then .GrandChildDialog~newInstance("SA",offset)
  when runtype = "GG" then .GreatGrandChildDialog~newInstance("SA",offset)
  otherwise do
    say "Parameter Error. Run '"||progName||" ?' for parameter values."
    exit
  end
end

::REQUIRES "ooDialog.cls"


/*---------------------------------------------------------------------------*/

::CLASS 'ParentDialog' SUBCLASS UserDialog

  ::METHOD newInstance CLASS
    dlg = self~new
    dlg~activate()


  ::METHOD init
    forward class (super) continue
    self~create(30, 30, 257, 123, "Parent Dialog for Popups", "CENTER")


  ::METHOD defineDialog
    self~createPushButton(901, 142, 99, 50, 14, "DEFAULT", "Pop Up Child", popup)


  ::METHOD activate UNGUARDED
    self~execute("SHOWTOP")


  ::METHOD popup UNGUARDED
    .ChildDialog~newInstance(self)

/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/

::CLASS 'ChildDialog' SUBCLASS UserDialog

  ::METHOD newInstance CLASS
    use arg rootDlg
    dlg = self~new
    dlg~activate(rootDlg)


  ::METHOD init
    forward class (super) continue
    self~create(30, 30, 257, 123, "Child Dialog", "CENTER")


  ::METHOD defineDialog
    self~createPushButton(901, 142, 99, 100, 14, "DEFAULT", "Pop Up Grandchild", popup)


  ::METHOD activate UNGUARDED
    expose rootDlg
    use arg rootDlg
    if rootDlg = "SA" then do
      rootDlg = self
      self~execute("SHOWTOP")
    end
    else self~popupAsChild(rootDlg, "SHOWTOP")


  ::METHOD popup UNGUARDED
    expose rootDlg
    .GrandChildDialog~newInstance(rootDlg)

/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/

::CLASS 'GrandChildDialog' SUBCLASS UserDialog

  ::METHOD newInstance CLASS
    use arg rootDlg
    dlg = self~new
    dlg~activate(rootDlg)


  ::METHOD init
    forward class (super) continue
    self~create(30, 30, 257, 123, "Grandchild", "CENTER")


  ::METHOD defineDialog
    self~createPushButton(901, 142, 99, 100, 14, "DEFAULT", "Pop Up GreatGrandchild", popup)


  ::METHOD activate
    expose rootDlg
    use arg rootDlg
    if rootDlg = "SA" then do
      rootDlg = self
      self~execute("SHOWTOP")
    end
    else self~popupAsChild(rootDlg, "SHOWTOP")


  ::METHOD popup UNGUARDED
    expose rootDlg
    .GreatGrandChildDialog~newInstance(rootDlg)

/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/

::CLASS 'GreatGrandChildDialog' SUBCLASS UserDialog

  ::METHOD newInstance CLASS
    use arg rootDlg
    dlg = self~new
    dlg~activate(rootDlg)


  ::METHOD init
    forward class (super) continue
    self~create(30, 30, 257, 123, "GreatGrandchild", "CENTER")


  ::METHOD activate
    use arg rootDlg
    if rootDlg = "SA" then self~execute("SHOWTOP")
    else self~popupAsChild(rootDlg, "SHOWTOP")

/*---------------------------------------------------------------------------*/

