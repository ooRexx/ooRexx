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

/* Windows Dialog Interface for Open Object Rexx (ooRexx)
 *
 * Utility Classes and Public Routines
 *
 * Contains basic utility classes, including .DlgUtil and all public routines
 * provided in the ooDialog framework.
 */

::class 'DlgUtil' public
::method init class external "LIBRARY oodialog dlgutil_init_cls"
::method version class external "LIBRARY oodialog dlgutil_version_cls"
::method comctl32Version class external "LIBRARY oodialog dlgutil_comctl32Version_cls"
::method hiWord class external "LIBRARY oodialog dlgutil_hiWord_cls"
::method loWord class external "LIBRARY oodialog dlgutil_loWord_cls"
::method and class external "LIBRARY oodialog dlgutil_and_cls"
::method or class external "LIBRARY oodialog dlgutil_or_cls"
::method getSystemMetrics class external "LIBRARY oodialog dlgutil_getSystemMetrics_cls"
::method handleToPointer class external "LIBRARY oodialog dlgutil_handleToPointer_cls"
::method test class external "LIBRARY oodialog dlgutil_test_cls"

::class 'Rect' public
::method init external "LIBRARY oodialog rect_init"
::method left external "LIBRARY oodialog rect_left"
::method 'left=' external "LIBRARY oodialog rect_setLeft"
::method top external "LIBRARY oodialog rect_top"
::method 'top=' external "LIBRARY oodialog rect_setTop"
::method right external "LIBRARY oodialog rect_right"
::method 'right=' external "LIBRARY oodialog rect_setRight"
::method bottom external "LIBRARY oodialog rect_bottom"
::method 'bottom=' external "LIBRARY oodialog rect_setBottom"

::class 'Point' public
::method init external "LIBRARY oodialog point_init"
::method x external "LIBRARY oodialog point_x"
::method 'x=' external "LIBRARY oodialog point_setX"
::method y external "LIBRARY oodialog point_y"
::method 'y=' external "LIBRARY oodialog point_setY"

::class 'Size' public
::method init external "LIBRARY oodialog size_init"
::method width external "LIBRARY oodialog size_cx"
::method 'width=' external "LIBRARY oodialog size_setCX"
::method height external "LIBRARY oodialog point_y"
::method 'height=' external "LIBRARY oodialog point_setY"


/* - - - - - - - - - - - - - Public routines - - - - - - - - - - - - - - - - -*/

/* Routine play() will search for a sound file in the PATH and play it */
::routine play public
   use arg fileName, asyn = "NO"

   if Arg(1, 'o') then return StopSoundFile()

   file = SysSearchPath("SOUNDPATH",fileName)
   if file = "" then return 1

   if asyn~translate = "LOOP" then return PlaySoundFileInLoop(file)
   else return PlaySoundFile(file, asyn)


::routine infoDialog public
   use arg info
   return InfoMessage(info)


::routine errorDialog public
   use arg info
   return ErrorMessage(info)


::routine askDialog public
   use arg info, defaultButton
   if arg(2,'e') then do
       defaultButton = defaultButton~left(1)~translate
       if defaultButton <> 'N', defaultButton <> 'Y' then do
           raise syntax 40.904 array('AskDialog', '2', '[Yes, No]', arg(2))
       end
   end
   if arg(2,'o') then return YesNoMessage(info)
   else return YesNoMessage(info, defaultButton)


::routine fileNameDialog public
   use arg selfile = "", parentwnd = 0, filter, load, title, defextension = "TXT", amulti, sepchar = ' '

   if load == 0 | load~left(1)~translate = "S" then load = 0; else load = 1
   if load == 1 & amulti~left(1)~translate = "M" then amulti = 1; else amulti = 0

   if arg() < 3 then return GetFileNameWindow(selfile, parentwnd)
   else if Arg(3, 'O') then return GetFileNameWindow(selfile, parentwnd, arg(3), load, arg(5), defextension, amulti, sepchar)
   else return GetFileNameWindow(selfile, parentwnd, arg(3) || '0'x, load, arg(5), defextension, amulti, sepchar)


::routine findWindow public
   use strict arg info
   return FindTheWindow(info)

::routine msSleep public
   use strict arg ms
   return SleepMS(ms)

::routine screenSize public
   parse value GetScreenSize() with dx dy px py
   return .array~of(dx,dy,px,py)

::routine systemMetrics public
   use strict arg index
   return .DlgUtil~getSystemMetrics(index)

/** The following routines are needed to maintain backward compatibilty with
 *  pre-4.0.0, because a classic Rexx external function with the same name was
 *  documented.
 */
::routine GetSysMetrics public
  use strict arg index
  return .DlgUtil~getSystemMetrics(index)

::routine InstMMFuncs public
  return 1

::routine BinaryAnd public
  use strict arg n1, n2
  return .DlgUtil~and(n1, n2)

/* ----- End compatibility routines. -----------------------------------------*/

/*-------------------------- shortcut routines -------------------------------*/

::routine TimedMessage public
   use arg msg, title, duration, earlyReply = .false
   if \ duration~datatype('W') then return -1
   if \ earlyReply~datatype('O') then earlyReply = .false
   dlg = .TimedMessage~new(msg,title,duration, earlyReply)
   dlg~execute
   if duration < 0 then return dlg
   if \ earlyReply then drop dlg
   return 0

::routine InputBox public
   use arg msg, title, default, size
   dlg = .InputBox~new(msg,title,default,size)
   ret = dlg~execute
   drop dlg
   return ret

::routine IntegerBox public
   use arg msg, title, default, size
   dlg = .IntegerBox~new(msg,title,default,size)
   ret = dlg~execute
   drop dlg
   return ret

::routine PasswordBox public
   use arg msg, title, default, size
   dlg = .Passwordbox~new(msg,title,default,size)
   ret = dlg~execute
   drop dlg
   return ret

::routine MultiInputBox public
   use arg msg, title, labels, datas, len
   if Arg(5,"o") = 1 then len = 0
   num = labels~items
   do i=1 to num
      j = i+100
      lab.i = labels[i]
      dat.j = datas[i]
   end
   dlg = .MultiInputBox~new(msg, title, lab., dat., len)
   if dlg~execute \= 1 then do; drop dlg; return .NIL; end
   ret = .array~new(num)
   do i=1 to num
      attr = labels[i]~changestr(' ','')~changestr('&','')
      Interpret 'ret[i] = dlg~'attr
   end
   drop dlg
   return ret

::routine ListChoice public
   use arg msg, title, list, lx, ly, predev
   if Arg(4,"o") = 1 then lx = 0
   if Arg(5,"o") = 1 then ly = 0
   if Arg(6,"o") = 1 then predev = ""
   num = list~items
   do i=1 to num
      lst.i = list[i]
   end
   dlg = .ListChoice~new(msg, title, lst., lx, ly, predev)
   res = dlg~execute
   drop dlg
   return res

::routine MultiListChoice public
   use arg msg, title, list, lx, ly, predev
   if Arg(4,"o") = 1 then lx = 0
   if Arg(5,"o") = 1 then ly = 0
   if Arg(6,"o") = 1 then predev = ""
   num = list~items
   do i=1 to num
      lst.i = list[i]
   end
   dlg = .MultiListChoice~new(msg, title, lst., lx, ly, predev)
   res = dlg~execute
   if res = '' then do; drop dlg; return .NIL; end
   ret = .array~new
   do i=1 to words(res)
      ret[i] = list[word(res,i)]
   end
   drop dlg
   return ret

::routine CheckList public
   use arg msg, title, labels, checks, lx = 0, max = 0
   if Arg(4,"o") then checks = .array~new
   num = labels~items
   do i=1 to num
      j = i+100
      lab.i = labels[i]
      if checks~hasIndex(i) then do
         if checks[i]~datatype('O') then dat.j = checks[i]
         else dat.j = 0
      end
      else dat.j = 0
   end
   dlg = .CheckList~new(msg, title, lab., dat., lx, max)
   if dlg~execute \= 1 then do; drop dlg; return .NIL; end
   ret = .array~new
   do j=101 to 100+num
      ret[j-100] = dat.j
   end
   drop dlg
   return ret

::routine SingleSelection public
   use arg msg, title, labels, data, lx, max
   if Arg(5,"o") = 1 then lx = 0
   if Arg(6,"o") = 1 then max= 0
   num = labels~items
   do i=1 to num
      lab.i = labels[i]
   end
   dlg = .SingleSelection~new(msg, title, lab., data, lx, max)
   res = dlg~execute
   drop dlg
   return res
