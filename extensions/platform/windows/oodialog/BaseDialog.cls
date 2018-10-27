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
 * Windows Dialog Interface for Open Object Rexx (ooRexx.)
 *
 * This module contains the WindowsExtensions mixinclass. This class is
 * inherited by the PlainBaseDialog class and helps form the foundation of all
 * non-trivial dialogs.
 *
 * Some classes related to Windows resources are also included here.
 */

/**
 * The class WindowExtensions implements methods that are common to all windows,
 * no matter whether they are dialogs or dialog controls.  It's an extension to
 * the WindowBase mixin-class.
 *
 * Note that this mixin class contains (did contian) many methods that should
 * not be a part of a dialog control class.  It also contains (did contain) many
 * methods that would be bettor off in a separate class.  The original comment
 * above, does not seem to have been followed through on.
 */

::class 'WindowExtensions' public mixinclass Object

::method initWindowExtensions external "LIBRARY oodialog winex_initWindowExtensions"

::method createBrush unguarded external "LIBRARY oodialog winex_createBrush"
::method createFont unguarded external "LIBRARY oodialog winex_createFont"
::method createFontEx unguarded external "LIBRARY oodialog winex_createFontEx"
::method createPen unguarded external "LIBRARY oodialog winex_createPen"
::method deleteFont unguarded external "LIBRARY oodialog winex_deleteObject"
::method deleteObject unguarded external "LIBRARY oodialog winex_deleteObject"
::method drawAngleArc unguarded external "LIBRARY oodialog winex_drawAngleArc"
::method drawArc unguarded external "LIBRARY oodialog winex_drawArcOrPie"
::method drawLine unguarded external "LIBRARY oodialog winex_drawLine"
::method drawPie unguarded external "LIBRARY oodialog winex_drawArcOrPie"
::method drawPixel unguarded external "LIBRARY oodialog winex_drawPixel"
::method fillDrawing unguarded external "LIBRARY oodialog winex_fillDrawing"
::method fillRect unguarded external "LIBRARY oodialog winex_fillRect"
::method fontColor unguarded external "LIBRARY oodialog winex_fontColor"
::method fontToDC unguarded external "LIBRARY oodialog winex_objectToDC"
::method freeDC unguarded external "LIBRARY oodialog winex_freeDC"
::method getArcDirection unguarded external "LIBRARY oodialog winex_getSetArcDirection"
::method getDC unguarded external "LIBRARY oodialog winex_getDC"
::method getFont unguarded external "LIBRARY oodialog winex_getFont"
::method getPixel unguarded external "LIBRARY oodialog winex_getPixel"
::method getSysBrush unguarded external "LIBRARY oodialog winex_getSysBrush"
::method getTextAlign unguarded external "LIBRARY oodialog winex_getTextAlign"
::method getTextExtent unguarded external "LIBRARY oodialog winex_getTextExtent"
::method hScrollPos unguarded external "LIBRARY oodialog winex_getScrollPos"
::method loadBitmap unguarded external "LIBRARY oodialog winex_loadBitmap"
::method objectToDC unguarded external "LIBRARY oodialog winex_objectToDC"
::method opaqueText unguarded external "LIBRARY oodialog winex_textBkMode"
::method rectangle unguarded external "LIBRARY oodialog winex_rectangle"
::method removeBitmap unguarded external "LIBRARY oodialog winex_removeBitmap"
::method scroll unguarded external "LIBRARY oodialog winex_scroll"
::method setArcDirection unguarded external "LIBRARY oodialog winex_getSetArcDirection"
::method setFont unguarded external "LIBRARY oodialog winex_setFont"
::method setHScrollPos unguarded external "LIBRARY oodialog winex_setScrollPos"
::method setTextAlign unguarded external "LIBRARY oodialog winex_setTextAlign"
::method setVScrollPos unguarded external "LIBRARY oodialog winex_setScrollPos"
::method transparentText unguarded external "LIBRARY oodialog winex_textBkMode"
::method vScrollPos unguarded  external "LIBRARY oodialog winex_getScrollPos"
::method write unguarded external "LIBRARY oodialog winex_write"
::method writeDirect unguarded external "LIBRARY oodialog winex_writeDirect"


-- DEPRECATED
::method absRect2LogRect unguarded
   parse arg l, t, r, b
   ret.left = l/self~factorX
   ret.right = r/self~factorX
   ret.top = t/self~factorY
   ret.bottom = b/self~factorY
   return ret.

::method cursor_Arrow unguarded
  use strict arg
  forward to (.Mouse~new(self)) message 'ARROW'

::method cursor_AppStarting unguarded
  use strict arg
  forward to (.Mouse~new(self)) message 'APPSTARTING'

::method cursor_Cross unguarded
  use strict arg
  forward to (.Mouse~new(self)) message 'CROSS'

::method cursor_No unguarded
  use strict arg
  forward to (.Mouse~new(self)) message 'NO'

::method cursor_Wait unguarded
  use strict arg
  forward to (.Mouse~new(self)) message 'WAIT'

::method cursorPos unguarded
   forward to (.Mouse~new(self)) message 'GETCURSORPOS' continue
   return result~x result~y

::method logRect2AbsRect unguarded
   parse arg l, t, r, b
   ret.left = l*self~factorX
   ret.right = r*self~factorX
   ret.top = t*self~factorY
   ret.bottom = b*self~factorY
   return ret.

::method restoreCursorShape unguarded
  forward to (.Mouse~new(self)) message 'RESTORECURSOR'

::method setCursorPos unguarded
  forward to (.Mouse~new(self)) message 'SETCURSORPOS'


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
-- ::method getGDIImage class external "LIBRARY oodialog image_getGDIImage_cls" future enhancement
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
::method add external "LIBRARY oodialog il_add"
::method addIcon external "LIBRARY oodialog il_addIcon"
::method addImages external "LIBRARY oodialog il_addImages"
::method addMasked external "LIBRARY oodialog il_addMasked"
::method duplicate external "LIBRARY oodialog il_duplicate"
::method getCount external "LIBRARY oodialog il_getCount"
::method getIcon external "LIBRARY oodialog il_getIcon"
::method getImageSize external "LIBRARY oodialog il_getImageSize"
::method handle external "LIBRARY oodialog il_handle"
::method isNull external "LIBRARY oodialog il_isNull"
::method release external "LIBRARY oodialog il_release"
::method remove external "LIBRARY oodialog il_remove"
::method removeAll external "LIBRARY oodialog il_removeAll"


::class 'CreateWindows' public mixinclass Object

::method initCreateWindows external "LIBRARY oodialog cw_initCreateWindows"
::method createReBarWindow external "LIBRARY oodialog cw_createReBarWindow"
::method createStatusBarWindow external "LIBRARY oodialog cw_createStatusBarWindow"
::method createToolBarWindow external "LIBRARY oodialog cw_createToolBarWindow"



::class 'CustomDraw' public mixinclass EventNotification

::constant CDRF_DODEFAULT                  0  -- 0x00000000
::constant CDRF_NEWFONT                    2  -- 0x00000002
::constant CDRF_SKIPDEFAULT                4  -- 0x00000004
::constant CDRF_DOERASE                    8  -- 0x00000008  draw the background
::constant CDRF_SKIPPOSTPAINT            256  -- 0x00000100  don't draw the focus rect
::constant CDRF_NOTIFYPOSTPAINT           16  -- 0x00000010
::constant CDRF_NOTIFYITEMDRAW            32  -- 0x00000020
::constant CDRF_NOTIFYSUBITEMDRAW         32  -- 0x00000020  flags are the same, MS can distinguish by context
::constant CDRF_NOTIFYPOSTERASE           64  -- 0x00000040

::constant CDDS_PREPAINT                   1  -- 0x00000001
::constant CDDS_POSTPAINT                  2  -- 0x00000002
::constant CDDS_PREERASE                   3  -- 0x00000003
::constant CDDS_POSTERASE                  4  -- 0x00000004

::constant CDDS_ITEM                   65536  -- 0x00010000  the 0x000010000 bit means it's individual item specific
::constant CDDS_ITEMPREPAINT           65537  -- 0x00010001 (CDDS_ITEM | CDDS_PREPAINT)
::constant CDDS_ITEMPOSTPAINT          65538  -- 0x00010002 (CDDS_ITEM | CDDS_POSTPAINT)
::constant CDDS_ITEMPREERASE           65539  -- 0x00010003 (CDDS_ITEM | CDDS_PREERASE)
::constant CDDS_ITEMPOSTERASE          65540  -- 0x00010004 (CDDS_ITEM | CDDS_POSTERASE)
::constant CDDS_SUBITEM               131072  -- 0x00020000
::constant CDDS_SUBITEMPREPAINT       196609  -- 0x00030001  -- ooRexx defined for convenience
::constant CDDS_SUBITEMPOSTPAINT      196610  -- 0x00030002  -- ooRexx defined for convenience

::constant CLR_NONE               4294967295  --  0xFFFFFFFFL
::constant CLR_INVALID            4294967295  --  0xFFFFFFFFL
::constant CLR_DEFAULT            4278190080  --  xFF000000L

::method customDraw external "LIBRARY oodialog cd_init"
::method customDrawControl external "LIBRARY oodialog cd_customDrawControl"
::method RGB external "LIBRARY oodialog image_colorRef_cls"


::class 'ResizingAdmin' public mixinclass Object

::constant IDC_DEFAULT_PINTO_WINDOW     0

::attribute minSize get external   "LIBRARY oodialog ra_minSize"
::attribute minSize set external   "LIBRARY oodialog ra_setMinSize"
::attribute maxSize get external   "LIBRARY oodialog ra_maxSize"
::attribute maxSize set external   "LIBRARY oodialog ra_setMaxSize"

::method controlBottom external "LIBRARY oodialog ra_controlSide"
::method controlLeft external "LIBRARY oodialog ra_controlSide"
::method controlRight external "LIBRARY oodialog ra_controlSide"
::method controlSizing external "LIBRARY oodialog ra_controlSizing"
::method controlTop external "LIBRARY oodialog ra_controlSide"
::method defaultBottom external "LIBRARY oodialog ra_defaultSide"
::method defaultLeft external "LIBRARY oodialog ra_defaultSide"
::method defaultRight external "LIBRARY oodialog ra_defaultSide"
::method defaultSizing external "LIBRARY oodialog ra_defaultSizing"
::method defaultTop external "LIBRARY oodialog ra_defaultSide"
::method defineSizing
  return 0

::method noMaxSize external "LIBRARY oodialog ra_noMaxSize"
::method noMinSize external "LIBRARY oodialog ra_noMinSize"
::method pagedTab external "LIBRARY oodialog ra_pagedTab"
::method useDefaultSizing external "LIBRARY oodialog ra_useDefaultSizing"
::method wantSizeEnded external "LIBRARY oodialog ra_wantSizeEnded"

-- Internal use only
::method initResizing private  external "LIBRARY oodialog ra_initResizing"              -- Do not document
