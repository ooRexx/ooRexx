/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2011-2012 Rexx Language Association. All rights reserved.    */
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
   TestPopups.rex 						  v01-01 23Feb12

   The four dialogs in this file illustrate how popups can be tested in
   stand-alone mode with a single code base. If a given popup pops up another
   dialog, then this is treated as part of the "stand-alone" operation of the
   first dialog.

   Associated files: None.

   Invocation:  testpopups [runType] [offset]

   "runType" controls which dialog to surface in "standalone" mode as follows:
      "p" for parent, "c" for child, or "g" for grandchild, "gg" for
      greatgrandchild. Parent is the default (and of course "standalone mode" has
      no meaning for the parent dialog, as when run it's always the root dialog.
   "offset" is a number that governs the extent to which a descendant dialog
      is visually offset from the ancestor that surfaced it.
   The default is runtype = Parent and offset = 0.

   Change Log:
   v01-00 18Sep11: First Version.
   v01-01 28Feb12: Inserted dialog offset code, so dialogs do not come up
                   over each other. Also, corrected a typo in a comment.

------------------------------------------------------------------------------*/

parse upper arg runtype offset
progName = "TestPopups"
if runtype = "?" then do
  say
  say "+-----------------------------------------------------------------------+"
  say "| Demonstration of Popups, four generations, where any generation       |"
  say "| can be run 'standalone' - that is, as the 'parent' of other 'younger' |"
  say "| dialogs.                                                              |"
  say "|                                                                       |"
  say "| Syntax: 'TestPopups [runType] [offset]'                               |"
  say "|                                                                       |"
  say "| - runType: Defines the 'root' dialog as follows:                      |"
  say "|   'p/P' or null for Parent,  'c/C'   for Child,                       |"
  say "|   'g/G' for GrandChild,      'gg/GG' for GreatGrandChild.             |"
  say "|   Default is 'P'.                                                     |"
  say "|                                                                       |"
  say "| - offset: a number (e.g. 100). If present, visibly offsets            |"
  say "|   a descendant dialog from its immediate ancestor.                    |"
  say "|   Default is 0 - that is, no offset.                                  |"
  say "+-----------------------------------------------------------------------+"
  exit
end

-- Work out which of the four possible formats of parameters applies:
if runtype  = ""  & offset = ""  then case = 1
if runtype~datatype = "NUM" & offset = ""  then case = 2
if ("PCG"~caselessPos(runtype) >0 | runtype = "GG") & offset = ""  then case = 3
if ("PCG"~caselessPos(runtype) >0 | runtype = "GG") & offset~datatype = "NUM" then case = 4

-- Set up parameter values:
select
  when case = 1 then do
    runtype = "P"; offset = 0
  end
  when case = 2 then do
    offset = runtype; runtype = "P";
  end
  when case = 3 then do
    offset = 0
  end
  when case = 4 then nop
  otherwise do
    say "Parameter Error. Run '"||progName||" ?' for parameter values."
    exit
  end
end

-- Launch First Dialog:
select
  when runtype = "P"  then .ParentDialog~newInstance(offset)
  when runtype = "C" then .ChildDialog~newInstance("SA",offset)
  when runtype = "G" then .GrandChildDialog~newInstance("SA",offset)
  when runtype = "GG" then .GreatGrandChildDialog~newInstance("SA",offset)
  otherwise nop
end

::REQUIRES "ooDialog.cls"


/*---------------------------------------------------------------------------*/

::CLASS 'ParentDialog' SUBCLASS UserDialog
  ::METHOD newInstance CLASS
    use arg offset
    dlg = self~new
    dlg~activate(offset)


  ::METHOD init
    forward class (super) continue
    self~create(30, 30, 257, 123, "Parent Dialog for Popups", "CENTER")


  ::METHOD defineDialog		-- Invoked automatically by ooDialog.
    self~createPushButton(901, 142, 99, 50, 14, "DEFAULT", "Pop Up Child", ok)


  ::METHOD activate UNGUARDED
    expose offset
    use arg offset
    self~execute("SHOWTOP")


  ::METHOD ok UNGUARDED
    expose offset
    .ChildDialog~newInstance(self,offset)

/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/

::CLASS 'ChildDialog' SUBCLASS View --UserDialog

  ::METHOD newInstance CLASS
    use arg rootDlg, offset
    dlg = self~new
    dlg~activate(rootDlg,offset)


  ::METHOD init
    forward class (super) continue
    self~create(30, 30, 257, 123, "Child Dialog", "CENTER")


  ::METHOD defineDialog		-- Invoked automatically by ooDialog.
    self~createPushButton(901, 142, 99, 100, 14, "DEFAULT", "Pop Up Grandchild", ok)


  ::METHOD activate UNGUARDED
    expose rootDlg offset standalone
    use arg rootDlg, offset
    if rootDlg = "SA" then do
      standalone = .true
      rootDlg = self
      self~execute("SHOWTOP")
    end
    else self~popupAsChild(rootDlg, "SHOWTOP")


  ::METHOD initDialog
    expose rootDlg offset standalone
    if standalone \= .true then self~offset(rootDlg, offset)


  ::METHOD ok UNGUARDED
    expose rootDlg offset
    .GrandChildDialog~newInstance(rootDlg, self, offset)

/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/

::CLASS 'GrandChildDialog' SUBCLASS View

  ::METHOD newInstance CLASS
    use arg rootDlg, parent, offset
    if rootDlg = "SA" then offset = parent
    dlg = self~new
    dlg~activate(rootDlg, parent, offset)


  ::METHOD init
    forward class (super) continue
    self~create(30, 30, 257, 123, "Grandchild", "CENTER")


  ::METHOD defineDialog
    self~createPushButton(901, 142, 99, 100, 14, "DEFAULT", "Pop Up GreatGrandchild", ok)


  ::METHOD activate
    expose rootDlg parent offset standalone
    use arg rootDlg, parent, offset
    if rootDlg = "SA" then do
      standalone = .true
      rootDlg = self
      self~execute("SHOWTOP")
    end
    else self~popupAsChild(rootDlg, "SHOWTOP")


  ::METHOD initDialog					-- for offsetting only.
    expose rootDlg parent offset standalone
    if standalone \= .true then self~offset(parent, offset)


  ::METHOD ok UNGUARDED
    expose rootDlg offset
    .GreatGrandChildDialog~newInstance(rootDlg,self,offset)	-- self is for offsetting.

/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/

::CLASS 'GreatGrandChildDialog' SUBCLASS View

  ::METHOD newInstance CLASS
    use arg rootDlg, parent, offset
    if rootDlg = "SA" then offset = parent
    dlg = self~new
    dlg~activate(rootDlg, parent, offset)


  ::METHOD init
    forward class (super) continue
    self~create(30, 30, 257, 123, "GreatGrandchild", "CENTER")

  ::METHOD activate
    expose rootDlg parent offset
    use arg rootDlg, parent, offset
    if rootDlg = "SA" then self~execute("SHOWTOP")
    else self~popupAsChild(rootDlg, "SHOWTOP")


  ::METHOD initDialog					-- for offsetting only.
    expose rootdlg parent offset
    if rootDlg \= "SA" then self~offset(parent, offset)

/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/

::CLASS View SUBCLASS UserDialog

  ::METHOD offset
    use arg parent, offset
    parentPos = parent~getRealPos
    parentPos~incr(offset,offset)
    self~moveTo(parentPos, 'SHOWWINDOW')
    self~ensureVisible()

/*---------------------------------------------------------------------------*/