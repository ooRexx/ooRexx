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
   Samples:  Person View Class			  	  	  v01-00 26May13

   Contains: classes "PersonView".

   Pre-requisites: Model-View Framework.

   Outstanding Problems:
   None.

   Changes:
   v01-00 01Oct12: First version.
          01Apr13: After ooDialog 4.2.2, Samples folder renamed to 'Extras'
                   and Support moved to within execise foldes.
                   so changes to ::Requires and ~addToConstDir needed.
          26May13: Corrected h path after re-factoring Extras folder.


------------------------------------------------------------------------------*/


.Application~addToConstDir("Extras\Person\PersonView.h")


/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  PersonView							 v01-00  01Oct12
  ----------
  A simple class that shows how to exploit the Model-View framework.
  See comments that include the string 'MFV'.

  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


::REQUIRES "ooDialog.cls"
::REQUIRES "Support\RcView.rex"

::CLASS 'PersonView' SUBCLASS 'RcView' PUBLIC

  /*----------------------------------------------------------------------------
    newInstance - creates an instance of the View:
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD newInstance CLASS
    use strict arg modelId, rootDlg	-- MVF provides id of this view's Model;
    					-- not used in this sample.
    dlg = .PersonView~new("Extras\Person\PersonView.rc", "IDD_DIALOG1")
    dlg~activate(modelId, rootDlg)
    return dlg				-- required by MVF.


    /*----------------------------------------------------------------------------
    init - initialises the dialog
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD init
    forward class (super) continue
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


  /*----------------------------------------------------------------------------
    activate - Model's data is provided by the superclass.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD activate UNGUARDED
    expose personData
    use strict arg modelId, rootDlg
    forward class (super) continue	-- Required by MVF to get this View's
    					--   data from its Model component.
    personData = RESULT			-- personData returned by superclass
    self~popupAsChild(rootDlg, "SHOWTOP")
    return
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


  /*----------------------------------------------------------------------------
    initDialog - invoked automatically after the dialog has been created.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD initDialog
    expose personData ecPersNo ecPersFamilyName ecPersFirstName ecPersDOB ecPersPosition ecPersSalary
    --say "PersonView-initDialog-01."
    ecPersNo         = self~newEdit("IDC_PERS_NO")
    ecPersFamilyName = self~NewEdit("IDC_PERS_FAMILYNAME")
    ecPersFirstName  = self~newEdit("IDC_PERS_FIRSTNAME")
    ecPersDob        = self~newEdit("IDC_PERS_DOB")
    ecPersPosition   = self~newEdit("IDC_PERS_POSITION")
    ecPersSalary     = self~newEdit("IDC_PERS_SALARY")
    self~setMyData(personData)		-- Note: cannor use 'setData' as this
    					-- would conflict with ooDialog's
    					-- setData method.
    --self~offset:super			-- offsetting logic is in the superclass.
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


  /*----------------------------------------------------------------------------
    setData - sets (or "populates") controls with data provided in the
              method's argument.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD setMyData
    expose ecPersNo ecPersFamilyName ecPersFirstName ecPersDOB ecPersPosition ecPersSalary
    use arg personData
    ecPersNo~setText(        personData["number"])
    ecPersFamilyName~setText(personData["familyName"])
    ecPersFirstName~setText( personData["firstName"])
    ecPersDOB~setText(       personData["dob"])
    ecPersPosition~setText(  personData["jobDescr"])
    ecPersSalary~setText(    personData["baseSalary"])
    return

/*============================================================================*/
