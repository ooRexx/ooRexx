/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2011-2014 Rexx Language Association. All rights reserved.    */
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

/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  ViewMgr							 v01-00  13Feb13
  ----------
  A singleton component that manages Views and view-related function
  such as Popup Offsetting.

  Changes:
    v01-00 23Apr12: First version
           11Jan13: Comment-out 'say' instructions.
           13Feb13: Remove code not used.
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

--::REQUIRES "ObjectMgr.rex"

::CLASS 'ViewMgr' PUBLIC

  :: ATTRIBUTE dlgOffset	-- A single number of dialog units by which a
  				--   child dialog is offset (vertically and
  				--   horizontally) from a parent.
  :: ATTRIBUTE parentOffsetDlg	-- The dialog from which a "child" dialog is
  				--   "popped up".

  /*----------------------------------------------------------------------------
    init - initialises the ViewMgr
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD init
    --expose dlgOffset
    --say "ViewMgr-init."
    .local~my.ViewMgr = self
    self~dlgOffset = 200
    return
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


  /*----------------------------------------------------------------------------
    setPopupParent - Remembers the id of a parent dlg that is "popping up"
                     a child dialog.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD setPopupParent PUBLIC
    use strict arg parentDlg	-- the dialog id of the Parent View
    self~parentDlg = parentDlg
    say "ViewMgr-setPopupParent-01. Parent View =" parentDlg

  /*----------------------------------------------------------------------------
    offsetDlg  - Calculates the desired position of a "child" dialog given
                 the dlg id of the "parent" dialog and offsets it from the
                 parent dialog by the "dlgOffset".
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/*  ::METHOD offsetDlg PUBLIC
    use strict arg childDlg
    say "ViewMgr-popupChild-01. Parent View, Child View:" parentDlg childDlg
    parentPos = parentView~getRealPos
    childPos = incr(dlgOffset, dlgOffset)
    return
*/

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */




  /*----------------------------------------------------------------------------
    showModel - Surface a View. Uses the ObjectMgr to see if the view already
                exists, and if so, to surface it; else the ObjectMgr causes the
                required view to be created.
                *** Note - Not supported by MessageSender in Exercise07. ***
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD showModel PUBLIC
    use strict arg modelClass, modelInstance
    say "ViewMgr-showModel-01: class / instance:" modelClass "/" modelInstance
    -- Get the ObjectMgr to do the work:
    dlg = .local~my.ObjectMgr~showModel(modelClass, modelInstance)
    if dlg = .false then do
      say "ViewMgr-showModel-02: bad response from ObjectMgr."
      return .false
    end
    else do
      --say "ViewMgr-showModel-03: good response from ObjectMgr."
      return .true
    end

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*============================================================================*/
