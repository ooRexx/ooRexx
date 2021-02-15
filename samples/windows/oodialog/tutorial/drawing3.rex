/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
 * Name: drawing3.rex
 * Type: Open Object REXX Script
 *
 * Description: Example demonstrating drawing functionality, using the
 *              extended arguments sent to the DRAW event handler.
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
    self~connectDraw(10, "drawIt", .true)

    self~graphicObject = "NONE"

    return self~initCode


::method drawIt unguarded
    use arg id, lp, drawObj, itemID, flags, dc, r, itemData

    if self~graphicObject = "NONE" then return .true

    if self~graphicObject = "CIRCLE" then do
        pen = drawObj~createPen(5, "SOLID", 1)
    end
    else do
        pen = drawObj~createPen(8, "SOLID", 2)
    end

    oldPen = drawObj~objectToDc(dc, pen)

    properties = .directory~new
    properties~weight = 700
    properties~italic = .true

    font = drawObj~createFontEx("Arial", 24, properties)
    oldFont = drawObj~fontToDC(dc, font)

    drawObj~transparentText(dc)

    -- Get the midpoint of the button rectangle and set the text align to center
    pos = .Point~new(r~right % 2, r~bottom  % 2)
    oldAlign = drawObj~setTextAlign(dc, 'CENTER BASELINE NOUPDATECP')

    -- Get a draw rectangle indented 5 from the button's area.
    dr = .Rect~new(r~left + 5, r~top + 5, r~right - 10, r~bottom - 10)

    if self~graphicObject = "CIRCLE" then do
        -- Draw the circle, within our draw rectangle
         drawObj~drawArc(dc, dr~left, dr~top, dr~right, dr~bottom)
    end
    else do
        -- Draw the rectangle, indented like the circle above.
        drawObj~rectangle(dc, dr~left, dr~top, dr~right, dr~bottom)
    end

    -- Write the text at the position we calculated above.
    drawObj~writeDirect(dc, pos~x, pos~y, self~graphicObject)

    -- Now restore the DC so it is the same as passed into us.
    drawObj~setTextAlign(dc, oldAlign)
    drawObj~fontToDC(dc, oldFont)
    drawObj~objectToDc(dc, oldPen)

    drawObj~opaqueText(dc)

    drawObj~deleteFont(font)
    drawObj~deleteObject(pen)

    return .true


::method circle unguarded
    self~graphicObject = "CIRCLE"
    self~redrawControl(10, 1)


::method myRectangle unguarded
    self~graphicObject = "RECTANGLE"
    self~redrawControl(10, 1)

