/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2008-2008 Rexx Language Association. All rights reserved.    */
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
 *   Name: quickShowAllMenus.rex
 *   Type: Example ooRexx program
 *
 *   Description:  The main purpose of this example is to demonstrate how you
 *                 can quickly put together an ooRexx script that does real
 *                 work once you have a created a package of useful functions.
 *
 *                 This program uses the windowsSystem.frm package to print out
 *                 a menu outline of every open window that has a menu.
 *
 *   The below table lists the external functions and classes used in this
 *   program and shows where to go to examine the source code for them.
 *
 *   External function            Source
 *   ---------------------------------------------------
 *   MenuDetailer                 windowsSystem.frm
 */

  -- Instantiate a WindowManager object and get the desktop window.
  mgr = .WindowsManager~new
  deskTop = mgr~desktopWindow

  -- Instantiate a MenuDetailer object using the desktop window.  The desktop
  -- window does not have a menu, so we won't bother displaying it.
  menuLister = .MenuDetailer~new(desktop)

  -- Keep track of how many windows we found, and how many have a menu.  The
  -- desktop window has a surprisingly large number of windows that we don't
  -- see.
  windowCount = 0
  menuCount  = 0

  -- Enumerate all the desktop child windows, printing a menu outline for each
  -- window that has a menu.
  --
  -- Note this:  many up-to-date Windows applications no longer use true menus.
  -- Instead they use a number of new Windows controls that give the appearance
  -- of menus.  These applications do not have a menu to display.
  ---
  -- Also, some of the menus printed are likely to belong to invisible windows.
  child = deskTop~firstChild
  do while child <> .nil
    windowCount += 1
    if child~menu <> .nil then do
      menuCount += 1

      menuLister~setWindow(child)
      menuLister~print
      say; say
    end
    child = child~next
  end

  say 'Found' windowCount 'windows, of which' menuCount 'had menus.'

::requires 'windowsSystem.frm'
