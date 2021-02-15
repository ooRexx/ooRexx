/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2012-2014 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                                        */
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
 * ToolTip Class.
 */

::class 'ToolTip' subclass DialogControl public

::method activate unguarded external "LIBRARY oodialog tt_activate"
::method addTool unguarded external "LIBRARY oodialog tt_addTool"
::method addToolEx unguarded external "LIBRARY oodialog tt_addToolEx"
::method addToolRect unguarded external "LIBRARY oodialog tt_addToolRect"
::method adjustRect unguarded external "LIBRARY oodialog tt_adjustRect"
::method delTool unguarded external "LIBRARY oodialog tt_delTool"
::method enumTools unguarded external "LIBRARY oodialog tt_enumTools"
::method getBubbleSize unguarded external "LIBRARY oodialog tt_getBubbleSize"
::method getCurrentToolInfo unguarded external "LIBRARY oodialog tt_getCurrentTool"
::method getDelayTime unguarded external "LIBRARY oodialog tt_getDelayTime"
::method getMargin unguarded external "LIBRARY oodialog tt_getMargin"
::method getMaxTipWidth unguarded external "LIBRARY oodialog tt_getMaxTipWidth"
::method getText unguarded external "LIBRARY oodialog tt_getText"
::method getTipBkColor unguarded external "LIBRARY oodialog tt_getTipBkColor"
::method getTipTextColor unguarded external "LIBRARY oodialog tt_getTipTextColor"
::method getTitle unguarded external "LIBRARY oodialog tt_getTitle"
::method getToolCount unguarded external "LIBRARY oodialog tt_getToolCount"
::method getToolInfo unguarded external "LIBRARY oodialog tt_getToolInfo"
::method hasCurrentTool unguarded external "LIBRARY oodialog tt_hasCurrentTool"
::method hitTest unguarded external "LIBRARY oodialog tt_hitTestInfo"  -- Erroneously named in ooDialog 4.2.1 doc, need to preserve for backwards compatibility, but no longer documented.
::method hitTestInfo unguarded external "LIBRARY oodialog tt_hitTestInfo"
::method manageAtypicalTool unguarded external "LIBRARY oodialog tt_manageAtypicalTool"
::method newToolRect unguarded external "LIBRARY oodialog tt_newToolRect"
::method pop unguarded external "LIBRARY oodialog tt_pop"
::method popUp unguarded external "LIBRARY oodialog tt_popUp"
::method setDelayTime unguarded external "LIBRARY oodialog tt_setDelayTime"
::method setMargin unguarded external "LIBRARY oodialog tt_setMargin"
::method setMaxTipWidth unguarded external "LIBRARY oodialog tt_setMaxTipWidth"
::method setTipBkColor unguarded external "LIBRARY oodialog tt_setTipBkColor"
::method setTipTextColor unguarded external "LIBRARY oodialog tt_setTipTextColor"
::method setTitle unguarded external "LIBRARY oodialog tt_setTitle"
::method setToolInfo unguarded external "LIBRARY oodialog tt_setToolInfo"
::method setWindowTheme unguarded external "LIBRARY oodialog tt_setWindowTheme"
::method trackActivate unguarded external "LIBRARY oodialog tt_trackActivate"
::method trackPosition unguarded external "LIBRARY oodialog tt_trackPosition"
::method update unguarded external "LIBRARY oodialog tt_update"
::method updateTipText unguarded external "LIBRARY oodialog tt_updateTipText"


::class 'ToolInfo' public
::method forHitTest class external "LIBRARY oodialog ti_forHitTest_cls"
::method forID class external "LIBRARY oodialog ti_forID_cls"

::method init external "LIBRARY oodialog ti_init"
::method unInit external "LIBRARY oodialog ti_unInit"

::attribute flags get external "LIBRARY oodialog ti_flags"
::attribute flags set external "LIBRARY oodialog ti_setFlags"
::attribute rect get external "LIBRARY oodialog ti_rect"
::attribute rect set external "LIBRARY oodialog ti_setRect"
::attribute resource get external "LIBRARY oodialog ti_resource"
::attribute rexxHwnd get external "LIBRARY oodialog ti_rexxHwnd"
::attribute rexxID get external "LIBRARY oodialog ti_rexxID"
::attribute text get external "LIBRARY oodialog ti_text"
::attribute text set external "LIBRARY oodialog ti_setText"
::attribute userData get external "LIBRARY oodialog ti_userData"
::attribute userData set external "LIBRARY oodialog ti_setUserData"

::attribute textMemoryIsAllocated set external "LIBRARY oodialog ti_setTextMemoryIsAllocated"  -- Internal use only.
::method hitTestHelper external "LIBRARY oodialog ti_hitTestHelper"                            -- Internal use only.

