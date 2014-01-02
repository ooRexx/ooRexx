/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2013-2014 Rexx Language Association. All rights reserved.    */
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

/**
 * An example showing usage of the BrowseForFolder class.  This example shows
 * how to use the class to browse virtual folders, in this case the Printers
 * folder.
 *
 * The BrowseForFolder object puts up the same Windows Shell browse dialog as
 * does the SimpleFolderBrowse object, but it is much more configurable.
 *
 * This example uses the browse for folder dialog to let the user select a
 * printer.
 *
 * The example puts up a regular dialog and on the push of a button shows the
 * browse for folder dialog set to browse only printers.  The user's pick is
 * is displayed in an edit box.
 */

    -- Set up some symbolic IDs and then put up our example dialog.
    .application~setDefaults('O', , .false)
    .constDir[IDC_PB_BROWSE]      = 100
    .constDir[IDC_ST_RESULTS]     = 101
    .constDir[IDC_EDIT]           = 102
    .constDir[IDC_RB_LONG_NAME] = 103
    .constDir[IDC_RB_SHORT_NAME]  = 104

    dlg = .PrintersDialog~new
    dlg~execute("SHOWTOP", IDI_DLG_OOREXX)

    return 0

::requires "ooDialog.cls"

::class 'PrintersDialog' subclass UserDialog

/** init()
 *
 *  Typical UserDialog init() method.  Initializes the super class, the starts
 *  the dialog template by creating the frame in the template.  Internally, the
 *  create() method will invoke the define() method.  When create() returns,
 *  the dialog template will be complete.
 */
::method init
  forward class (super) continue

  self~create(30, 30, 257, 123, "Browse For Printers Example", "CENTER")


/** define()
 *
 *  A typical define() method for a UserDialog.  We create a push button in the
 *  dialog template that allows the user to show the browse for folder dialog.
 *
 *  An edit control and two radio buttons are created, the edit control shows
 *  the result of the user's selection in the browse for folder dialog.  The
 *  radio button control how the selection is returned from the browse for
 *  folder dialog.
 */
::method defineDialog

    self~createStaticText(IDC_ST_RESULTS, 10, 10, 30, 11, , "Results:")
    self~createEdit(IDC_EDIT, 10, 24, 237, 11, 'AUTOSCROLLH')
    self~createRadioButton(IDC_RB_LONG_NAME, 10, 60, 90, 14, "AUTO", 'Long Display Name')
    self~createRadioButton(IDC_RB_SHORT_NAME, 10, 79, 90, 14, "AUTO", 'Short Display Name')
    self~createPushButton(IDC_PB_BROWSE, 10, 99, 65, 14, "DEFAULT", "Browse Printers", onBrowse)
    self~createPushButton(IDCANCEL, 197, 99, 50, 14, , "Done")


/** onBrowse()
 *
 * The event handler for the Browse Printers push button.  We configure a
 * BrowseForFolder object, display it, and report the user's actions in the edit
 * control.
 *
 * The Windows Shell keeps track of things in the shell using a structure called
 * an 'item ID list.'  Every thing in the shell has an item ID list.  Most
 * things in the shell have a corresponding file system path.  But not every
 * thing.  Virtual folders like the printer objects do not have a file system
 * path.
 *
 * The getFolder() method returns the file system path picked by the user.  That
 * method can not return a value for a virtual folder, so it returns .nil.  To
 * get the printer the user picks, we need to use the getItemIDList() method.
 * This method returns the handle to an item ID list.
 *
 * Currently, the programmer can not do much with an item ID list handle.  This
 * may change in future versions of ooDialog.  However, the programmer can get
 * the display name of the shell item through the item ID list, which is what
 * we do here.
 *
 * Like most, if not all, handles in Windows, the item ID list handle represents
 * a system resource that has been allocated by the OS.  When the Rexx
 * programmer is done with the item ID list, it is good practice to release the
 * handle to free up the system resources used by the item ID list.
 */
::method onBrowse unguarded
    expose rbShort edit

    -- Set title, banner, and a hint for the dialog.
    title  = 'Browse For a Printer'
    banner = 'Select the printer to use for this test.'
    hint   = 'This is only a demonstration, no printing will be done.'

    bff = .BrowseForFolder~new(title, banner, hint)

    -- Make this dialog the owner window of the browse dialog.  The root for the
    -- browse dialog will be the virtual Printers folder.  If that is not set,
    -- the user will not see the printers.  In addition, we want the user to
    -- only see the printers.  That way, if the dialog is not canceled, we are
    -- sure a printer was picked and not some random folder.
    bff~owner = self
    bff~root = 'CSIDL_PRINTERS'

    -- Set non-default options for the browse dialog.  We need the browse for
    -- printers option.  The operating system will not allow a new folder to be
    -- created in the virtual printers folder, so it disables the Make New
    -- Folder button.  It looks better to just remove the button altogether.
    bff~options = 'BROWSEFORPRINTERS NONEWFOLDERBUTTON'

    -- The getItemIDList() method is what actually puts up the browse for folder
    -- dialog.  Normally, by default, the getItemIDList() will release the COM
    -- resources used our BrowseForFolder object.  But, we still need the COM
    -- resources to be able to use the getDisplayName() method.  So, we tell the
    -- method we still need COM active by passing in .true to the method.
    --
    -- We we tell the object we still need COM, it then becomes our
    -- responsibility to release COM.  If the user cancels, we release COM and
    -- return.  Otherwise, we release COM further down in this method when we
    -- are finished with COM.
    pidl = bff~getItemIDList(.true)
    if pidl == .nil then do
        edit~setText('The user canceled.')
        bff~releaseCOM
        return 0
    end

    -- Decide what the format for the returned display name.  With no arguments,
    -- the getDisplayName() method will try to return the most complete name.
    -- For a folder with a file system path, that will be a fully qualified
    -- path name.  For a virtual folder, the name will include the parent
    -- folder(s) of the actual folder picked.
    --
    -- The second optional argument to getDisplayName() specifies the format of
    -- the returned name.  Normal display will be just the printer name.
    if rbShort~checked then do
        name = bff~getDisplayName(pidl, 'NORMALDISPLAY')
    end
    else do
        name = bff~getDisplayName(pidl, 'DESKTOPABSOLUTEEDITING')
    end

    -- We are done with pidl, release it.
    bff~releaseItemIDList(pidl)

    -- We are done with the BrowseForFolder object.  Since we told the
    -- getItemIDList() method to not release the COM resources, we need to
    -- explicitly do it ourself.
    bff~releaseCOM

    -- Determine text for the edit control ...
    if name == .nil then do
        text = 'Unexpected result. ' .DlgUtil~errMsg(.systemErrorCode)
    end
    else do
        text = 'The user picked: ' name
    end

    -- ... and set it
    edit~setText(text)

    return 0


/** initDialog()
 *
 * Simple init dialog method.  We get references to the edit control and one of
 * the two radio buttons.  Since the radio buttons are auto and there are only 2
 * of them we only need one reference to be able to tell which is checked.
 */
::method initDialog
    expose rbShort edit

    edit = self~newEdit(IDC_EDIT)
    rbShort = self~newRadioButton(IDC_RB_SHORT_NAME)
    rbShort~check
