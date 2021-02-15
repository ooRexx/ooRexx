/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2013-2014 Rexx Language Association. All rights reserved.    */
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
 * ReBar and ReBar helper classes.
 */

::class 'ReBar' subclass DialogControl public

::method deleteBand unguarded external "LIBRARY oodialog rebar_deleteBand"
::method getBandBorders unguarded external "LIBRARY oodialog rebar_getBandBorders"
::method getBandCount unguarded external "LIBRARY oodialog rebar_getBandCount"
::method getBandInfo unguarded external "LIBRARY oodialog rebar_getBandInfo"
::method getBandMargins unguarded external "LIBRARY oodialog rebar_getBandMargins"
::method getBarHeight unguarded external "LIBRARY oodialog rebar_getBarHeight"
::method getBarInfo unguarded external "LIBRARY oodialog rebar_getBarInfo"
::method getBkColor unguarded external "LIBRARY oodialog rebar_getBkColor"
::method getColorScheme unguarded external "LIBRARY oodialog rebar_getColorScheme"
::method getExtendedStyle unguarded external "LIBRARY oodialog rebar_getExtendedStyle"
::method getImageList unguarded external "LIBRARY oodialog rebar_getBarInfo"
::method getPalette unguarded external "LIBRARY oodialog rebar_getPalette"
::method getRect unguarded external "LIBRARY oodialog rebar_getRect"
::method getRowCount unguarded external "LIBRARY oodialog rebar_getRowCount"
::method getRowHeight unguarded external "LIBRARY oodialog rebar_getRowHeight"
::method getTextColor unguarded external "LIBRARY oodialog rebar_getTextColor"
::method getToolTips unguarded external "LIBRARY oodialog generic_getToolTips"
::method hitTestInfo unguarded external "LIBRARY oodialog rebar_hitTestInfo"
::method idToIndex unguarded external "LIBRARY oodialog rebar_idToIndex"
::method insertBand unguarded external "LIBRARY oodialog rebar_insertBand"
::method maximizeBand unguarded external "LIBRARY oodialog rebar_maximizeBand"
::method minimizeBand unguarded external "LIBRARY oodialog rebar_minimizeBand"
::method moveBand unguarded external "LIBRARY oodialog rebar_moveBand"
::method pushChevron unguarded external "LIBRARY oodialog rebar_pushChevron"
::method setBandInfo unguarded external "LIBRARY oodialog rebar_setBandInfo"
::method setBandWidth unguarded external "LIBRARY oodialog rebar_setBandWidth"
::method setBarInfo unguarded external "LIBRARY oodialog rebar_setBarInfo"
::method setBkColor unguarded external "LIBRARY oodialog rebar_setBkColor"
::method setColorScheme unguarded external "LIBRARY oodialog rebar_setColorScheme"
::method setExtendedStyle unguarded external "LIBRARY oodialog rebar_setExtendedStyle"
::method setImageList unguarded external "LIBRARY oodialog rebar_setBarInfo"
::method setPalette unguarded external "LIBRARY oodialog rebar_setPalette"
::method setParent unguarded external "LIBRARY oodialog rebar_setParent"
::method setTextColor unguarded external "LIBRARY oodialog rebar_setTextColor"
::method setToolTips unguarded external "LIBRARY oodialog generic_setToolTips"
::method setWindowTheme unguarded external "LIBRARY oodialog rebar_setWindowTheme"
::method showBand unguarded external "LIBRARY oodialog rebar_showBand"
::method sizeToRect unguarded external "LIBRARY oodialog rebar_sizeToRect"


::class 'ReBarBandInfo' public

::method init class external "LIBRARY oodialog rbbi_init_cls"

::method init external "LIBRARY oodialog rbbi_init"
::method unInit external "LIBRARY oodialog rbbi_unInit"

::attribute bitmapBack get external "LIBRARY oodialog rbbi_bitmapBack"
::attribute bitmapBack set external "LIBRARY oodialog rbbi_setBitmapBack"
::attribute chevronLocation get external "LIBRARY oodialog rbbi_chevronLocation"
::attribute chevronLocation set external "LIBRARY oodialog rbbi_setChevronLocation"
::attribute chevronState get external "LIBRARY oodialog rbbi_chevronState"
::attribute chevronState set external "LIBRARY oodialog rbbi_setChevronState"
::attribute child get external "LIBRARY oodialog rbbi_child"
::attribute child set external "LIBRARY oodialog rbbi_setChild"
::attribute clrBack get external "LIBRARY oodialog rbbi_clrBack"
::attribute clrBack set external "LIBRARY oodialog rbbi_setClrBack"
::attribute clrFore get external "LIBRARY oodialog rbbi_clrFore"
::attribute clrFore set external "LIBRARY oodialog rbbi_setClrFore"
::attribute cx get external "LIBRARY oodialog rbbi_cx"
::attribute cx set external "LIBRARY oodialog rbbi_setCx"
::attribute cxHeader get external "LIBRARY oodialog rbbi_cxHeader"
::attribute cxHeader set external "LIBRARY oodialog rbbi_setCxHeader"
::attribute cxIdeal get external "LIBRARY oodialog rbbi_cxIdeal"
::attribute cxIdeal set external "LIBRARY oodialog rbbi_setCxIdeal"
::attribute cxMinChild get external "LIBRARY oodialog rbbi_cxMinChild"
::attribute cxMinChild set external "LIBRARY oodialog rbbi_setCxMinChild"
::attribute cyChild get external "LIBRARY oodialog rbbi_cyChild"
::attribute cyChild set external "LIBRARY oodialog rbbi_setCyChild"
::attribute cyIntegral get external "LIBRARY oodialog rbbi_cyIntegral"
::attribute cyIntegral set external "LIBRARY oodialog rbbi_setcyIntegral"
::attribute cyMaxChild get external "LIBRARY oodialog rbbi_cyMaxChild"
::attribute cyMaxChild set external "LIBRARY oodialog rbbi_setCyMaxChild"
::attribute cyMinChild get external "LIBRARY oodialog rbbi_cyMinChild"
::attribute cyMinChild set external "LIBRARY oodialog rbbi_setCyMinChild"
::attribute id get external "LIBRARY oodialog rbbi_id"
::attribute id set external "LIBRARY oodialog rbbi_setId"
::attribute imageIndex get external "LIBRARY oodialog rbbi_imageIndex"
::attribute imageIndex set external "LIBRARY oodialog rbbi_setImageIndex"
::attribute itemData get external "LIBRARY oodialog rbbi_itemData"
::attribute itemData set external "LIBRARY oodialog rbbi_setItemData"
::attribute mask get external "LIBRARY oodialog rbbi_mask"
::attribute mask set external "LIBRARY oodialog rbbi_setMask"
::attribute style get external "LIBRARY oodialog rbbi_style"
::attribute style set external "LIBRARY oodialog rbbi_setStyle"
::attribute text get external "LIBRARY oodialog rbbi_text"
::attribute text set external "LIBRARY oodialog rbbi_setText"
