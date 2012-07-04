/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2012-2012 Rexx Language Association. All rights reserved.    */
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

/** Aero Wizard Example - Rexx On A Stick
 *
 * This example demonstrates the new Aero Wizard and presents a wizard that
 * walks a user through the process of setting up a custom Rexx installation on
 * a USB stick.
 *
 * This is a Vista or later only example.  It quits if it is ont run on Vista or
 * later.
 */

  -- A directory manager saves the current directory and can later go back to
  -- that directory.  The class itself is located in DirectoryManaager.cls
  mgr = .DirectoryManager~new()

  .application~setDefaults("O", "rc\aeroRexxOnAStick.h", .false)

  say 'Aero Wizard Dialog.  main() threadID:' .DlgUtil~threadID || msg || .endOfLine

  propDlg = createPropSheet()
  pages = propDlg~pages
  propDlg~execute


  mgr~goBack
  return 0

::requires "ooDialog.cls"
::requires "DirectoryManager.cls"

::class 'PageOneDlg' subclass RcPSPDialog

::method initDialog
  say 'In PageOne::initDialog() hDlg:' self~hwnd

::method queryInitialFocus
  use arg propSheet
  return IDC_PB_GETDIR_FULL

::method translateAccelerator
  use arg msg, keyCode, d, propSheet
  say msg keyCode
  say 'Released:' d~released 'was down:' d~wasDown 'is extended:' d~isExtended
  say 'Alt held:' d~altHeld 'shift held:' d~shiftHeld 'control held:' d~controlHeld
  say

  return self~PSNRET_NOERROR

/** setActive()
 *
 * Note: The order of setWizButtons() / showWizButtons() is important.  If
 *       showWizButtons() is first, it does not work.
 */
::method setActive
  use arg propSheet

  say 'Page one setActive'
  propSheet~setWizButtons("NEXT")
  propSheet~showWizButtons("NEXT", "BACK FINISH NEXT")

  return 0

::method onRevert
  say 'In PageOne::onRevert()'
  propSheet = self~propSheet
  count = propSheet~pages~items
  do i = 1 to count
    say 'Page' i 'resource ID:' propSheet~indexToID(i)
  end
  say "Try altering tab control style"
  old = propSheet~test

::method onPushMe
  say 'In PageOne::onPushMe()'



::class 'PageTwoDlg' subclass RcPSPDialog

::method initDialog
  say 'In PageTwo::initDialog()'

::method validate unguarded
  use strict arg isOkButton, propSheet
  say 'PageTwoDlg::validate() okay button press:' isOkButton 'property sheet:' propSheet

  return self~PSNRET_NOERROR

::method setActive unguarded
  use arg propsheet

  say 'Page two setActive'

  propSheet~setWizButtons("BACK NEXT")
  propSheet~showWizButtons("BACK FINISH NEXT", "BACK FINISH NEXT")

  return 0



::class 'PageThreeDlg' subclass RcPSPDialog

::method setActive unguarded
  use arg propsheet
  say 'Page three setActive'
  propSheet~setWizButtons("BACK NEXT")
  return 0



::class 'PageFourDlg' subclass RcPSPDialog

::method setActive unguarded
  use arg propsheet
  say 'Page the REAL four setActive'
  propSheet~setWizButtons("BACK NEXT")
  return 0

/*::method wizNext
  use arg propSheet
  return 0

::method wizBack
  use arg propSheet
  return propSheet~indexToID(0)

::method queryInitialFocus
  use strict arg idDefFocus, propSheet
  return IDC_CK_HOSTEMU          */



::class 'PageFiveDlg' subclass RcPSPDialog

::method setActive unguarded
  use arg propsheet
  say 'Page five setActive'
  propSheet~setWizButtons("BACK NEXT")
  return 0

/*::method wizNext
  use arg propSheet
  return 0

::method wizBack
  use arg propSheet
  return propSheet~indexToID(5)

::method queryInitialFocus
  use strict arg idDefFocus, propSheet
  return IDC_CK_SAMPLES
                                 */


::class 'PageSixDlg' subclass RcPSPDialog

::method setActive unguarded
  use arg propsheet
  say 'Page six setActive'
  propSheet~setWizButtons("BACK NEXT")
  return 0



::class 'PageSevenDlg' subclass RcPSPDialog

::method setActive unguarded
  use arg propsheet
  say 'Page seven setActive'
  propSheet~setWizButtons("BACK")
  return 0



::class 'AeroWizardDlg' subclass PropertySheetDialog

::method initDialog unguarded
  say 'In AeroWizardDlg::initDialog() hPropSheet:' self~hwnd
  reply

  pages = self~pages
  say '  Pages:' pages

  do d over pages
    say 'Dialog:' d 'is active?' d~isDialogActive
  end


::routine createPropsheet

  p1 = .PageOneDlg~new('rc\aeroRexxOnAStick.rc'  , IDD_PAGE1, , , "AEROPAGE SMALL")
  p2 = .PageTwoDlg~new('rc\aeroRexxOnAStick.rc'  , IDD_PAGE2, , , "AEROPAGE SMALL")
  p3 = .PageTwoDlg~new('rc\aeroRexxOnAStick.rc'  , IDD_PAGE3, , , "AEROPAGE SMALL")
  p4 = .PageFourDlg~new('rc\aeroRexxOnAStick.rc' , IDD_PAGE4, , , "AEROPAGE SMALL")
  p5 = .PageFiveDlg~new('rc\aeroRexxOnAStick.rc' , IDD_PAGE5, , , "AEROPAGE SMALL")
  p6 = .PageSixDlg~new('rc\aeroRexxOnAStick.rc'  , IDD_PAGE6, , , "AEROPAGE SMALL")
  p7 = .PageSevenDlg~new('rc\aeroRexxOnAStick.rc', IDD_PAGE7, , , "AEROPAGE SMALL")

  p1~headerTitle = 'This Wizard will guide you through the steps to create a minimal ooRexx installation package from a full ooRexx installation on this system.'
  p2~headerTitle = 'Enter the path to the current ooRexx installation and the path to the location for the minimal installation file set.'
  p3~headerTitle = 'Select, or deselect, executables other than the core Rexx executables for the minimal file set.'
  p4~headerTitle = 'Include or Exclude Native Extensions for the minimal file set.'
  p5~headerTitle = 'These additional items are good to include when over-all size is not an issue.'
  p6~headerTitle = 'Personal program area needs some good text.'
  p7~headerTitle = 'The set environment command is the heart of the minimal install, allowing a rich ooRexx environment without requiring Admin privileges.'


  pages = .array~of(p1, p2, p3, p4, p5, p6, p7)

  aeroWiz = .AeroWizardDlg~new(pages, "AEROWIZARD WIZARDHASFINISH", "Aero Wizard On Windows 7 Demo")
  aeroWiz~header = .Image~getImage("rc\aeroRexxOnAStickRexxLA.bmp")

  return aeroWiz


