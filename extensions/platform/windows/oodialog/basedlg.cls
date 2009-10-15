/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
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
 * Windows Dialog Interface for Open Object Rexx (ooRexx.)
 *
 * This module contains the WindowsExtensions mixinclass and the BaseDialog
 * class.  These two classes form the foundation of all non-trivial dialogs.
 *
 * Some classes related to Windows resources are also included here.
 */
::requires "plbdlg.cls"
::requires "dlgext.cls"

/**
 * The class WindowExtensions implements methods that are common to all windows,
 * no matter whether they are dialogs or dialog items.  It's an extension to
 * the WindowBase mixin-class.
 *
 * Note that this mixinclass contains many methods that should not be a part of
 * a dialog control class.  The original comment above, does not seem to have
 * been followed through on.
 */

::class 'WindowExtensions' public mixinclass Object

::method initWindowExtensions external "LIBRARY oodialog winex_initWindowExtensions"
::method setFont unguarded external "LIBRARY oodialog winex_setFont"
::method getFont unguarded external "LIBRARY oodialog winex_getFont"
::method hScrollPos unguarded external "LIBRARY oodialog winex_getScrollPos"
::method setHScrollPos unguarded external "LIBRARY oodialog winex_setScrollPos"
::method vScrollPos unguarded  external "LIBRARY oodialog winex_getScrollPos"
::method setVScrollPos unguarded external "LIBRARY oodialog winex_setScrollPos"
::method scroll unguarded external "LIBRARY oodialog winex_scroll"

-- createFont() has always been broken, maintained for program compatibility.
-- createFontEx() is a correct implementation.
::method createFont unguarded external "LIBRARY oodialog winex_createFont"
::method createFontEx unguarded external "LIBRARY oodialog winex_createFontEx"

::method getTextSizeScreen  external "LIBRARY oodialog winex_getTextSizeScreen"
::method cursor_Arrow unguarded external "LIBRARY oodialog winex_setCursorShape"
::method cursor_AppStarting unguarded external "LIBRARY oodialog winex_setCursorShape"
::method cursor_Cross unguarded external "LIBRARY oodialog winex_setCursorShape"
::method cursor_No unguarded external "LIBRARY oodialog winex_setCursorShape"
::method cursor_Wait unguarded external "LIBRARY oodialog winex_setCursorShape"
::method setCursorPos unguarded external "LIBRARY oodialog winex_setCursorPos"
::method getCursorPos unguarded external "LIBRARY oodialog winex_getCursorPos"
::method restoreCursorShape unguarded external "LIBRARY oodialog winex_restoreCursorShape"

::method cursorPos unguarded       -- old method, returns a string
   forward message "getCursorPos" continue
   return result~x result~y



::method GetDC unguarded
   return HandleDC_Obj("G", self~hwnd)

::method FreeDC unguarded
   use arg dc
   return HandleDC_obj("F", self~hwnd, dc)

   /* Deletes a font */
::method DeleteFont unguarded
   use arg hFont
   return self~DeleteObject(hFont)

   /* This method deletes a graphic object */
::method DeleteObject unguarded
   use arg obj
   return HandleDC_Obj("D", obj)

   /* This method loads a bitmap from file into memory */
   /* It returns a handle to the bitmap */

::method LoadBitmap unguarded
   use arg bmpFilename, option
   if arg(2, 'o') = 1 then option = ""; else option = option~translate
   return LoadRemoveBitmap(0 /* NULL adm */, bmpFilename, "LOAD" || option)

   /* This method will free a memory bitmap */

::method RemoveBitmap unguarded
   use arg hBitmap
   return LoadRemoveBitmap(0 /* NULL adm */, hBitmap, "REMOVE")

::method CreateBrush unguarded
    use arg color, brushSpecifier
    if arg() = 0 then return HandleDC_Obj("B")
    if arg(1, "o") = 1 then color = 1
    if arg(2, "o") = 1 then return HandleDC_Obj("B", color)
    brushSpecifier = brushSpecifier~translate
    if brushSpecifier = "UPDIAGONAL" | brushSpecifier = "DOWNDIAGONAL" | brushSpecifier = "CROSS" | brushSpecifier = "DIAGCROSS",
    | brushSpecifier = "HORIZONTAL" | brushSpecifier = "VERTICAL" then return HandleDC_Obj("B", color, brushSpecifier)
    else return HandleDC_Obj("B", 0 /* NULL adm */, color, brushSpecifier)  /* brushSpecifier must be filename */

   /* This method allows you to write text to a window in the given font, in */
   /* the given size, and to the given position */
   /* argument 'fontStyle' can be one or more off: */
   /*    "THIN"            "UNDERLINE"    */
   /*    "EXTRALIGHT"      "ITALIC"       */
   /*    "LIGHT"           "STRIKEOUT"    */
   /*    "MEDIUM"                */
   /*    "SEMIBOLD"              */
   /*    "EXTRABOLD"             */
   /*    "BOLD"                  */
   /*    "HEAVY"                 */

::method Write unguarded
   argarr = arg(1, "A")
   newarg = .array~new(argarr~Items+1)
   do i = 1 to arg(); newarg[i+1] = argarr[i]; end
   newarg[1] = self~hwnd
   forward message (WriteToWindow) arguments (newarg)

   /* This method allows you to write text to a device context to a given */
   /* position */

::method WriteDirect unguarded
   use arg dc, xPos, yPos, text
   return WriteText(dc, xPos, yPos, text)               /* 4 arguments means don't use font */

   /* This method loads a graphic object into device context */

::method ObjectToDC unguarded
   use arg dc, obj
   return HandleDC_Obj("S", dc, obj)

   /* Loads font into device context of a window. */
   /* Use 'createFont' method to get a font handle */

::method FontToDC unguarded
   use arg dc, hFont
   return self~ObjectToDC(dc, hFont)

   /* Set the font color of a device context. */
   /* Use 'createFont' method to get a font handle */

::method FontColor unguarded
   use arg color, dc
   if arg(1,'o') = 1 then color = 1
   return DrawGetSet("COL", dc, color)

   /* This method will create a pen in the given color and style */
   /* It returns a handle to a pen object */
::method CreatePen unguarded
   use arg width, style, color
   if arg(1, "o") = 1 then width = 1
   if arg(2, "o") = 1 then style = "SOLID"; else style = style~translate
   if arg(3, "o") = 1 then color = 0
   return HandleDC_Obj("P", width, style, color)

   /* This method will draw a rectangle to the given device context */
   /* Use one of the following methods to get a valid DC: */
   /* 'GetDC', 'GetWindowDC', 'GetButtonDC' */

::method Rectangle unguarded
   use arg dc, x, y, cx, cy, fill
   if arg(6, "o") = 1 then fill = ''
   if fill~left(1)~translate = 'F' then
        return DCDraw("REC", dc, x, y, cx, cy)
   ret = DrawGetSet("SDP", dc, x, y)
   ret = DCDraw("LN", dc, cx, y)
   ret = DCDraw("LN", dc, cx, cy)
   ret = DCDraw("LN", dc, x, cy)
   return DCDraw("LN", dc, x, y)

   /* This method draws a line within the device context */

::method DrawLine unguarded
   use arg dc, fromX, fromY, toX, toY
   if arg(2, 'o') = 0 | arg(3, 'o') = 0 then ret = DrawGetSet("SDP",dc, fromX, fromY)
   return DCDraw("LN",dc, toX, toY)

   /* This method draws a pixel within the device context */

::method DrawPixel unguarded
   use arg dc, x, y, color
   return DCDraw("PT",dc, X, Y, color)

   /* This method gets the color of a pixel within the device context */

::method GetPixel unguarded
   use arg dc, x, y
   return DrawGetSet("GPT", dc, X, Y)

   /* fill with current brush, border determined by color */
::method FillDrawing unguarded
   use arg dc, x, y, color
   return DCDraw("FL", dc, X, Y, color)

::method TransparentText unguarded
   use arg dc
   return DrawGetSet("SBK", dc, "TRANSPARENT")

::method OpaqueText unguarded
   use arg dc
   return DrawGetSet("SBK", dc, "OPAQUE")

   /* This method draws an arc / circle  */

::method DrawArc unguarded
   use arg dc, left, top, right, bottom, startx, starty, endx, endy
   return DCDraw("ARC",dc, left, top, right, bottom, startx, starty, endx, endy)

   /* This method draws a pie shape */

::method DrawPie unguarded
   use arg dc, left, top, right, bottom, xrad1, yrad1, xrad2, yrad2
   return DCDraw("PIE",dc, left, top, right, bottom, xrad1, yrad1, xrad2, yrad2)

   /* This method draws an arc / circle using an angle */

::method DrawAngleArc unguarded
   use arg dc, movex, movey, x, y, radius, startangle, sweepangle
   if arg(2, 'o') = 0 | arg(3, 'o') = 0 then ret = DrawGetSet("SDP", dc, movex, movey)
   return DCDraw("ANG",dc, x, y, radius, startangle, sweepangle)

::method SetArcDirection unguarded
   use arg dc, direction
   return DrawGetSet("SAD", dc, (direction~translate~wordpos("CLOCKWISE") > 0))

::method GetArcDirection unguarded
   use arg dc
   ret = DrawGetSet("GAD",dc)
   if ret = 1 then return "CLOCKWISE"
   else return "COUNTERCLOCKWISE"

::method LogRect2AbsRect unguarded
   parse arg l, t, r, b
   ret.left = l*self~factorX
   ret.right = r*self~factorX
   ret.top = t*self~factorY
   ret.bottom = b*self~factorY
   return ret.

::method AbsRect2LogRect unguarded
   parse arg l, t, r, b
   ret.left = l/self~factorX
   ret.right = r/self~factorX
   ret.top = t/self~factorY
   ret.bottom = b/self~factorY
   return ret.

/** class: BaseDialog
 * Implements methods that are common to all 3 types of dialogs, ResDialog,
 * RcDialog, and UserDialog.
 */
::class 'BaseDialog' public subclass PlainBaseDialog inherit DialogExtensions WindowExtensions

::attribute scrollNow     -- Indicator whether or not scrolling is active

::method init external "LIBRARY oodialog baseDlg_init"

::method newStatic unguarded external "LIBRARY oodialog baseDlg_newControl"
::method newPushButton unguarded external "LIBRARY oodialog baseDlg_newControl"
::method newRadioButton unguarded external "LIBRARY oodialog baseDlg_newControl"
::method newCheckBox unguarded external "LIBRARY oodialog baseDlg_newControl"
::method newGroupBox unguarded external "LIBRARY oodialog baseDlg_newControl"
::method newEdit unguarded external "LIBRARY oodialog baseDlg_newControl"
::method newComboBox unguarded external "LIBRARY oodialog baseDlg_newControl"
::method newListBox unguarded external "LIBRARY oodialog baseDlg_newControl"
::method newScrollBar unguarded external "LIBRARY oodialog baseDlg_newControl"
::method newTreeView unguarded external "LIBRARY oodialog baseDlg_newControl"
::method newListView unguarded external "LIBRARY oodialog baseDlg_newControl"
::method newTab unguarded external "LIBRARY oodialog baseDlg_newControl"
::method newProgressBar unguarded external "LIBRARY oodialog baseDlg_newControl"
::method newTrackBar unguarded external "LIBRARY oodialog baseDlg_newControl"
::method newMonthCalendar unguarded external "LIBRARY oodialog baseDlg_newControl"
::method newDateTimePicker unguarded external "LIBRARY oodialog baseDlg_newControl"
::method putControl private unguarded external "LIBRARY oodialog baseDlg_putControl_pvt"

::method test unguarded external "LIBRARY oodialog baseDlg_test"

-- TODO needs doc
::method getMenuBar
  return self~menuBar

-- TODO needs doc
::method hasMenuBar
  return self~menuBar \== .nil & self~isLinked

-- DEPRECATED
::method getStaticControl unguarded
   forward message 'newStatic'
::method getButtonControl unguarded
   forward message 'newPushButton'
::method getRadioControl unguarded
   forward message 'newRadioButton'
::method getCheckControl unguarded
   forward message 'newCheckBox'
::method getGroupBox unguarded
   forward message 'newGroupBox'
::method getEditControl unguarded
   forward message 'newEdit'
::method getComboBox unguarded
   forward message 'newComboBox'
::method getListBox unguarded
   forward message 'newListBox'
::method getScrollBar unguarded
   forward message 'newScrollBar'
::method getTreeControl unguarded
   forward message 'newTreeView'
::method getListControl unguarded
   forward message 'newListView'
::method getTabControl unguarded
   forward message 'newTab'
::method getProgressBar unguarded
   forward message 'newProgressBar'
::method getSliderControl unguarded
   forward message 'newTrackBar'
::method getMonthCalendar unguarded
   forward message 'newMonthCalendar'
::method getDateTimePicker unguarded
   forward message 'newDateTimePicker'


::class 'ResourceImage' public

::method init external "LIBRARY oodialog ri_init"
::method release external "LIBRARY oodialog ri_release"
::method handle external "LIBRARY oodialog ri_handle"
::method isNull external "LIBRARY oodialog ri_isNull"
::method systemErrorCode external "LIBRARY oodialog ri_systemErrorCode"
::method getImage external "LIBRARY oodialog ri_getImage"
::method getImages external "LIBRARY oodialog ri_getImages"


::class 'Image' public

::method toID class external "LIBRARY oodialog image_toID_cls"
::method getImage class external "LIBRARY oodialog image_getImage_cls"
::method fromFiles class external "LIBRARY oodialog image_fromFiles_cls"
::method fromIDs class external "LIBRARY oodialog image_fromIDs_cls"
::method userIcon class external "LIBRARY oodialog image_userIcon_cls"

::method colorRef class external "LIBRARY oodialog image_colorRef_cls"
::method getRValue class external "LIBRARY oodialog image_getRValue_cls"
::method getGValue class external "LIBRARY oodialog image_getGValue_cls"
::method getBValue class external "LIBRARY oodialog image_getBValue_cls"

::method init external "LIBRARY oodialog image_init"
::method release external "LIBRARY oodialog image_release"
::method handle external "LIBRARY oodialog image_handle"
::method isNull external "LIBRARY oodialog image_isNull"
::method systemErrorCode external "LIBRARY oodialog image_systemErrorCode"

::class 'ImageList' public
::method create class external "LIBRARY oodialog il_create_cls"

::method init external "LIBRARY oodialog il_init"
::method release external "LIBRARY oodialog il_release"
::method add external "LIBRARY oodialog il_add"
::method addMasked external "LIBRARY oodialog il_addMasked"
::method addIcon external "LIBRARY oodialog il_addIcon"
::method addImages external "LIBRARY oodialog il_addImages"
::method getCount external "LIBRARY oodialog il_getCount"
::method getImageSize external "LIBRARY oodialog il_getImageSize"
::method duplicate external "LIBRARY oodialog il_duplicate"
::method remove external "LIBRARY oodialog il_remove"
::method removeAll external "LIBRARY oodialog il_removeAll"
::method handle external "LIBRARY oodialog il_handle"
::method isNull external "LIBRARY oodialog il_isNull"
