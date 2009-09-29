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

/**
 * Purpose: Builds the ooDialog package file: ooDialog.cls from the individual
 *          *.cls files that make up the implementation code.
 *
 * Note that the code for reading the input files assumes that each file starts
 * with the license / copyright header, which is 36 or 37 lines long.
 */

use arg outdir
if arg() = 0 then do
  outdir = value("OR_OUTDIR","","ENVIRONMENT")
  if outdir \== "" & outdir~right(1) \== '\' then outdir ||= '\'
end
else do
  if outdir~right(1) \== '\' then outdir ||= '\'
end

parse source . . progname
inpdir = left(progname, progname~lastpos("\"))

outname = .array~new(3)
outname[1] = "oodPlain.cls"
outname[2] = "oodWin32.cls"
outname[3] = "ooDialog.cls"

do i = 1 to 2
  ret = writeStubFile(outdir, outname[i])
  if ret <> 0 then do
    say 'Error writing' outname[i] 'aborting.'
    return 9
  end
end

-- Files are in the order they are read and written out. oodutils.cls must be kept first, otherwise
-- the order should not make any difference.
srcFiles = .array~of("oodutils.cls", "plbdlg.cls", "dyndlg.cls", "pludlg.cls", "stddlg.cls", "dlgext.cls",   -
                     "basedlg.cls", "resdlg.cls", "userdlg.cls", "rcdialog.cls", "dialog.cls", "Menu.cls",   -
                     "catdlg.cls", "anibuttn.cls", "dlgarea.cls", "advctrl.cls", "stdext.cls", "msgext.cls", -
                     "propsht.cls")

outFile = .stream~new(outdir || outname[3])
if outFile~open("WRITE REPLACE") \= "READY:" then return 9

signal on notready

do i = 1 to 37
  outFile~lineout(sourceLine(i))
end
outFile~lineout("")
outFile~lineout("::requires 'oodialog' LIBRARY")
outFile~lineout("")

do inFile over srcFiles
  inputFile = inpdir || inFile
  say inputFile
  readFile = .stream~new(inputFile)
  readFile~open("READ")

  -- Skip the header, with a little leeway.
  do 35
    readFile~linein
  end

  line = readFile~linein~strip
  do while line~left(2) == '/*', line~right(2) == '*/', readFile~lines() > 0
    line = readFile~linein
  end

  outFile~lineout("")

  do while readFile~lines() > 0
    line = readFile~linein
    if line~pos('::requires') > 0 then iterate
    else outFile~lineout(line)
  end
  readFile~close
end
outFile~close

return 0

notready:
  say "Stream I/O error while creating ooDialog.cls"
return 9

::routine writeStubFile
  use strict arg outDir, outFileName

  stubFile = .stream~new(outdir || outFileName)
  if stubFile~open("WRITE REPLACE") \= "READY:" then return 9

  do i = 1 to 37
    stubFile~lineout(sourceLine(i))
  end
  stubFile~lineout("")
  stubFile~lineout("::requires 'ooDialog.cls'")
  stubFile~close

  return 0
