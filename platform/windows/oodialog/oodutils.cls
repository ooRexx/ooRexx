/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
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
/******************************************************************************/
/*                                                                            */
/*       Windows Dialog Interface for Object REXX                             */
/*                                                                            */
/* Public routines                                                            */
/*                                                                            */
/******************************************************************************/

   /* Routine Play will search for a sound file in the PATH and play it */

::routine Play public
   use arg fileName, asyn
   installed = 0
   if Arg(2, 'o') = 1 then asyn = "NO"
   if Arg(1, 'o') = 1 then do
       if RxFuncQuery("StopSoundFile") \= 0 then do
           call Rxfuncadd "StopSoundFile", "oodialog", "StopSoundFile"
           installed = 1
       end
       ret = StopSoundFile()
       if installed = 1 then call RxfuncDrop "StopSoundFile"
       return ret
   end
   file = SysSearchPath("SOUNDPATH",fileName)
   if file = "" then return 1
   if asyn~translate = "LOOP" then do
       if RxFuncQuery("PlaySoundFileInLoop") \= 0 then do
           call Rxfuncadd "PlaySoundFileInLoop", "oodialog", "PlaySoundFileInLoop"
           installed = 1
       end
       ret = PlaySoundFileInLoop(file)
       if installed = 1 then call RxfuncDrop "PlaySoundFileInLoop"
       return ret
   end
   if RxFuncQuery("PlaySoundFile") \= 0 then do
       call Rxfuncadd "PlaySoundFile", "oodialog", "PlaySoundFile"
       installed = 1
   end
   ret = PlaySoundFile(file, asyn)
   if installed = 1 then call RxfuncDrop "PlaySoundFile"
   return ret


::routine InfoDialog public
   use arg info
   installed = 0
   if RxFuncQuery("InfoMessage") \= 0 then do
       call Rxfuncadd "InfoMessage", "oodialog", "InfoMessage"
       installed = 1
   end
   ret = InfoMessage(info)
   if installed = 1 then call RxfuncDrop "InfoMessage"
   return ret


::routine ErrorDialog public
   use arg info
   installed = 0
   if RxFuncQuery("ErrorMessage") \= 0 then do
       call Rxfuncadd "ErrorMessage", "oodialog", "ErrorMessage"
       installed = 1
   end
   ret = ErrorMessage(info)
   if installed = 1 then call RxfuncDrop "ErrorMessage"
   return ret

::routine AskDialog public
   use arg info, defaultButton
   if Arg(2,'e') then do
       defaultButton = defaultButton~left(1)~translate
       if defaultButton <> 'N' & defaultButton <> 'Y' then
           raise syntax 40.904 array('AskDialog', '2', '[Yes, No]', arg(2))
   end
   installed = 0
   if RxFuncQuery("YesNoMessage") \= 0 then do
       call Rxfuncadd "YesNoMessage", "oodialog", "YesNoMessage"
       installed = 1
   end
   if Arg(2,'o') then ret = YesNoMessage(info)
   else ret = YesNoMessage(info, defaultButton)

   if installed = 1 then call RxfuncDrop "YesNoMessage"
   return ret

::routine FileNameDialog public
   use arg selfile, parentwnd, filter, load, title, defextension, amulti, sepchar
   installed = 0
   if RxFuncQuery("GetFileNameWindow") \= 0 then do
       call Rxfuncadd "GetFileNameWindow", "oodialog", "GetFileNameWindow"
       installed = 1
   end
   if Arg(1,'o') = 1 then selfile = ""
   if Arg(2,'o') = 1 then parentwnd = 0
   if Arg(4,'o') = 1 then load = 1
   else do
       if load~translate = "SAVE" then load = 0
       else if load~translate = "LOAD" then load = 1
   end
   if Arg(6,'o') = 1 then defextension = "TXT"
   if Arg(7,'o') = 1 then amulti = 0
   if Arg(8,'o') = 1 then sepchar = ' '
   else if amulti~translate = "MULTI" then amulti = 1
   if Arg() < 3 then ret = GetFileNameWindow(selfile, parentwnd)
   else ret = GetFileNameWindow(selfile, parentwnd, Arg(3), load, Arg(5), defextension, amulti, sepchar)
   if installed = 1 then call RxfuncDrop "GetFileNameWindow"
   return ret

::routine FindWindow public
   use arg info
   installed = 0
   if RxFuncQuery("FindTheWindow") \= 0 then do
       call Rxfuncadd "FindTheWindow", "oodialog", "FindTheWindow"
       installed = 1
   end
   ret =  FindTheWindow(info)
   if installed = 1 then call RxfuncDrop "FindTheWindow"
   return ret



/*-------------------------------- shortcut routines ---------------------------------*/


::routine TimedMessage public
   use arg msg, title, duration
   dlg = .TimedMessage~new(msg,title,duration)
   dlg~execute
   drop dlg
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
   use arg msg, title, labels, lx, max
   if Arg(4,"o") = 1 then lx = 0
   if Arg(5,"o") = 1 then max= 0
   num = labels~items
   do i=1 to num
      j = i+100
      lab.i = labels[i]
      dat.j = 0
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
