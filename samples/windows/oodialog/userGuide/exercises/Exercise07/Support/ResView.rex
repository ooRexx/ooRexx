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
/* ooDialog User Guide - Exercise07

   Support - ResView						 v01-00  11Jan13
   ----------------
   A simple superclass class for the Model-View framework.
   Code is idential to that in RcView.

   Versions:
   v01-00 21Aug12: First version.
          11Jan13: Comment-out say instructions.

  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

  --say "ResView."
::REQUIRES "ooDialog.cls"
--::REQUIRES "ObjectMgr.rex"

/*============================================================================*/

::CLASS 'ResView' SUBCLASS ResDialog PUBLIC

  --::ATTRIBUTE offsetParentDlg
  ::ATTRIBUTE viewMgr


  /*----------------------------------------------------------------------------
    init - initialises the dialog
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD init
    expose objectMgr
    --say "ResView-init-01."
    forward class (super) continue
    objectMgr = .local~my.ObjectMgr	-- Needed to clear up when dialog closed.
    self~viewMgr = .local~myViewMgr
    return
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


  /*----------------------------------------------------------------------------
    activate - must be invoked by subclass.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD activate UNGUARDED
    expose viewClass viewInstance
    use arg modelId
    -- Get View Instance name and View Class for tidy-up when dialog is closed.
    --say ".ResView~activate-01: class = " viewClass
    viewInstance = self~identityHash
    dlgName = self~objectName
    --say ".ResView~activate-02: dlgName = " dlgName
    parse var dlgName "a " viewClass
    --say ".ResView~activate-03: class name = '"||viewClass||"'"
    --say ".ResView-activate-04: viewInstance =" viewInstance
    --say ".ResView-activate-05: modelId =" modelId
    modelData = modelId~query
    return modelData
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*----------------------------------------------------------------------------
    loadList - must be invoked by subclass.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  --::METHOD loadList Wait till check out how do ShowModel for List.
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


  /*----------------------------------------------------------------------------
    leaving - invoked by ooDialog when a dialog closes.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD leaving UNGUARDED
    expose objectMgr viewClass viewInstance
    --say "RcView-leaving-01. viewClass =" viewClass "viewInstance =" viewInstance
    objectMgr~removeView(viewClass, viewInstance)
    -- Note - we do not remove the Model. Should we? If so, not from here!
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


  /*----------------------------------------------------------------------------
    Popup Offsets
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    setOffsetParent - set the paprent dialog id for later offsetting of a child
                      dialog.                                   	      */
  ::METHOD setOffsetParent
    use strict arg parentDlg
    viewMgr = .local~my.ViewMgr
    viewMgr~parentOffsetDlg = self

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    offset - offsets a "child" dialog from it "parent" dialog (i.e. the dialog
             from which the child is "popped up"). 			      */
  ::METHOD offset
    offset    = .local~my.ViewMgr~dlgOffset
    parentDlg = .local~my.ViewMgr~parentOffsetDlg
    popupPos  = parentDlg~getRealPos
    popupPos~incr(offset,offset)
    self~moveTo(popupPos, "SHOWWINDOW")
    self~ensureVisible()

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*----------------------------------------------------------------------------
    initDialog - invokes offset.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD initDialog
    self~offset
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


/*============================================================================*/
