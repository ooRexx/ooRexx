/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2006 Rexx Language Association. All rights reserved.         */
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
/* DlgAreaDemo.Rex  --  Demonstrate DlgArea & DlgAreaU Classes  --  Feb 2006 */

MyDlg=.MyDialog~new
MyDlg~execute('ShowTop')
MyDlg~DeInstall

exit
::requires "ooDialog.cls"
/* ========================================================================= */
::class MyDialog Subclass UserDialog Inherit AdvancedControls
/* ========================================================================= */
::method Init
/* ------------------------------------------------------------------------- */
  self~Init:super
  rc=self~CreateCenter(250,250,'MyDialog',,
                               'ThickFrame MinimizeBox MaximizeBox',,,
                               'MS Sans Serif',8)
  self~InitCode=(rc=0)
  self~connectResize('OnResize')

/* ------------------------------------------------------------------------- */
::method DefineDialog
/* ------------------------------------------------------------------------- */
expose u

u=.dlgAreaU~new(self)                                         /* whole dlg   */
if u~lastError \= .nil then call errorDialog u~lastError

u~NoResize~put(13)
e=.dlgArea~new(u~x       ,u~y       ,u~w('70%'),u~h('90%'))   /* edit   area */
s=.dlgArea~new(u~x       ,u~y('90%'),u~w('70%'),u~hr      )   /* status area */
b=.dlgArea~new(u~x('70%'),u~y       ,u~wr      ,u~hr      )   /* button area */

self~addEntryLine(12,'text',e~x,e~y,e~w,e~h,'multiline')
self~addText(s~x,s~y,s~w,s~h,'Status info appears here',,11)

self~addButton(13,b~x,b~y('00%'),b~w,b~h('9%'),'Button' 0,'Button'||0)
self~addButton(14,b~x,b~y('10%'),b~w,b~h('9%'),'Button' 1,'Button'||1)
self~addButton(15,b~x,b~y('20%'),b~w,b~h('9%'),'Button' 2,'Button'||2)
self~addButton(16,b~x,b~y('30%'),b~w,b~h('9%'),'Button' 3,'Button'||3)
self~addButton(17,b~x,b~y('40%'),b~w,b~h('9%'),'Button' 4,'Button'||4)
self~addButton(18,b~x,b~y('50%'),b~w,b~h('9%'),'Button' 5,'Button'||5)
self~addButton(19,b~x,b~y('60%'),b~w,b~h('9%'),'Button' 6,'Button'||6)
self~addButton( 1,b~x,b~y('90%'),b~w,b~h('9%'),'Ok','Ok','DEFAULT')

/* ------------------------------------------------------------------------- */
::method onResize
/* ------------------------------------------------------------------------- */
expose u
use arg dummy,sizeinfo
                                 /* wait for last size event msg then resize */
if self~peekDialogMessage~left(8) \= "onResize" then u~resize(self,sizeinfo)

/* ------------------------------------------------------------------------- */
::method Unknown
/* ------------------------------------------------------------------------- */
use arg msgname, args
self~getStaticControl(11)~setText('You Pressed' msgname)
