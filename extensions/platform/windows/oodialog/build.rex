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
say "This Object REXX script creates a file that contains all OODialog classes."
say "The file is stored in the current directory and the tokenized file"
say "is stored in the parent directory."
say "It's not allowed to name the output file OODIALOG.CLS, OODPLAIN.CLS, or OODWIN32.CLS!"
say
say "Please enter the name for the output file"
pull outname
if outname~pos(":") > 0 | outname~pos("\") > 0 then do
   say "The name of the output file cannot contain a directory/drive!"
   exit
end
if outname~wordpos("OODWIN32.CLS") > 0 | outname~wordpos("OODIALOG.CLS") > 0 | outname~wordpos("OODPLAIN.CLS") > 0 then do
   say "The name of the output file cannot be" outname"!"
   exit
end
outdir = "..\"
p = time('R')
-- say '----------------------------------------------------------------'
-- say 'OOdialog build started                  ' time() 'on' date()
-- say '----------------------------------------------------------------'
Arrax = .CheckArray~of("OODUTILS.CLS", "PLBDLG.CLS","DYNDLG.CLS","PLUDLG.CLS","STDDLG.CLS",,
          "DLGEXT.CLS", "DLGAREA.CLS", "BASEDLG.CLS","RESDLG.CLS","USERDLG.CLS","CATDLG.CLS","ANIBUTTN.CLS",,
          "ADVCTRL.CLS","MSGEXT.CLS", "STDEXT.CLS", "PROPSHT.CLS")

NewFile = .stream~new(outname)
NewFile~open("WRITE REPLACE")
NewFile~lineout("/* This OODIALOG class library was rebuilt" Date("L")". */")
NewFile~lineout("")

do file over Arrax
 say file
 ReadFile = .stream~new(File)
 ReadFile~open("READ")
 do while ReadFile~lines() > 0
   s = ReadFile~linein
   if s~pos('::requires') > 0 then do
     parse upper value s with . '"' filename '"' .
     if Arrax~HasEntry(filename) = 0 then
        NewFile~lineout(s)
   end
   else NewFile~lineout(s)
 end
 ReadFile~close
end
NewFile~close

"rexxc" outname outdir||outname "/s"

-- say '----------------------------------------------------------------'
-- say 'Build ended after: ' time('E') 'sec  ' ' at:' time() 'on' date()
-- say '----------------------------------------------------------------'
-- say
if (rc = 0) then say outdir||outname "should be used as the ::Requires file."

exit

::class CheckArray subclass array

::method HasEntry
  use arg srch
  do x over self
     if x = srch then return 1
  end
  return 0



