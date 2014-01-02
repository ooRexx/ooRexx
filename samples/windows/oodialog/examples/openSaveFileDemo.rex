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

/** openSaveFileDemo.rex
 *
 *  This application shows several examples of using the Common Item Dialog.
 *
 * The Common Item Dialog (formerly known as the Common File Dialog) is used in
 * two variations: the Open dialog and the Save dialog. These two dialogs share
 * most of their functionality, but each have some unique methods.
 *
 * The common item dialog implementation found in Windows Vista and later
 * provides several advantages over the implementation provided in earlier
 * versions of Windows.
 *
 *  ooDialog provides complete support for all features of the Common Item
 *  Dialog.
 *
 *  This example will only run on Vista or later.
 *
 *  Both the .OpenFileDialog and the .SaveFileDialog are subclasses of the
 *  .CommonItemDialog class.  The Rexx programmer can not instantiate a
 *  .CommonItemDialog object.  Instead the programmer picks the type of file
 *  dialog he wants, a save file or an open file dialog and instantitates that
 *  class.  Almost all of the methods for these classes come from the
 *  CommonItemDialog class and are documented in the reference manual under the
 *  CommonItemDialog section.
 *
 *  Some of the features of the Common Item Dialog are a little advanced.  How-
 *  ever, the basic every day usage is simple.  This example puts up an opening
 *  dialog that allows the user to choose from a number of different usage
 *  patterns and then shows the Common Item Dialog picked.
 *
 *  One of the features of the Common Item Dialog is that it allows its state to
 *  be saved on a per instance basis  in addition to the per process basis.
 *  This is done by generating a GUID and assigning it to the dialog before it
 *  is configured.  Then, for each dialog with the same GUID, the operating
 *  system saves its state separately.
 *
 *  To make use of this feature, the programmer would generate a single GUID and
 *  then assign the same GUID each time the file dialog was shown in the
 *  application.
 *
 *  This example makes use of that feature by assigning a different GUID for
 *  each of the XX dialogs that can be displayed from the opening dialog.
 *
 *  This simple program can be used to generate a GUID.  Run the program and
 *  then copy and paste the output on the command line in to your program.
 *
 * - - - - - - - - - Cut begin - - - - - - - - - - - - - - - - - - - - - - - -
 *  /* genGUID.rex */
 *
 *  guid = .DlgUtil~getGUID
 *  say guid
 *  return 0
 *
 *  ::requires 'ooDialog.cls'
 * - - - - - - - - - Cut end - - - - - - - - - - - - - - - - - - - - - - - - -
 *
 */
    if \ .application~requiredOS('Vista', 'openSaveFileDemo.rex') then return 99

    srcDir = locate()

    -- Set up the symbolic IDs and then put up our example dialog.
    .application~setDefaults('O', srcDir'resources\osfDialogs.h', .false)

    dlg = .CommonSaveDialog~new(srcDir'resources\osfDialogs.rc', IDD_SIMPLE_OSF_DIALOGS)
    dlg~execute("SHOWTOP", IDI_DLG_OOREXX)

    return 0

::requires "ooDialog.cls"

::class 'CommonSaveDialog' subclass RcDialog

-- Do not copy these GUIDs into your own code.  Always generate your own GUID.
::constant GUID_OPEN          '021b0a3d-cb16-4def-ab42-7fc259fd84d9'
::constant GUID_SAVE          'beff4495-fa60-468b-90c3-1dcb6fde0e78'
::constant GUID_OPEN_MULTI    'b6c25d80-198e-4baa-aad8-1249e4f15b3f'
::constant GUID_OPEN_CUSTOM   '745c45cb-3464-46fc-96cf-8628ad4bf63b'
::constant GUID_SAVE_DEFAULT  'fe2c5305-d7f9-43cd-9d7b-9c3d4d1b79be'
::constant GUID_OPEN_FOLDER   'ce5ad183-b42f-4f37-a5d6-78d1cffea2e6'


/** initDialog()
 *
 * Simple standard init dialog method.  We save references to our commonly used
 * controls, and connect the button click event for the push button.
 *
 * We also use this method to find the installed directory of ooRexx.
 */
::method initDialog
		expose rbOpen rbSave rbOpenMulti rbOpenCustom rbSaveDefault rbOpenFolder edit rexx_home

    rbOpen        = self~newRadioButton(IDC_RB_OPEN)~~check
    rbSave        = self~newRadioButton(IDC_RB_SAVE)
    rbOpenMulti   = self~newRadioButton(IDC_RB_OPEN_MULTI)
    rbOpenCustom  = self~newRadioButton(IDC_RB_OPEN_CUSTOM)
    rbSaveDefault = self~newRadioButton(IDC_RB_SAVE_DEFAULT)
    rbOpenFolder  = self~newRadioButton(IDC_RB_OPEN_FOLDER)

    edit = self~newEdit(IDC_EDIT_OSF)

    self~connectButtonEvent(IDC_PB_SHOW_OSF, 'CLICKED', onShowOSFDialog)

    rexx_home = value('REXX_HOME', , 'ENVIRONMENT')
    if rexx_home == '' then rexx_home = 'C:\Program Files'

    self~addToolTips


/** onShowOSFDialog()
 *
 * When the user clicks on the 'Show' push button, we put up the example Common
 * Item Dialog specified by the user through the radio buttons.
 *
 * Each Common Item Dialog is displayed using a separate, private, method.  To
 * look at the example code for some type of dialog, for instance the simple
 * open file dialog, examine the associated method.
 */
::method onShowOSFDialog unguarded
    expose rbOpen rbSave rbOpenMulti rbOpenCustom rbOpenFolder rbSaveDefault

    select
      when rbOpen~checked then return self~openExample
      when rbSave~checked then return self~saveExample
      when rbOpenMulti~checked then return self~openMultifilesExample
      when rbOpenCustom~checked then return self~openCustomExample
      when rbSaveDefault~checked then return self~saveDefaultExample
      when rbOpenFolder~checked then return self~openFolderExample
      otherwise return 0  -- Can not really happen
    end
    -- End select


/** openExample()
 *
 *  Displays the most simple Open File Dialog example.  We just show the dialog
 *  and get the result.  We do not use any customization or advanced features,
 *  other than slightly changing the options.
 */
::method openExample unguarded private

    -- Setting the client GUID has the operating system preserve the state for
    -- this specific open file dialog.
    ofd = .OpenFileDialog~new
    ret = ofd~setClientGuid(self~GUID_OPEN)

    -- The default flags for the Open File Dialog contain the file must exsit
    -- flag.  This prevents the user from typing in their own file name; the
    -- user can only open an exsiting file.
    --
    -- For certain applications, this behaviour may be desired.  But for this
    -- example, we want the user to be able to type in any file name, so we
    -- remove that flag.  We get the exsiting flags and set the flags to the
    -- existing flags minus the FILEMUSTEXIST keyword.
    ofd~options = ofd~options~delWord(ofd~options~wordPos('FILEMUSTEXIST'), 1)

    -- Very simple, we are all set, show the dialog and get the user's response:
    ret = ofd~show(self)

    if ret == ofd~canceled then text = 'The user canceled the open'
    else text = 'Open file:' ofd~getResult

    -- The proper use of both the .SaveFileDialog and the .OpenFileDialog is to
    -- instantiate the object, configure it, show it, and then release it.  The
    -- release() method is essential to ensuring the COM resources are properly
    -- cleaned up.
    --
    -- If release() is not called, the ooDialog framework will *attempt* to do
    -- the clean up in an uninit() method.  However, 1.) there is *no* guarentee
    -- that the interpreter will invoke the uninit() method.  2.) There is *no*
    -- guarentee the uninit() will be run on this thread.  The COM resources can
    -- *not* be cleaned up if uninit() is run on another thread than this one.
    --
    -- The only way to guarentee that the COM resources are cleaned up properly
    -- is for the programmer to invoke the release() method.
    ofd~release

    -- Have the edit box display the result.
    self~showResult(text)

    return 0


/** saveExample()
 *
 *  Displays the most simple Save File Dialog example.  We just show the dialog
 *  and get the result.  We do not use any customization or advanced features.
 */
::method saveExample unguarded private

    -- Setting the client GUID has the operating system preserve the state for
    -- this specific save file dialog.
    sfd = .SaveFileDialog~new
    ret = sfd~setClientGuid(self~GUID_SAVE)

    -- We are all set, show the dialog and get the user's response:
    ret = sfd~show(self)

    if ret == sfd~canceled then text = 'The user canceled the save'
    else text = 'Save to file:' sfd~getResult

    -- The proper use of both the .SaveFileDialog and the .OpenFileDialog is to
    -- instantiate the object, configure it, show it, and then release it.  The
    -- release() method is essential to ensuring the COM resources are properly
    -- cleaned up.
    --
    -- If release() is not called, the ooDialog framework will *attempt* to do
    -- the clean up in an uninit() method.  However, 1.) there is *no* guarentee
    -- that the interpreter will invoke the uninit() method.  2.) There is *no*
    -- guarentee the uninit() will be run on this thread.  The COM resources can
    -- *not* be cleaned up if uninit() is run on another thread than this one.
    --
    -- The only way to guarentee that the COM resources are cleaned up properly
    -- is for the programmer to invoke the release() method.
    sfd~release

    -- Have the edit box display the result.
    return self~showResult(text)

    return 0


/** openMultiFilesExample()
 *
 * Displays a basic open file dialog that allows the user to pick multiple file
 * names at one time.
 */

::method openMultiFilesExample unguarded private
    expose rexx_home

    ofd = .OpenFileDialog~new
    ret = ofd~setClientGuid(self~GUID_OPEN_MULTI)

    -- To allow the user to select multiple files, we need to add this keywod
    -- to the options:
    ofd~options = ofd~options 'ALLOWMULTISELECT'

    -- Show the dialog and get the response
    ret = ofd~show(self)

    -- If msg ends up not being .nil, we show a message box ...
    msg = .nil

    if ret == ofd~canceled then do
        text = 'The user canceled the open operation.'
    end
    else do
        files = ofd~getResults
        if files~items == 1 then do
            text = 'Open file:' files[1]
        end
        else do
            text = 'Open files (file names only):'
            msg = 'Open files (complete file names as returned):' || .endOfLine~copies(2)

            do f over files
                text ||= ' ' || fileSpec('N', f)
                msg  ||= f || .endOfLine
            end
        end
    end
    ofd~release

    -- Have the edit box display the result.
    self~showResult(text)

    if msg \== .nil then do
        title = 'Complete Open File Names'
        j = MessageDialog(msg, self~hwnd, title, 'OK', 'INFORMATION')
    end

    return 0


/** openCustomExample()
 *
 *  Displays an Open File Dialog that has some easy to use customizations.
 */
::method openCustomExample unguarded private

    ofd = .OpenFileDialog~new
    ret = ofd~setClientGuid(self~GUID_OPEN_CUSTOM)

    ofd~options = ofd~options~delWord(ofd~options~wordPos('FILEMUSTEXIST'), 1)

    -- With the Common Item Dialog you can easily change the title for the
    -- dialog, the label for the edit box, and the label for the ok button:
    ofd~setTitle("The ooRexx Project's Open File Dialog")
    ofd~setFileNameLabel('Type in, or select an existing, file to open here:')
    ofd~setOkButtonLabel('Finished')

    -- There are also a large number of options that can be set or changed.
    -- Here, we show just a few that effect the appearance of the dialog:
    opts = 'FORCEPREVIEWPANEON HIDEMRUPLACES HIDEPINNEDPLACES'
    ofd~options = ofd~options opts

    -- Okay, we are all set, show the dialog and get the user's response:
    ret = ofd~show(self)

    if ret == ofd~canceled then text = 'The user canceled the open'
    else text = 'Open file:' ofd~getResult

    ofd~release

    -- Have the edit box display the result.
    return self~showResult(text)


/** saveDefaultExample()
 *
 * Displays a save file dialog that uses file type filters and sets the initial
 * file name and directory location.
 */
::method saveDefaultExample unguarded private
    expose rexx_home

    sfd = .SaveFileDialog~new
    ret = sfd~setClientGuid(self~GUID_SAVE_DEFAULT)

    -- Add a file types filter.  To do this you use an array containing the
    -- filter values.  Each single filter consists of 2 parts, the description
    -- and the extension, so the array must contain an even number of items.
    filter = .array~of('Rexx Program File', '*.rex', 'Rexx Class File', '*.cls', 'Text File', '*.txt')
    sfd~setFileTypes(filter)

    -- Set the initial filter selected.  This is done by using the index for
    -- the filter in the array.  Each filter consists of a pair, so in the above
    -- we have 3 filters.  The second filter is the 'Rexx Class File' filter
    sfd~setFileTypeIndex(2)

    -- Set the default extension.  Note that you do not include the period.  The
    -- default extension governs whether or not an extension is added if the
    -- user leaves off the extension.  If you do not set a deault extension, no
    -- extension is added when the user does not type an extension, even if a
    -- file type filter is in effect.  When a default extension is set, if the
    -- user picks a new filter, the extension is updated automatically by the
    -- operating system.
    sfd~setDefaultExtension('cls')

    -- Set the initial save as file name.  You must provide a complete path
    -- name or an error is generated.  Likewise the file must exist.  The
    -- complete path name also serves to specifiy the directory that the save
    -- file dialog is initially opened in.
    ret = sfd~setSaveAsItem(rexx_home'\ooDialog.cls')

    -- The default options for the Save File Dialog contain the OVERWRITEPROMPT
    -- keyword.  This option causes the operating system to prompt the user if
    -- they select a save file name that is the name of an existing file.  If
    -- that behaviour is not desired it can be changed by removing the keyword
    -- as in this code.  Uncomment to see the effect:

    /* sfd~options = sfd~options~delWord(sfd~options~wordPos('OVERWRITEPROMPT'), 1) */

    -- Show the dialog and get the response
    ret = sfd~show(self)

    if ret == sfd~canceled then text = 'The user canceled the save operation.'
    else text = 'Save file:' sfd~getResult

    sfd~release

    -- Have the edit box display the result.
    return self~showResult(text)


/** openFolderExample()
 *
 * Displays a basic open file dialog.  The main point here is to demonstrate
 * how to control which directory is initially selected when the dialog opens.
 */

::method openFolderExample unguarded private
    expose rexx_home

    ofd = .OpenFileDialog~new
    ret = ofd~setClientGuid(self~GUID_OPEN_FOLDER)

    -- Normally the operating system will pick the initial directory that is
    -- selected when the open file dialog is first shown.  Usually this is the
    -- last directory the user had selected.  The setFolder() method over-rides
    -- this behaviour and forces the dialog to open with the specified directory
    -- selected.
    ofd~setFolder(rexx_home)

    -- In this example we do not change the default options.  One of the
    -- defaults is that the file must exist.  This is useful in the case where
    -- we want to ensure the user picks an exsisting file.  We do not need to
    -- check for file existence.  The operating system forces the user to pick
    -- an exsiting file or cancel.

    -- Show the dialog and get the response
    ret = ofd~show(self)

    if ret == ofd~canceled then text = 'The user canceled the open operation.'
    else text = 'Open file:' ofd~getResult

    ofd~release

    -- Have the edit box display the result.
    return self~showResult(text)


/** showResult()
 *
 * A convenience method that updates the edit box with the results for each
 * open / save file dialog.
 */
::method showResult unguarded private
    expose edit
    use strict arg text

    edit~setText(text)
    return 0


/** addToolTips()
 *
 * A convenience method to add a tool tip for each radio button.  The tool tips
 * let us shwo some text explaining the tip of Open / Save File dialog that will
 * be displayed when the specific radio button is selected.
 *
 * Notes:  A tool tip will display the text on multiple lines, and will break
 *         the text at embedded new line characters.  But - it will only do this
 *         if setMaxTipWidth() is invoked.  However, if the width is set to less
 *         than the width up to the new line, the tool tip will break the text
 *         at a word break.  So, what we do is set the max width to very large,
 *         the width of an entire text, and then the tool tip breaks the text at
 *         every new line.
 *
 *         The text is pretty long, so to ensure the user has enough time to
 *         read it all, we set the delay before the tool tip disappears to twice
 *         the default delay time.
 */
::method addToolTips private
		expose rbOpen rbSave rbOpenMulti rbOpenCustom rbSaveDefault rbOpenFolder edit

    text = "Select this Radio Button to show " || .endOfLine || -
           "a simple Open File Dialog that"    || .endOfLine || -
           "allows the user to type in a"      || .endOfLine || -
           "file name."
    s = self~getTextSizePX(text)

    ttOpen = self~createToolTip('IDC_TT_OPEN')
    ttOpen~addTool(rbOpen, text)
    ttOpen~setMaxTipWidth(s~width)

    delayTime = ttOpen~getDelayTime('AUTOPOP') * 2

    ttOpen~setDelayTime('AUTOPOP', delayTime)

    text = "Select this Radio Button to show " || .endOfLine || -
           "a simple Save File Dialog that"    || .endOfLine || -
           "allows the user to type in a"      || .endOfLine || -
           "file name."

    ttSave = self~createToolTip('IDC_TT_SAVE')
    ttSave~addTool(rbSave, text)
    ttSave~setMaxTipWidth(s~width)
    ttSave~setDelayTime('AUTOPOP', delayTime)

    text = "Select this Radio Button to show"  || .endOfLine || -
           "an Open File Dialog that allows"   || .endOfLine || -
           "the user to select multiple files" || .endOfLine || -
           "at one time."

    ttMulti = self~createToolTip('IDC_TT_OPEN_MULTI')
    ttMulti~addTool(rbOpenMulti, text)
    ttMulti~setMaxTipWidth(s~width)
    ttMulti~setDelayTime('AUTOPOP', delayTime)

    text = "Select this Radio Button to show"       || .endOfLine || -
           "an Open File Dialog that has some"      || .endOfLine || -
           "simple customizations of the apparence" || .endOfLine || -
           "of the dialog."

    ttCustom = self~createToolTip('IDC_TT_OPEN_CUSTOM')
    ttCustom~addTool(rbOpenCustom, text)
    ttCustom~setMaxTipWidth(s~width)
    ttCustom~setDelayTime('AUTOPOP', delayTime)

    text = "Select this Radio Button to show"  || .endOfLine || -
           "a Save File Dialog that sets a"    || .endOfLine || -
           "default save as file name and a"   || .endOfLine || -
           "file type filter."

    ttDefault = self~createToolTip('IDC_TT_SAVE_DEFAULT')
    ttDefault~addTool(rbSaveDefault, text)
    ttDefault~setMaxTipWidth(s~width)
    ttDefault~setDelayTime('AUTOPOP', delayTime)

    text = "Select this Radio Button to show"  || .endOfLine || -
           "an Open File Dialog that sets"    || .endOfLine || -
           "the initial direcotry that the"    || .endOfLine || -
           "dialog opens in."

    ttFolder = self~createToolTip('IDC_TT_OPEN_FOLDER')
    ttFolder~addTool(rbOpenFolder, text)
    ttFolder~setMaxTipWidth(s~width)
    ttFolder~setDelayTime('AUTOPOP', delayTime)

