/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2013-2014 Rexx Language Association. All rights reserved.    */
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
 * ToolBar and ToolBar helper classes.
 */

::class 'ToolBar' subclass DialogControl public

::method addBitmap external  "LIBRARY oodialog tb_addBitmap"
::method addButtons external  "LIBRARY oodialog tb_addButtons"
::method addString external  "LIBRARY oodialog tb_addString"
::method autoSize external  "LIBRARY oodialog tb_autoSize"
::method buttonCount external  "LIBRARY oodialog tb_buttonCount"
::method changeBitmap external  "LIBRARY oodialog tb_changeBitmap"
::method checkButton external  "LIBRARY oodialog tb_checkButton"
::method commandToIndex external  "LIBRARY oodialog tb_commandToIndex"
::method customize external  "LIBRARY oodialog tb_customize"
::method enableButton external  "LIBRARY oodialog tb_enableButton"
::method getButton external  "LIBRARY oodialog tb_getButton"
::method getButtonText external  "LIBRARY oodialog tb_getButtonText"
::method getButtonTextEx external  "LIBRARY oodialog tb_getButtonTextEx"
::method getDisabledImageList external  "LIBRARY oodialog tb_getDisabledImageList"
::method getExtendedStyle external  "LIBRARY oodialog tb_getExtendedStyle"
::method getHotImageList external  "LIBRARY oodialog tb_getHotImageList"
::method getImageList external  "LIBRARY oodialog tb_getImageList"
::method getPressedImageList external  "LIBRARY oodialog tb_getPressedImageList"
::method indexTocommand external  "LIBRARY oodialog tb_indexTocommand"
::method insertButton external  "LIBRARY oodialog tb_insertButton"
::method isButtonChecked external  "LIBRARY oodialog tb_isButtonChecked"
::method isButtonEnabled external  "LIBRARY oodialog tb_isButtonEnabled"
::method isButtonHidden external  "LIBRARY oodialog tb_isButtonHidden"
::method isButtonHighlighted external  "LIBRARY oodialog tb_isButtonHighlighted"
::method isButtonIndeterminate external  "LIBRARY oodialog tb_isButtonIndeterminate"
::method isButtonPressed external  "LIBRARY oodialog tb_isButtonPressed"
::method loadImages external  "LIBRARY oodialog tb_loadImages"
::method setBitmapSize external  "LIBRARY oodialog tb_setBitmapSize"
::method setButtonText external  "LIBRARY oodialog tb_setButtonText"
::method setDisabledImageList external  "LIBRARY oodialog tb_setDisabledImageList"
::method setExtendedStyle external  "LIBRARY oodialog tb_setExtendedStyle"
::method setHotImageList external  "LIBRARY oodialog tb_setHotImageList"
::method setImageList external  "LIBRARY oodialog tb_setImageList"
::method setPressedImageList external  "LIBRARY oodialog tb_setPressedImageList"


::class 'TbButton' public

::method init class external "LIBRARY oodialog tbb_init_cls"

::method init external "LIBRARY oodialog tbb_init"

::attribute bitmapID get external "LIBRARY oodialog tbb_bitmapID"
::attribute bitmapID set external "LIBRARY oodialog tbb_setBitmapID"
::attribute cmdID get external "LIBRARY oodialog tbb_cmdID"
::attribute cmdID set external "LIBRARY oodialog tbb_setCmdID"
::attribute itemData get external "LIBRARY oodialog tbb_itemData"
::attribute itemData set external "LIBRARY oodialog tbb_setItemData"
::attribute state get external "LIBRARY oodialog tbb_state"
::attribute state set external "LIBRARY oodialog tbb_setState"
::attribute style get external "LIBRARY oodialog tbb_style"
::attribute style set external "LIBRARY oodialog tbb_setStyle"
::attribute text get external "LIBRARY oodialog tbb_text"
::attribute text set external "LIBRARY oodialog tbb_setText"

::method assignBitmapID external "LIBRARY oodialog tbb_assignBitmapID"
