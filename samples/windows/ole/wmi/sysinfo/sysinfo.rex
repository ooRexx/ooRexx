/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
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
/****************************************************************************/
/* Name: sysinfo.rex                                                        */
/* Type: Object REXX Script using OODialog and OLE                          */
/* Resource: sysinfo.rc                                                     */
/*                                                                          */
/* Description:                                                             */
/* Demo application for inspecting some system properties using WMI         */
/*                                                                          */
/* Attention: You need the IBM Object REXX Development Edition to run this. */
/*                                                                          */
/* Note:                                                                    */
/* Windows 2000 has WMI pre-installed, on WinNT/98 it has to be installed   */
/* manually. See: http://msdn.microsoft.com/downloads/sdks/wmi/eula.asp     */
/*                                                                          */
/* A complete overview of the used classes is available at:                 */
/* http://msdn.microsoft.com/library/psdk/wmisdk/clascomp_3d4j.htm          */
/*                                                                          */
/****************************************************************************/

System = .SystemClass~new
if System~InitCode = 0 then do
  rc = System~Execute("SHOWTOP")
end

exit


::requires "ooDialog.cls"

::class SystemClass subclass UserDialog inherit AdvancedControls MessageExtensions

::method Init
  forward class (super) continue /* call parent constructor */
  InitRet = Result

  if self~Load("sysinfo.rc", ) \= 0 then do
     self~InitCode = 1
     return 1
  end

  /* Connect dialog control items to class methods */
  self~ConnectButton(1,"Ok")

  self~ConnectComboBoxNotify(100,"SELCHANGE",selectionChange)

  /* Add your initialization code here */
  return InitRet

::method InitDialog
  cb = self~newComboBox(100)
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

::method run
  self~selectionChange  /* call self~selectionChanged to display information of initial selection */
  self~run:super

::method Ok
  resOK = self~OK:super  /* make sure self~Validate is called and self~InitCode is set to 1 */
  self~Finished = resOK  /* 1 means close dialog, 0 means keep open */
  return resOK

::method selectionChange
  lc = self~newListBox(101)
  if lc = .nil then return
  lc~DeleteAll
  component = self~newComboBox(100)~title
-- Gather data on the current size and position of the dialog
  parse value lc~getPos() with siX siY
  parse value lc~getSize() with siW siH
-- Get the current position of the cursor
  parse value self~CursorPos with preCX preCY
-- Set the cursor to the hour glass
  ch1 = lc~Cursor_Wait
-- Place the hour glass cursor in the center of the dialog being populated
  lc~SetCursorPos((siX+(siW/2))*self~FactorX,(siY+(siH/2))*self~FactorY)

  WMIobject = .OLEObject~GetObject("WinMgmts:")
  objects = WMIobject~InstancesOf(component)

  do instance over objects
    /* please note: these objects offer a lot more information than is */
    /* displayed here. for simplicity's sake only name and description */
    /* (if available) are shown                                        */
    name = instance~name
    desc = instance~description
    if ((name = desc) | (desc = .nil)) then
      lc~add(name)
    else
      lc~add(name "("desc")")
  end
-- Restore the cursor to its original shape and position
  lc~RestoreCursorShape(ch1)
  self~SetCursorPos(preCX,preCY)
