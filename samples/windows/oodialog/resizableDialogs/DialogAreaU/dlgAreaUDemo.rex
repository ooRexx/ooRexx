/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2006-2014 Rexx Language Association. All rights reserved.    */
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
/* DlgAreaDemo.Rex  --  Demonstrate DlgArea & DlgAreaU Classes  --  Feb 2006 */

sd = locate()
.application~useGlobalConstDir("O", sd'dlgAreaUDemo.h')

MyDlg=.MyDialog~new
MyDlg~execute('ShowTop')

return 0

::requires "ooDialog.cls"
/* ========================================================================= */
::class MyDialog Subclass UserDialog
/* ========================================================================= */
::method init
/* ------------------------------------------------------------------------- */
  forward class (super) continue
  success=self~createCenter(250,250,'My Resizable Dialog',,
                                    'ThickFrame MinimizeBox MaximizeBox',,,
                                    'MS Sans Serif',8)
  if \success then do
    self~initCode = 1
    return
  end

  self~connectResize('OnResize', .true)

/* ------------------------------------------------------------------------- */
::method defineDialog
/* ------------------------------------------------------------------------- */
expose u

u=.dlgAreaU~new(self)                                         /* whole dlg   */
if u~lastError \= .nil then call errorDialog u~lastError

u~noResizePut(IDC_PB_0)
e=.dlgArea~new(u~x       , u~y       , u~w('70%'), u~h('90%'))   /* edit   area */
s=.dlgArea~new(u~x       , u~y('90%'), u~w('70%'), u~hr      )   /* status area */
b=.dlgArea~new(u~x('70%'), u~y       , u~wr      , u~hr      )   /* button area */

self~createEdit(IDC_EDIT, e~x, e~y, e~w, e~h, 'multiline', 'text')
self~createStaticText(IDC_ST_STATUS, s~x, s~y, s~w, s~h, , 'Status info appears here')

self~createPushButton(IDC_PB_0, b~x, b~y('00%'), b~w, b~h('9%'), , 'Button' 0, 'Button'||0)
self~createPushButton(IDC_PB_1, b~x, b~y('10%'), b~w, b~h('9%'), , 'Button' 1, 'Button'||1)
self~createPushButton(IDC_PB_2, b~x, b~y('20%'), b~w, b~h('9%'), , 'Button' 2, 'Button'||2)
self~createPushButton(IDC_PB_3, b~x, b~y('30%'), b~w, b~h('9%'), , 'Button' 3, 'Button'||3)
self~createPushButton(IDC_PB_4, b~x, b~y('40%'), b~w, b~h('9%'), , 'Button' 4, 'Button'||4)
self~createPushButton(IDC_PB_5, b~x, b~y('50%'), b~w, b~h('9%'), , 'Button' 5, 'Button'||5)
self~createPushButton(IDC_PB_6, b~x, b~y('60%'), b~w, b~h('9%'), , 'Button' 6, 'Button'||6)
self~createPushButton(IDOK, b~x, b~y('90%'), b~w, b~h('9%'), 'DEFAULT', 'Ok')

/* ------------------------------------------------------------------------- */
::method onResize
/* ------------------------------------------------------------------------- */
expose u
use arg dummy,sizeinfo
u~resize(self,sizeinfo)

/* ------------------------------------------------------------------------- */
::method unknown
/* ------------------------------------------------------------------------- */
use arg msgname, args
if msgname~abbrev("BUTTON") then
   self~newStatic(IDC_ST_STATUS)~setText('You Pressed Button' msgname~right(1))
