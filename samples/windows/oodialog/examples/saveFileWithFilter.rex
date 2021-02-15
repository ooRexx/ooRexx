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

/** saveFileWithFilter.rex
 *
 *  This example shows how to use the Common Item Dialog in file save mode.
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
 *  This example shows 2 things.  1.) How to use a filter with the dialog.  This
 *  is done with the .ShellItemFilter class.  2.) How to connect event
 *  notifications.  This is done with the .CommonDialogEvents class.
 *
 *  For the purpose of the example, we use a hypothetical case that the
 *  application wants the user to pick a file to save to, but the data being
 *  saved can not be saved to file with the extension of .exe, or .dll, or .cls.
 *
 *  To achieve this a filter is used so that the save dialog does not show any
 *  files with those extensions.  To ensure the user does not type in a file
 *  name with those extensions, when the user picks a file name, the OnFileOk
 *  event is used to check the file name the user picked.
 *
 *  To give the example a little variety, a dialog is first put up to allow the
 *  user to pick some or all of the extensions to filter out and the Save File
 *  Dialog can be shown multiple times.
 *
 *  The Common Item Dialog allows its state to be saved on a per instance basis
 *  in addition to the per process basis.  This is done by generating a GUID and
 *  assigning it to the dialog before it is configured.  Then, for each dialog
 *  with the same GUID, the operating system saves its state separately.
 *
 *  To make use of this feature, the programmer would generate a single GUID and
 *  then assign the same GUID each time the file dialog was shown in the
 *  application.
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
    if \ .application~requiredOS('Vista', 'saveFileWithFilter.rex') then return 99

    srcDir = locate()

    -- Set up the symbolic IDs and then put up our example dialog.
    .application~setDefaults('O', srcDir'resources\saveFileWithFilter.h', .false)

    dlg = .CommonSaveDialog~new(srcDir'resources\saveFileWithFilter.rc', IDD_SAVE_FILE)
    dlg~execute("SHOWTOP", IDI_DLG_OOREXX)

    return 0

::requires "ooDialog.cls"

::class 'CommonSaveDialog' subclass RcDialog

-- Do not copy this GUID into your own code.  Always generate your own GUID.
::constant GUID   'a25b89e4-db67-4423-ae66-a4836bb76024'


/** initDialog()
 *
 * Simple standard init dialog method.  We save references to our commonly used
 * controls, set the .exe check box to checked on start up, and connect the
 * button click event for the push button and the 'None' check box.  Each time
 * the None check box is clicked we toggle the state of the other 3 check boxex.
 *
 * We also use this method to find the installed directory of ooRexx.
 */
::method initDialog
		expose ckExe ckDll ckCls ckNone edit rexx_home

    ckExe  = self~newCheckBox(IDC_CK_EXE)~~check
    ckDll  = self~newCheckBox(IDC_CK_DLL)
    ckCls  = self~newCheckBox(IDC_CK_CLS)
    ckNone = self~newCheckBox(IDC_CK_NONE)

    edit = self~newEdit(IDC_EDIT)

    self~connectButtonEvent(IDC_CK_NONE, 'CLICKED', onCheckNone)
    self~connectButtonEvent(IDC_PB_SHOW, 'CLICKED', onShowSaveDialog)

    rexx_home = value('REXX_HOME', , 'ENVIRONMENT')
    if rexx_home == '' then rexx_home = 'C:\Program Files'


/** onShowSaveDialog()
 *
 * When the user clicks on the 'Show Save File Dialog' push button, we put up
 * the Save File Dialog and then report the results in the edit control.
 *
 * The Save File Dialog filters out files in the view of the dialog.  The
 * checked check boxes determine which files are filtered out, or no files are
 * filtered out.
 *
 * Say the .exe and .dll extensions are set (through the check boxes) to be
 * filtered out.
 *
 * To filter out those values in the view of the dialog, we need to instantiate
 * a .ShellItemFilter object.  The ShellItemFilter has just one method:
 * includeItem().  The operating system's Save File Dialog will invoke that
 * method for each file in the initial folder.  When / if the user changes to a
 * new folder, the operating system again invokes the method for each file in
 * the new folder.  If the method returns S_OK the file is included, if S_FALSE
 * is returned, the file is not shown in the view.
 *
 * The includeItem() of the .ShellItemFilter class simply returns S_OK.  To
 * change that behavior the Rexx programmer creates a subclass and over-rides
 * the includeItem() method.
 *
 * However, the user could simply type in the name: myfile.exe in the edit box.
 * We also want to prevent that.  To do this we need to instantiate a
 * .CommonDialogEvents object.  The .CommonDialogEvents class has a method for
 * every notification the operating system's common item dialog sends.  These
 * methods all simply return the proper value to continue.  To change the
 * default behaviour the Rexx programmer creates a subclass and over-rides the
 * methods she is interested in.
 */
::method onShowSaveDialog unguarded
    expose edit rexx_home

    -- Setting the client GUID has the operating system preserve the state for
    -- this specific save file dialog.
    sfd = .SaveFileDialog~new
    ret = sfd~setClientGuid(self~GUID)

    -- The filter variable is a simple string of words.  Each word in the string
    -- is a file extension to filter out of the view.
    filter = self~getCurrentFilter

    -- If the filter is 'None', the None check box is checked.  No point in
    -- setting the filter or event handler at all for that case.
    if filter \== 'None' then do
        -- Instantiate our .CommonDialogEvents object and inform the Common Item
        -- Dialog of it through the advise() method.
        eventHandler = .CDevents~new
        eventHandler~filter = filter
        sfd~advise(eventHandler)

        -- Same basic thing for our filter object.  The Common Item Dialog is
        -- informed of the filter through the setFilter() method.
        filterObj = .SIFilter~new
        filterObj~filter = filter
        sfd~setFilter(filterObj)
    end

    -- We set the initial folder to the install directory of ooRexx because we
    -- know that directory contains .exe, .dll, and .cls files.
    sfd~setFolder(rexx_home)

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
    edit~setText(text)


/** onCheckNone()
 *
 *  The event handler for the CLICK event.  This method is invoked whenever the
 *  user clicks on the 'None' check box.  If the click checks the check box we
 *  uncheck all the othe check boxes and disable them.  If the click unchecks
 *  the check box we re-enable the other check boxes.
 */
::method onCheckNone unguarded
		expose ckExe ckDll ckCls ckNone

    if ckNone~checked then do
        ckExe~~uncheck~disable
        ckDll~~uncheck~disable
        ckCls~~uncheck~disable
    end
    else do
        ckExe~enable
        ckDll~enable
        ckCls~enable
    end


/** getCurrentFilter()
 *
 * A simple private helper method.  We create a string with each file extension
 * name for every extension whose check box is checked.  If the None check box
 * is checked we use the string 'None'  The file extension names need to be
 * separated by spaces so that we have a string of 'words.'
 */
::method getCurrentFilter unguarded private
		expose ckExe ckDll ckCls ckNone

    if ckNone~checked then do
        f = 'None'
    end
    else do
        f = ''
        if ckExe~checked then f = '.exe'
        if ckDll~checked then f ||= ' .dll'
        if ckCls~checked then f ||= ' .cls'
    end
    if f == '' then f = 'None'

    return f


/* Class:  Helper
 *
 *  Both the .SIFilter and the .CDEvents classes need to obtain the file
 *  extension from complete path names.  The code to do that is put in this
 *  mixin class so we don't need to duplicate the code in each class.
 *
 *  In addition both of the classes need a way to know which filter the user
 *  selected (through the check boxes.)  The filter attribute is a convenient
 *  way to inform the objects of the current filter.
 */
::class 'Helper' mixinclass object
::attribute filter unguarded
::method getExtension
    use strict arg file

    pos = file~lastpos('.')
    if pos > 0 then return file~substr(pos)
    else return ''


/* Class:  SIFilter
 *
 * To assign a filter to your Open/Save File Dialog you need to use a
 * ShellItemFilter object.
 *
 * The .ShellItemFilter class has one method, includeItem().  This method is
 * invoked by the operating system (through the ooDialog framework's
 * implementation of the COM IShellItemFilter interface.)
 *
 * The Rexx ShellItemFilter returns S_OK from the includeItem() method.  To
 * actually filter items, the programmer needs to subclass ShellItemFilter and
 * provide a custom filter by over-riding the includeItem() method.
 */
::class 'SIFilter' subclass ShellItemFilter inherit Helper

/** includeItem()
 *
 * Our includeItem() over-ride.
 *
 * sfd  ->  The Rexx SaveFileDialog object we are connected to.
 * hwnd ->  The window handle of the operating system's Save File Dialog.
 * item ->  The complete path name of a file about to be put in the view.
 */
::method includeItem
    use arg sfd, hwnd, item

    -- If the filter is none, every file should be included and we simply return
    -- S_OK.  This can not actually happen in our program because, if the filter
    -- is none, this event handler is not connected in the first place.
    filStr = self~filter
    if filStr == 'None' then return self~S_OK

    -- Get the extension of this file and see if the extension is in the filter.
    -- If it is, return S_FALSE to exclude it from the view.
    ext = self~getExtension(item)
    if filStr~caselessWordPos(ext) <> 0 then return self~S_FALSE

    return self~S_OK


/* Class:  CDEvents
 *
 * The operating system's Common Item Dialog has 12 event notifications it sends
 * to an event handler.  The Rexx programmer can elect to handle some or all of
 * those events by using a CommonDialogEvents object.
 */
::class 'CDEvents' subclass CommonDialogEvents inherit Helper

/** onFileOk()
 *
 *  This is the event handler for the Ok push button, which is labeled Open, or
 *  Save depending on the type of file dialog.  Or it could be a custom label.
 *
 *  It is invoked when the user has pushed Save (for this dialog) but before
 *  the dialog is closed.  It gives us a chance to veto the close.  If S_OK is
 *  returned the dialog closes.  If S_FALSE is returned the dialog does not
 *  close.
 *
 *  Obviously, if the dialog does not close when the user presses the button, it
 *  can be disconcerting.  So, you should always put up some explanation of why
 *  the dialog does not close.
 *
 *  Here we examine the choosen file and see if it has one of the extensions not
 *  allowed.  If so we refuse the file.
 *
 *  cfd  -> .CommonFileDialog  This is the Rexx Common File Dialog object.
 *  hwnd -> This is the window handle of the operating system file dialog.
 */
::method onFileOk unguarded
  use arg cfd, hwnd

  if self~filter == 'None' then return self~S_OK

  file = cfd~getResult
  if file \== .nil then do
      ext = self~getExtension(file)

      if self~filter~caselessWordPos(ext) <> 0 then do
          extList = self~nameExtensions(self~filter)
          title   = 'ooRexx Save File Selection Error'
          msg     = 'File:' file 'is not acceptable.' || .endOfLine~copies(2) || -
                    'The file to save to must not have' extList
          j = MessageDialog(msg, hwnd, title, 'OK', 'ERROR')
          return self~S_FALSE
      end
  end

  return self~S_OK


/** nameExtensions()
 *
 * Just a simple helper method to take the filter string: '.exe .dll' and turn
 * it into a phrase for the message box.  I.e.:
 *
 * '.cls'  -> an extension of .cls
 *
 * '.exe .cls' -> an extension of .exe, or .cls
 *
 * etc., etc..
 */
::method nameExtensions unguarded private
    use strict arg exts

    count = exts~words
    str = 'an extension of '

    if count == 1 then do
      str ||= exts
    end
    else do i = 1 to count
        if i == count then str ||= 'or' exts~word(i)
        else str ||= exts~word(i) || ', '
    end
    return str || '.'
