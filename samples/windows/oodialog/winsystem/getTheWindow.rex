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
 *   Name: getTheWindow.rex
 *   Type: Example ooRexx program
 *
 *   Description:  This example shows how to find a window without using the
 *                 exact window title.
 *
 *                 The WindowsManager class provides the find() method to get a
 *                 top-level window using its title.  But, you need to use the
 *                 exact title.  This example uses a method of the
 *                 WindowsManager, desktopWindow(), that is available in ooRexx
 *                 4.0.0 and later.  Once you get the desktop window, you can
 *                 enumerate all its children windows and compare their titles
 *                 in a custom matching function.
 *
 */
say
say
say 'This sample program will find an open top-level window'
say 'using a substring of the window title.  Case does not'
say 'matter.'
say
say 'Enter part of the title of the window you want found'
.stdout~charout("  Substring: ")
parse pull text

-- Instantiate a WindowManager object and get the desktop window.
mgr = .WindowsManager~new
deskTop = mgr~desktopWindow

say
say
say 'First some information about the desktop window:'
say 'Desktop:   ' desktop
say '  Class:   ' deskTop~wClass
say '  Position:' deskTop~coordinates
say
say

text = text~strip
if text == "" then do
  say 'Sorry the empty string is not going to work.'
  say 'Try again when you are serious about finding'
  say 'a window.'
  say
  return 99
end

-- This function is where the real work is done.
wnd = fuzzyFindWindow(deskTop, text)

if wnd <> .nil then do
  say 'Found your window.'
  say '  Title:   ' wnd~title
  say '  Class:   ' wnd~wClass
  say '  Position:' wnd~coordinates
  say '  Styles:  ' wnd~getStyle
end
else do
  say 'Could not find your window'
  say '  Are you sure' text 'is a substring'
  say '  of the window title?'
end

-- fuzzyFindWindow() returns the first matching window.  We'll use another,
-- similar, function to get all matching windows.
say
say
say 'This is a similar exercise, but will find all matching'
say 'windows, using a substring of the window title.'
say
say 'Enter part of the title of the window(s) you want found'
.stdout~charout("  Substring: ")
parse pull text
say
say

text = text~strip
if text == "" then do
  say 'Sorry the empty string is not going to work.'
  say 'Try again when you are serious about finding'
  say 'a window.'
  say
  return 99
end

-- This function will return an array with all matching windows.  An empty array
-- signals no match.
windows = fuzzyFindWindows(deskTop, text)

if windows~items > 0 then do wnd over windows
  say 'Found this window.'
  say '  Title:   ' wnd~title
  say '  Class:   ' wnd~wClass
  say '  Position:' wnd~coordinates
  say '  Styles:  ' wnd~getStyle
  say
end
else do
  say 'Could not find any matching windows.'
  say '  Are you sure' text 'is a substring'
  say '  of some open window title?'
end
say

return 0

::requires 'winsystm.cls'

/** fuzzyFindWindow()
 * Searches the desktop children window for a window whose title contains the
 * text specified.
 *
 * @param  desktopWnd  The desktop window object.
 * @param  text        The text to look for, case is not significant.
 *
 * @return The window object if a window is found, otherwise .nil.
 *
 * @note  Although in this program, only the desktop window is used, the
 *        function itself would work for any window.
 */
::routine fuzzyFindWindow
  use strict arg deskTopWnd, text

  -- Set up the default return.
  wnd = .nil

  -- Starting with the first child window, look at each window's title and see
  -- if it contains the specified text.  The first match found is returned.
  child = deskTopWnd~firstChild
  do while child <> .nil
    title = child~title
    if title~caselessPos(text) <> 0 then do
      wnd = child
      leave
    end

    child = child~next
  end

return wnd

/** fuzzyFindWindows()
 * This is the exact same function as the above, except it returns an array of
 * all the windows that contain the specified text.
 */
::routine fuzzyFindWindows
  use strict arg deskTopWnd, text

  -- Set up the default return.
  w = .array~new

  -- Starting with the first child window, look at each window's title and see
  -- if it contains the specified text.  Add each matching window to the array.
  child = deskTopWnd~firstChild
  do while child <> .nil
    title = child~title
    if title~caselessPos(text) <> 0 then w~append(child)
    child = child~next
  end

return w
