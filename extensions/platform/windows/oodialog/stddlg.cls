/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
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
/* Standard Dialog Classes                                                    */
/*                                                                            */
/******************************************************************************/

::requires "PluDlg.cls"

/* This class shows a message window for a defined duration */

::class 'TimedMessage' subclass PlainUserDialog public

   /* The class' constructor */

::method init
   expose message title sleeping earlyReply
   use arg message, title, sleeping, earlyReply = .false
   if \ earlyReply~datatype('O') then earlyReply = .false
   if \ sleeping~datatype('W') then sleeping = 0
   self~init:super("")

::method DefineDialog
   expose message sx sy
   self~addText(10,10,sx+2,sy, message)

::method InitAutoDetection
   self~NoAutoDetection

::method execute
   expose message title sleeping earlyReply sx sy

   s = self~getTextSizeDlg(message)
   sx = s~width
   sy = s~height
   self~createCenter(sx+20, sy + 20, title,"NOMENU", , , , 1)
   self~startit
   self~show("SHOWTOP")
   if sleeping >= 0 then
   do
      if earlyReply then reply
      ret = sleepms(sleeping)
      self~stopit
   end


/* This class provides a simple dialog with a titel, a message, one entry line, */
/*   an OK and a Cancel push button */

::class 'InputBox' subclass PlainUserDialog public

::method sx attribute
::method sy attribute

::method init
   expose message title datas. size
   use arg message, title, default, size
   if Arg(3,"o") = 1 | default = "DEFAULT" then datas.101 = ""; else datas.101 = default
   if Arg(4,"o") = 1 | size = "SIZE" then size = 0
   self~init:super(datas.)

::method DefineDialog
   expose message size dlgy
   if size = 0 then size = self~sx
   self~addText(10,10, self~sx+2, self~sy, message)
   self~AddLine(10, self~sy+4+10, size)
   self~AddButtonGroup(self~sx-80, dlgy - 18, 0, 0, "&Ok 1 OK &Cancel 2 CANCEL", 1, "DEFAULT")

::method AddLine
   use arg x, y, l
   self~AddEntryLine(101,"DATA", x, y, l)


::method execute
   expose message title datas. dlgy size

   s = self~getTextSizeDlg(message)
   self~sx = max(s~width,100,size)
   self~sy = s~height

   dlgy = (self~sy+4)*2+36
   dlgx = self~sx + 20
   if dlgx < 110 then dlgx = 110

   self~createCenter(dlgx, dlgy, title,,,,,4)
   self~execute:super("SHOWTOP")
   if self~InitCode = 1 then return datas.101
   else return ""


/* This class is an InputBox dialog which entry line have the not visible flag */

::class 'PasswordBox' subclass InputBox public

::method AddLine
   use arg x, y, l
   self~AddPasswordLine(101, "DATA", x, y, l)


/* This class is an InputBox dialog which entry line allows only numerical data */

::class 'IntegerBox' subclass InputBox public

::method validate
   self~GetDataStem(datas.)
   if datas.101~DataType('W') = 0 then
   do
      call InfoMessage("You did not enter numerical data")
      return 0
   end
   else return 1


/* This class is a dialog that provides a title a message and one or more */
/* entry lines. After execution of this dialog you can query the values of the */
/* entry lines. */

::class 'MultiInputBox' subclass PlainUserDialog public

::method init
   expose message title datas. entries. len
   use arg message, title, entries., datas., len
   if Arg(5,"o") = 1 then len = 0
   self~init:super(datas.)

::method DefineDialog
   expose message sx sy len dlgx dlgy entries. maxlen
   self~addText(10,10, sx+2, sy, message)
   self~AddInputStem(101, 10, sy+4+10, maxlen+4, len, entries.)
   self~AddButtonGroup(dlgx-100, dlgy - 18, 0, 0, "&Ok 1 OK &Cancel 2 CANCEL", 1, "DEFAULT")

::method execute
   expose message title sx sy datas. dlgx dlgy len entries. maxlen

   s = self~getTextSizeDlg(message)
   sx = s~width
   sy = s~height
   ll = 0; count = 0

   do while var("entries."count+1) = 1
      count = count + 1
      s = self~getTextSizeDlg(entries.count)
      if s~width > ll then ll = s~width
   end

   if len = 0 then len = max(sx - ll, 50)
   maxlen = ll
   dlgy = count * (sy * 2) + trunc((sy*count)/4) + 56

   if sx >= len + ll + 4 then dlgx = (sx+24)
   else dlgx = (len + ll + 24)
   if dlgx < 110 then dlgx = 110

   self~createCenter(dlgx, dlgy, title,,,,, count*2+5)
   self~execute:super("SHOWTOP")
   return self~InitCode


/* This class provides a dialog with a list box, an OK and a Cancel button. */
/* You the selected item will be returned when the OK pushbutton terminates the dialog */

::class 'ListChoice' subclass PlainUserDialog public

::method sx attribute
::method sy attribute
::method dlgx attribute
::method dlgy attribute
::method lenx attribute
::method leny attribute
::method message attribute

::method init
   expose message title datas. input. lenx leny
   use arg message, title, input., lenx, leny, predev
   if Arg(4,"o") = 1 then lenx = 0
   if Arg(5,"o") = 1 then leny = 0
   if Arg(6,"o") = 0 then datas.101 = predev; else datas.101 = ""
   self~init:super(datas.)
   s = self~getTextSizeDlg(message)
   self~sx = s.width
   self~sy = s.height
   self~message = message

::method DefineDialog
   expose message sx sy lenx leny dlgx dlgy
   self~addText(10,10, sx+2, sy, message)
   self~AddListBox(101, "DATA", 10, sy+4+10, lenx, leny+4)
   self~AddButtonGroup(dlgx-100, dlgy - 18, 0, 0, "&Ok 1 OK &Cancel 2 CANCEL", 1, "DEFAULT")

::method InitDialog
   expose count input.
   do i = 1 to count
      self~AddListEntry(101, input.i)
   end
   self~SetListLine(101, "")


   /* after execution it will return the value of the selected item */

::method execute
   expose message title sx sy datas. input. count lenx leny dlgx dlgy

   s = self~getTextSizeDlg(message)
   sx = s~width
   sy = s~height

   ll = 0; count = 0
   do while var("input."count+1) = 1
      count = count + 1
      s = self~getTextSizeDlg(input.count)
      if s~width > ll then ll = s~width
   end

   if self~lenx = 0 then self~lenx = max(self~sx,ll+5)
   if self~leny = 0 then do
      if count > 15 then self~leny = self~sy*15
      else self~leny = count*self~sy
   end

   self~dlgy = self~leny+(self~sy+4)+34

   if self~sx >= self~lenx then self~dlgx = self~sx + 20; else self~dlgx = self~lenx + 20
   if self~dlgx < 110 then self~dlgx = 110

   self~createCenter(self~dlgx, self~dlgy, title,,,,, 4)
   self~execute:super("SHOWTOP")

   if self~InitCode = 1 then return datas.101
   else return ""



::class 'MultiListChoice' subclass ListChoice public


::method DefineDialog
   self~addText(10,10, self~sx+2, self~sy, self~message)
   self~AddListBox(101, "DATA", 10, self~sy+4+10, self~lenx, self~leny+4, "MULTI VSCROLL TABULATORS")
   self~AddButtonGroup(self~dlgx-100, self~dlgy - 18, 0, 0, "&Ok 1 OK &Cancel 2 CANCEL", 1, "DEFAULT")


/* This class is a dialog with a group of one or more check boxes. */

::class 'CheckList' subclass PlainUserDialog public

  /* Entries is a stem variable with all the titles for the check boxes */
  /* Datas is a stem variable you can use to preselect the check boxes */
  /* 'Datas.i=1' means there will be a check mark on the i-th box */
::method Init
   expose message title datas. entries. len max
   use arg message, title, entries., datas., len, max
   if Arg(5,"o") = 1 then len = 0
   if Arg(6,"o") = 1 then max = 0
   self~init:super(datas.)

::method DefineDialog
   expose message sx sy len dlgx dlgy entries. placex max
   self~addText(10,10, sx+2, sy, message)
   self~AddCheckBoxStem(101, placex, sy+4+10, len, entries., max)
   self~AddButtonGroup(dlgx-100, dlgy - 18, 0, 0, "&Ok 1 OK &Cancel 2 CANCEL", 1, "DEFAULT")

::method Execute
   expose message title sx sy datas. dlgx dlgy len entries. placex max

   s = self~getTextSizeDlg(message)
   sx = s~width
   sy = s~height
   ll = 0; count = 0
   do while var("entries."count+1) = 1
      count = count + 1
      s = self~getTextSizeDlg(entries.count)
      if s.width > ll then ll = s~width
   end

   if max = 0 then max = count
   dlgy = max * trunc(sy * 1.5) + sy + 60
   placex = 10
   columns = (count % max)
   if  (count // max) > 0 then columns = columns +1

   if sx >= (ll + 20) * columns then
   do
      dlgx = (sx+24)
      if columns = 1 then placex = trunc((dlgx - ll - 20) /2)
   end
   else dlgx = (ll + 20) * columns + 24
   if dlgx < 110 then dlgx = 110

   self~createCenter(dlgx, dlgy, title,,,,, count+5)
   self~execute:super("SHOWTOP")
   return self~InitCode


/* The dialog this class shows has a group of radio buttons. That is you only can */
/* select one item at the same time. */

::class 'SingleSelection' subclass PlainUserDialog public

  /*  */
::method Init
   expose message title entries. len datas. prev max
   use arg message, title, entries., prev, len, max
   if Arg(5,"o") = 1 then len = 0
   if Arg(6,"o") = 1 then max = 0
   if Arg(4,"o") = 1 then prev = 1
   self~init:super(datas.)

::method DefineDialog
   expose message sx sy len dlgx dlgy entries. placex max
   self~addText(10,10, sx+2, sy, message)
   self~AddRadioStem(101, placex, sy+4+10, len, entries., max)
   self~AddButtonGroup(dlgx-100, dlgy - 18, 0, 0, "&Ok 1 OK &Cancel 2 CANCEL", 1, "DEFAULT")

::method Execute
   expose message title sx sy datas. dlgx dlgy len entries. placex prev max

   s = self~getTextSizeDlg(message)
   sx = s~width
   sy = s~height
   ll = 0; count = 0
   do while var("entries."count+1) = 1
      count = count + 1
      s = self~getTextSizeDlg(entries.count)
      if s~width > ll then ll = s~width
   end

   if max = 0 then max = count
   dlgy = max * trunc(sy * 1.5) + sy + 60
   placex = 10
   columns = (count % max)
   if  (count // max) > 0 then columns = columns +1

   if sx >= (ll + 20) * columns then
   do
      dlgx = (sx+24)
      if columns = 1 then placex = trunc((dlgx - ll - 20) /2)
   end
   else dlgx = (ll + 20) * columns + 24
   if dlgx < 110 then dlgx = 110

   do i = 101 to 100+count
      datas.i = 0
   end
   pr = (100+prev)
   datas.pr = 1

   self~createCenter(dlgx, dlgy, title,,,,, count+5)
   self~execute:super("SHOWTOP")
   if self~InitCode = 1 then
   do
     do i = 101 to 100+count
      if datas.i = 1 then return i-100
     end
     return 0
   end
   else
    return ""






