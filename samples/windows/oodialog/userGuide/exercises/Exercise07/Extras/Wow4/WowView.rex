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
/* ooDialog User Guide
   Exercise03 Part 2: The WowView component.			  v02-00 01Apr13

   Contains: 	   Classes: WowView.

   Pre-requisites:
     .local~my.idWowPicker - an object that provides a 'pickWow' method.

   Description: The view component for the "Words of Wisdom" app.

   Changes:
     v01-00 31May12: First version.
     v02-00 06Sep12: Changed to use MVF.
            09Jan13: 'say's commented out.
            01Apr13: After ooDialog 4.2.2, Support folder moved to exercise
                     folder, so change to ::Requires needed.

------------------------------------------------------------------------------*/

/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  WowView							  v02-00 06Sep12
  -------
  A class that defines the User Interface for the Wow application.

  Changes:
    v01-00 31May12: First version.
    v02-00 06Sep12: Second version - uses the Model-View Framework.
           09Jan13: 'say's commented-out.
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::REQUIRES "ooDialog.cls"
::REQUIRES "..\Exercise07\Support\UdView.rex"

::CLASS 'WowView' SUBCLASS UdView PUBLIC				-- MVF

  /*----------------------------------------------------------------------------
    newInstance - invoked to create the View instance			-- MVF
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD newInstance CLASS
    use strict arg idModel, rootDlg		-- MVF: idModel is the object ref of the Model instance.
    dlg = self~new()
    dlg~activate(idModel,rootDlg)		-- MVF: idModel required by MVF
    return dlg					-- MVF: must return view id.

  /*----------------------------------------------------------------------------
    init - initialises the dialog
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD init
    forward class (super) continue
    self~create(30, 30, 257, 123, "Words of Wisdom", "CENTER")
    return
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*----------------------------------------------------------------------------
    defineDialog - defines the "Words of Wisdom" controls
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD defineDialog		-- Invoked automatically by ooDialog.
    self~createPushButton(901, 142, 99, 50, 14, "DEFAULT", "More wisdom", OkClicked)
    self~createPushButton(IDCANCEL, 197, 99, 50, 14, ,"Cancel")
    self~createStaticText(101, 40, 40, 200, 40, , "Click 'More wisdom'")
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*----------------------------------------------------------------------------
    initDialog - invoked automatically after the dialog has been created.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD initDialog
    -- expose newText							-- v01-00
    expose modelData newText						-- MVF
    newText = self~newStatic(101)
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*----------------------------------------------------------------------------
    activate - gets id for wowPicker, shows the dialog.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD activate UNGUARDED
    --expose wowPicker							-- v01-00
    expose rootDlg modelData idModel					-- MVF
    --wowPicker = .local~my.idWowPicker					-- v01-00
    use strict arg idModel, rootDlg					-- MVF
    say "WowView-activate-01: idModel, rootDlg" idModel||"," rootDlg
    forward class (super) continue		-- MVF: gets Model's data
    modelData = RESULT				-- MVF: Model's data returned by super
    --self~execute("SHOWTOP", IDI_DLG_OOREXX)
    self~popupAsChild(rootDlg,"SHOWTOP", IDI_DLG_OOREXX)
    return
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*----------------------------------------------------------------------------
    okClicked - Actions the "More wisdom" control
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD okClicked
    -- expose wowPicker newText						-- v01-00
    expose idModel newText						-- MVF
    --wow = wowPicker~pickWow						-- v01-00
    wow = idModel~pickWow
    --newText~setText(wow)						-- v01-00
    --say "WowView-okClicked-01: newText, modelData =" newText wow
    newText~setText(wow)						-- MVF
    return
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*============================================================================*/
