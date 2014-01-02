/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
 * Name: drawing2.rex
 * Type: Open Object REXX Script
 *
 * Description: Example demonstrating drawing functionality.
 */

sd = locate()
dlg = .MyDialogClass~new(sd"drawings.rc", 100)
if dlg~initCode <> 0 then return 99
dlg~execute("SHOWTOP")

return 0

::requires "ooDialog.cls"

::class 'MyDialogClass' subclass RcDialog

::attribute graphicObject

::method init

    forward class (super) continue
    if self~initCode <> 0 then return self~initCode

    self~connectButtonEvent(11, "CLICKED", "circle")
    self~connectButtonEvent(12, "CLICKED", "myRectangle")
    self~connectDraw(10, "drawIt")

    self~graphicObject = "NONE"

    return self~initCode


::method drawIt
    use arg id

    if self~graphicObject = "NONE" then return 0

    dc = self~getControlDC(10)

    if self~graphicObject = "CIRCLE" then do
        pen = self~createPen(5, "SOLID", 1)
    end
    else do
        pen = self~createPen(8, "SOLID", 2)
    end

    oldPen = self~objectToDc(dc, pen)

    properties = .directory~new
    properties~weight = 700
    properties~italic = .true

    font = self~createFontEx("Arial", 24, properties)
    oldFont = self~fontToDC(dc, font)

    self~transparentText(dc)

    self~getTextAlign(dc)

    if self~graphicObject = "CIRCLE" then do
        -- Get the size of the bounding rectangle for the text we are going to
        -- write, in this DC:
        size = self~getTextExtent(dc, self~graphicObject)

        -- Get the midpoint of the circle we are about to draw;
        pos = .Point~new(((300 - 10) % 2) + 10, ((200 - 10) % 2) + 10)

        -- Now adjust the pos point to specify where to write the text, in order
        -- to center it in the circle:
        pos~x -= size~width  % 2
        pos~y -= size~height % 2

        -- Draw the circle.
        self~drawArc(dc, 10, 10, 300, 200)
    end
    else do
        -- Do the same calculations for the position to write the text as we did
        -- above for CIRCLE:
        size = self~getTextExtent(dc, self~graphicObject)

        pos = .Point~new(((320 - 10) % 2) + 10, ((200 - 10) % 2) + 10)
        pos~x -= size~width  % 2
        pos~y -= size~height % 2

        -- Draw the rectangle.
        self~rectangle(dc, 10, 10, 320, 200)
    end

    -- Write the text at the position we calculated above.
    self~writeDirect(dc, pos~x, pos~y, self~graphicObject)

    self~fontToDC(dc, oldFont)
    self~deleteFont(font)

    self~objectToDc(dc, oldPen)
    self~deleteObject(pen)

    self~opaqueText(dc)
    self~freeControlDC(10, dc)

    return .true


::method circle
    self~graphicObject = "CIRCLE"
    self~redrawControl(10, 1)


::method myRectangle
    self~graphicObject = "RECTANGLE"
    self~redrawControl(10, 1)

