/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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

/**
 * Name: sysinfo.rex
 * Type: Open Object Rexx Script using ooDialog and OLE
 * Resource: sysinfo.rc
 *
 * Description:
 * Demo application for inspecting some system properties using WMI
 *
 * WMI is the acronym for Windows Management Instrumentation. Full documentation
 * of WMI can be found in the Windows SDK which Microsoft provides free of
 * charge.  You can also locate this documentation on the web.  A google search
 * of:
 *
 *   "MSDN Windows Management Instrumentation"
 *
 * will lead you straight to the documentation
 */

  sd = locate()
  .application~setDefaults("O", sd'sysInfo.h', .false)

  sysInfoDlg = .SystemClass~new(sd'sysinfo.rc', SYSINFO_DLG)
  if sysInfoDlg~initCode == 0 then do
    sysInfoDlg~execute("SHOWTOP")
  end

return 0


::requires "ooDialog.cls"

::class 'SystemClass' subclass RcDialog

::method initDialog
  expose cb lb componentQueue queueHasData userHasQuit

  -- Connect the selection change event of the combo box to a method in this
  -- dialog.
  self~connectComboBoxEvent(IDC_CB_COMPONENTS, "SELCHANGE", selectionChange)

  -- Fill the combo box with the names of the WMI classes we will display.
  cb = self~newComboBox(IDC_CB_COMPONENTS)
  if cb \= .nil then do
    cb~add("Win32_BootConfiguration")
    cb~add("Win32_ComputerSystem")
    cb~add("Win32_Environment")
    cb~add("Win32_OperatingSystem")
    cb~add("Win32_Printer")
    cb~add("Win32_Process")
    cb~add("Win32_Processor")
    cb~add("Win32_Service")
  end

  lb = self~newListBox(IDC_LB_DATA)
  cb~select("Win32_ComputerSystem")

  componentQueue = .queue~new
  queueHasData = .false
  userHasQuit = .false
  doneProcessing = .true
  self~start("queueProcessor")

  self~addToQueue("Win32_ComputerSystem")


::method queueProcessor unguarded
  expose queueHasData componentQueue lb userHasQuit doneProcessing

  doneProcessing = .false

  do forever
    if userHasQuit then leave

    guard on when queueHasData

    if userHasQuit then leave

    component = componentQueue~pull
    do while component \== .nil
      if userHasQuit then leave

      -- Change the cursor to the hour glass cursor to indicate we are processing
      -- and the user should be patient.
      self~showHourGlass

      WMIObject = .OLEObject~getObject("WinMgmts:")

      if WMIObject \== .nil, component \== "" then do
        if userHasQuit then leave

        lb~deleteAll
        objects = WMIObject~instancesOf(component)

        do instance over objects
          if userHasQuit then leave

          -- Please note: these objects offer a lot more information than is
          -- displayed here. For simplicity's sake only the name and description
          -- (if available) are shown.

          name = instance~name
          desc = instance~description
          if ((name = desc) | (desc = .nil)) then
            lb~add(name)
          else
            lb~add(name "("desc")")
        end

      end
      -- Restore the cursor to its original shape and position.
      self~showHourGlass(.false)

      component = componentQueue~pull
    end
    queueHasData = .false
  end

  doneProcessing = .true

::method selectionChange unguarded
  expose cb lb

  component = cb~selected
  self~addToQueue(component)

  return 0

::method ok unguarded
  expose userHasQuit queueHasData doneProcessing

  userHasQuit = .true
  queueHasData = .true

  guard on when doneProcessing

  return self~ok:super

::method cancel unguarded
  expose userHasQuit queueHasData doneProcessing

  userHasQuit = .true
  queueHasData = .true

  guard on when doneProcessing

  return self~cancel:super

::method addToQueue private
  expose queueHasData componentQueue
  use strict arg component

  componentQueue~queue(component)
  queueHasData = .true


::method showHourGlass private
  expose lb oldCursorPosition oldCursor
  use strict arg show = .true

  mouse = .Mouse~new(lb)

  if show then do
    -- Save the current cursor position.  Change the list box's cursor shape to
    -- the hour glass and save the old shape.
    oldCursorPosition = mouse~getCursorPos
    oldCursor = mouse~wait

    -- Get the current size and position of the list box.
    p = lb~getRealPos
    s = lb~getRealSize

    -- Set the point object to the midpoint of the list box and position the
    -- cursor at that posiiion
    p~incr(s~width % 2, s~height % 2)
    mouse~setCursorPos(p)
  end
  else do
    mouse~restoreCursor(oldCursor)
    mouse~setCursorPos(oldCursorPosition)
  end

