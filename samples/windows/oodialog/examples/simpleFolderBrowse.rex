/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2013-2014 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                         */
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

/**
 * A quick example of the SimpleFolderBrowse class.  This class only has one
 * method: getFolder() and it is a class method.
 *
 * It is very easy to use, you just invoke the getFolder() method and the folder
 * picked by the user is returned.  When the user picks a folder the fully
 * qualified path name is returned.  If the user cancels, the empty string is
 * returned.
 *
 * It is possible, depending on how the dialog is set up, for the user to pick a
 * virtual folder that has no file system path.  In that case, .nil is returned.
 *
 */

    -- Set up some symbolic IDs and then put up our dialog.
    .application~setDefaults('O', , .false)
    .constDir[IDC_PB_BROWSE]      = 100
    .constDir[IDC_ST_RESULTS]     = 101
    .constDir[IDC_EDIT]           = 102
    .constDir[IDC_RB_USE_OPTIONS] = 103
    .constDir[IDC_RB_NO_OPTIONS]  = 104

    dlg = .BrowseDialog~new
    dlg~execute("SHOWTOP", IDI_DLG_OOREXX)

    return 0

::requires "ooDialog.cls"

::class 'BrowseDialog' subclass UserDialog

::method init
    forward class (super) continue

    self~create(30, 30, 257, 123, "Simple Folder Browse Example", "CENTER")


/** defineDialog()
 *
 *  Create a simple dialog template.
 */
::method defineDialog

    self~createStaticText(IDC_ST_RESULTS, 10, 10, 30, 11, , "Results:")
    self~createEdit(IDC_EDIT, 10, 24, 237, 11, 'AUTOSCROLLH')
    self~createRadioButton(IDC_RB_USE_OPTIONS, 10, 60, 90, 14, "AUTO", 'Use optional arguments')
    self~createRadioButton(IDC_RB_NO_OPTIONS, 10, 79, 90, 14, "AUTO", 'No optional arguments')
    self~createPushButton(IDC_PB_BROWSE, 10, 99, 50, 14, "DEFAULT", "Browse", onBrowse)
    self~createPushButton(IDCANCEL, 197, 99, 50, 14, , "Done")


/** onBrowse()
 *
 * When the user clicks on the 'Browse' push button, we put up the browse for
 * folder dialog and then report the results in the edit control.
 *
 * If the 'Use opitonal arguments' radio button is checked we use all the
 * possible optional arugments.  Otherwise we just put up the browse for folder
 * dialog 'plain.'  I.e., using all defaults.
 */
::method onBrowse unguarded
    expose rbUse

    if rbUse~checked then do
        title  = 'Browsing for folders the REXXish way'
        banner = 'Instructions could go here ...'
        hint   = 'Pick any folder you wish, or some other hint'

        startDir = value("REXX_HOME", , 'ENVIRONMENT')
        if startDir == '' then startDir = 'C:\Program Files'

        folder = .SimpleFolderBrowse~getFolder(title, banner, hint, startDir, 'CSIDL_DRIVES', self)
    end
    else do
        folder = .SimpleFolderBrowse~getFolder
    end

    edit = self~newEdit(IDC_EDIT)

    select
        when folder == .nil then text = 'The user picked a virtual folder'
        when folder == '' then text = 'The user canceled'
        otherwise text = 'The user picked:' folder
    end
    -- End select

    edit~setText(text)


/** initDialog()
 *
 * Simple init dialog method.  We just use it to check one of the radio buttons.
 */
::method initDialog
    expose rbUse

    rbUse = self~newRadioButton(IDC_RB_USE_OPTIONS)
    rbUse~check
