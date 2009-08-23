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
use arg outdir
if arg() = 0 then outdir = value("OR_OUTDIR","","ENVIRONMENT")
parse source . . progname
inpdir = left(progname, progname~lastpos("\"))
p = time('R')
-- say '----------------------------------------------------------------'
-- say 'OOdialog build started                  ' time() 'on' date()
-- say '----------------------------------------------------------------'
outname = .array~new(3)
outname[1] = "OODPLAIN"
outname[2] = "OODIALOG"
outname[3] = "OODWIN32"
UtilName = "OODUTILS.CLS"
.Local["UtilName"] = UtilName
Arrax = .Array~new(3)
Arrax[1] = .CheckArray~of("PLBDLG.CLS", "DYNDLG.CLS", "PLUDLG.CLS", "STDDLG.CLS")

Arrax[2] = .CheckArray~of("DLGEXT.CLS", "BASEDLG.CLS", "RESDLG.CLS", "USERDLG.CLS", "RCDIALOG.CLS", "MENU.CLS", -
                          "CATDLG.CLS", "ANIBUTTN.CLS", "DLGAREA.CLS")

Arrax[3] = .CheckArray~of("ADVCTRL.CLS", "STDEXT.CLS", "MSGEXT.CLS", "PROPSHT.CLS")

i = 0
do j over Arrax
    i += 1
    NewFile = .stream~new(outname[i] || ".CLS")
    if NewFile~open("WRITE REPLACE") \= "READY:" then leave
    NewFile~lineout("/"||"*"~copies(78)||"/")
    NewFile~lineout("/*"||" "~copies(76)||"*/")
    NewFile~lineout("/*"||center(outname[i]||".CLS - OODialog Class Definition File",76)||"*/")
    NewFile~lineout("/*"||center("Windows Dialog Interface for Open Object REXX",76)||"*/")
    NewFile~lineout("/*"||" "~copies(76)||"*/")
    NewFile~lineout("/"||"*"~copies(78)||"/")
    NewFile~lineout("")

    if i = 1 then do
       NewFile~lineout("")
       NewFile~lineout("::requires 'oodialog' LIBRARY")
       NewFile~lineout("")
       call ProcessUtils
    end
    else do
       NewFile~lineout('::requires "'outname[i-1]'.CLS"')
    end
    do file over j
       say (inpdir || file)
       ReadFile = .stream~new(inpdir || File)
       ReadFile~open("READ")
       do while ReadFile~lines() > 0
           s = ReadFile~linein
           if s~pos('::requires') > 0 then do
              parse upper value s with . '"' filename '"' .
              isincl = 0
              do jj over Arrax
                  if jj~HasEntry(filename) = 1 then isincl = 1
              end
              if isincl = 0 then NewFile~lineout(s)
          end
          else NewFile~lineout(s)
       end
       ReadFile~close
    end
    NewFile~close
end


-- say '----------------------------------------------------------------'
-- say 'Build ended after: ' time('E') 'sec  ' ' at:' time() 'on' date()
-- say '----------------------------------------------------------------'
-- say

exit


ProcessUtils:
    UtilFile = .stream~new(inpdir || UtilName)
    if UtilFile~open("READ") \= "READY:" then do
        say "Couldn't process" inpdir || UtilName
        exit
    end
    do while UtilFile~lines > 0
        s = UtilFile~Linein
        NewFile~lineout(s)
    end
    return


::class CheckArray subclass array

::method HasEntry
  use arg srch
  do x over self
     if x = srch then return 1
  end
  if (srch = .Local["UtilName"]) then return 1
  return 0


