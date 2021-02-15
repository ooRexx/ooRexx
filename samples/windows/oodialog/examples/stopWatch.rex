/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2013-2014 Rexx Language Association. All rights reserved.    */
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

/* ========================================================================= */
/* Stopwatch    a Rexx Stopwatch                                             */
/* ========================================================================= */

   dlg = .StopWatchCls~new      /* Create OODialog Class instance            */
   dlg~execute('ShowTop')       /* Create, show and run the Windows Object   */

EXIT                                                                       -->|
/* ========================================================================= */
::requires 'ooDialog.cls'       /* OODialog Windows GUI Class                */
::requires "winSystm.cls"       /* for clipboard manager                     */
/* ========================================================================= */
::class StopWatchCls subclass userdialog
/* ------------------------------------------------------------------------- */
::attribute state   unguarded           /* [Stopped,Running,Split,splitting] */
::attribute session unguarded           /* session number                    */
::attribute events  unguarded           /* event record for clipboard        */
/* ------------------------------------------------------------------------- */
::method Init
/* ------------------------------------------------------------------------- */

   self~Init:super              /* we call the Super Class (userdialog)      */

   rc = self~CreateCenter(160,62,'Stopwatch -   - Stopped',,,'MS Sans Serif',8)
   self~InitCode  = (rc=0)
   self~state     = 'Stopped'
   self~time      = '00:00:00.00'
   self~Session   = 0
   self~events    = .array~new

/* ------------------------------------------------------------------------- */
::method DefineDialog
/* ------------------------------------------------------------------------- */
expose menuBar

   u = .dlgAreaU~new(self)                         /* Whole dialog           */
   self~createEdit(      10, u~x,       u~y,       u~w,       u~h('31%'),  'readonly center', 'time')
   self~createPushButton(11, u~x,       u~y('41%'),u~w('45%'),u~h('38%'), ,'Start',           'ButtonPress')
   self~createPushButton(12, u~x('55%'),u~y('41%'),u~w('45%'),u~h('38%'), ,'Exit',            'ButtonPress')

   menuBar = .BinaryMenuBar~new
     filePopup = .PopupMenu~new(13)
       filePopup~insertItem(IDCANCEL,IDCANCEL,"E&xit")
     editPopup = .PopupMenu~new(14)
       editPopup~insertItem(15,      15,      "&Copy","GRAYED",,,.true,"Copy2CB")

   menuBar~insertPopup(14,14,editPopup,"&Edit")
   menuBar~insertPopup(13,13,filePopup,"&File")

/* ------------------------------------------------------------------------- */
::method InitDialog
/* ------------------------------------------------------------------------- */
expose el b1 b2 menuBar

  menuBar~attachTo(self)

  b1 = self~newPushButton(11)
  b2 = self~newPushButton(12)
  el = self~newEdit(10)

  d = .Directory~new~~weight(700)
  el~~setcolor(0,13)~~setFont(self~createFontEx('Lucida Console',18,d),1)

return InitDlgRet

/* ------------------------------------------------------------------------- */
::Method ButtonPress UnGuarded
/* Pressing either button runs this method                                   */
/* ------------------------------------------------------------------------- */
expose el b1 b2 events state session menuBar

   -----
   REPLY -->                                         -- run asynchronously -v>|
   -----

   action = self~newPushButton(arg(1))~title       /* pressed button title   */

   Select
      when action='Start'  then do
         call time 'r'                             /* RESET the clock        */
         session += 1
         events~append(session '0909'x 'Start' '09'x date('s',,,'-') time())
         call SetStateButtonsAndTitle 'Running','Stop','Split','Running'
         menuBar~enable(15)
         do while State <> 'Stopped'            /* Run the stopwatch display */
            if State='Running' | State='Splitting'
            then do
               ftime=FormatTime(time('e'))
               el~~SetTitle(ftime)~~Update
               if State='Splitting'
               then do
                  State='Split'
                  events~append(session '09'x ftime '09'x 'Split')
               end /* DO */
            end
            call syssleep 0.004                     /* dont hog resources    */
         end /* DO */
         events~append(session '09'x ftime '09'x 'Stop')
      end /* DO */
      when action='Stop'
         then call SetStateButtonsAndTitle 'Stopped','Start','Exit','Stopped'
      when action='Resume'
         then call SetStateButtonsAndTitle 'Running','Stop','Split','Running'
      when action='Split'
         then call SetStateButtonsAndTitle 'Splitting','Resume','Split','Split Time'
      when action='Exit'   then do
         State='Stopped'                            /* kill running threads  */
         self~OK                                             -- EXIT Dialog ->|
      end
      otherwise
   end /* select */

return

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
SetStateButtonsAndTitle:
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   State = arg(1)
   b1~SetTitle(arg(2))
   b2~SetTitle(arg(3))
   self~SetTitle('Stopwatch -' session '-' arg(4))

return

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
FormatTime:
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
procedure
arg t
  parse var t t'.'ms                                      /* take off the ms */
  hh = t % 3600
  t  = t // 3600
  mm = t %  60
  ss = t // 60

return hh~right(2,'0')||':'||mm~right(2,'0')||':'ss~right(2,'0')||'.'ms~left(2)

/* ------------------------------------------------------------------------- */
::Method Cancel UnGuarded            /* stop possible running thread on exit */
/* ------------------------------------------------------------------------- */
   self~state='Stopped'
   return self~Cancel:Super                            -- Leave the dialog -->|
/* ------------------------------------------------------------------------- */
::method Copy2CB UnGuarded
/* ------------------------------------------------------------------------- */
expose events

  cb = .WindowsClipboard~new
  cb~copy(events~makeString('l',.endofline))

/* ========================================================================= */
