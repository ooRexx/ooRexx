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
/****************************************************************************/
/* Name: CALCULATOR.REX                                                     */
/* Type: Object REXX Script                                                 */
/* Resource: CALCULATOR                                                     */
/*                                                                          */
/* Description:                                                             */
/* Sample that demonstrates the usage of RxMath and OODialog                */
/*                                                                          */
/****************************************************************************/


reStart:
signal on any /* go to the error handling on any error that signal can catch */

/* create the dialog */
CalcDlg = .Calculator~new

/* Set the defaults for arithmetic operations (optional) */
NUMERIC DIGITS 9            /* precision can be up to 16 digits for RxMath */
NUMERIC FORM   SCIENTIFIC   /* controls exponential notation */
NUMERIC FUZZ   0            /* number of digits ignored for numeric comparison */

/* Display the dialog */
if CalcDlg~InitCode = 0 then do
  rc = CalcDlg~Execute("SHOWTOP")
end

CalcDlg~deinstall

exit   /* leave program */

/* error handling: Display the cause of the error and restart the programm */
any:
  call errorDialog "Error" rc "occurred at line" sigl":" errortext(rc) "a"x condition("o")~message
  if CalcDlg~IsDialogActive then
    CalcDlg~StopIt
  signal reStart

/* If you use a requires directive for oodwin32.cls, you don't need to use a
 * requires directive for any of the other ooDialog class files.  oodwin32.cls
 * has a requires directive for oodialog.cls, which in turn has a requires
 * directive for oodplain.cls.
 */
::requires "ooDialog.cls"

/* This requires loads the RxMath functions. */
::requires "rxmath" library

/* The Calculator dialog class */
/* Advanced controls are required to get the dialog-item objects */
::class Calculator subclass UserDialog inherit AdvancedControls

::method Init
  forward class (super) continue /* call parent constructor */
  InitRet = Result

  if self~Load("rc\CALCULATOR.RC", ) \= 0 then do
     self~InitCode = 1
     return 1
  end

  /* Connect dialog control items to class methods */
  self~ConnectButton("BNO1","BNO1")
  self~ConnectButton("BNO2","BNO2")
  self~ConnectButton("BNO3","BNO3")
  self~ConnectButton("BNO4","BNO4")
  self~ConnectButton("BNO5","BNO5")
  self~ConnectButton("BNO6","BNO6")
  self~ConnectButton("BNO7","BNO7")
  self~ConnectButton("BNO8","BNO8")
  self~ConnectButton("BNO9","BNO9")
  self~ConnectButton("BNO0","BNO0")
  self~ConnectButton("BSIGN","BSIGN")
  self~ConnectButton("BPOINT","BPOINT")
  self~ConnectButton("BDIVIDE","BDIVIDE")
  self~ConnectButton("BTIMES","BTIMES")
  self~ConnectButton("BMINUS","BMINUS")
  self~ConnectButton("BPLUS","BPLUS")
  self~ConnectButton("BSQRT","BSQRT")
  self~ConnectButton("BLOG","BLOG")
  self~ConnectButton("BLOG10","BLOG10")
  self~ConnectButton("BPI","BPI")
  self~ConnectButton("BBACKSPACE","BBACKSPACE")
  self~ConnectButton("BCLEAR","BCLEAR")
  self~ConnectButton("BCALC","BCALC")
  self~ConnectButton("BSINUS","BSINUS")
  self~ConnectButton("BCOSINUS","BCOSINUS")
  self~ConnectButton("BTANGENS","BTANGENS")
  self~ConnectButton("BARCSIN","BARCSIN")
  self~ConnectButton("BARCCOS","BARCCOS")
  self~ConnectButton("BARCTAN","BARCTAN")
  self~ConnectButton(1,"OK")

  /* Initial values that are assigned to the object attributes */
  self~TLine= '0' /* set text-line to 0 initially */

  /* Add your initialization code here */
  return InitRet

::method InitDialog
  InitDlgRet = self~InitDialog:super

  /* Initialization Code (e.g. fill list and combo boxes) */
  return InitDlgRet

::method run
  /* The run message is not automatically generated.      */
  /* If you override it you have to call self~run:super.  */
  /* It is the place for things that can't be done in the */
  /* init method, e.g. SetCurrentComboIndex.              */

  expose tl
  tl = self~GetEditControl(TLine) /* get the EditControl object */
  self~run:super

::method getLine
  /* Return the current text-line content, */
  /* or 0 if infinity has been reached.    */

  expose tl
  if tl~Title~Left(5) = 'ERROR' then
    return 0
  return tl~Title

::method setLine
  /* Set the argument as new text-line. If this is ERROR raise an */
  /* errorDialog with additional information.                    */

  expose tl
  use arg line, merror
  if line~left(5) = 'ERROR' then
    call errorDialog "RxCalc returned an error:" merror
  tl~Title= line

::method justZero
  /* Return true if the current text-line is only one 0.   */
  /* Most functions will then ignore the current text-line */

  line = self~GetLine
  if line~Length = 1 & line = 0 then
    return 1
  return 0

::method getCheckedLine
  /* Return the result of the current calculation or the line  */
  /* if it is only a number. DataType will not return NUM      */
  /* if any operators are present.                             */

  line = self~getLine
  if DataType(line) = 'NUM' then
    return line
  else do
    interpret 'calcResult =' line
    return calcResult
  end

/* --------------------- message handler -----------------------------------*/

::method UNKNOWN
  /* The UNKNOWN-method is called, whenever the defined message for a       */
  /* button cannot be found. This is a good way to roll all the methods for */
  /* the 10 digits into one. The last character of the argument is the      */
  /* number that was pressed.                                               */

  use arg message
  number = message~SubStr(4)
  if self~justZero then
    self~SetLine(number)
  else self~SetLine(self~GetLine||number)

::method BSIGN
  /* Toggles the sign of the leading number on the text line. */

  if self~justZero then
    self~setLine('-')
  else do
    select
      when left(self~GetLine,1)='-' then
        self~setLine('+'||substr(self~GetLine,2))
      when left(self~GetLine,1)='+' then
        self~setLine('-'||substr(self~GetLine,2))
      otherwise
        self~setLine('-'||self~GetLine)
    end
  end

::method BPOINT
  /* Append a point.. */
  self~SetLine(self~GetLine||'.')

::method BDIVIDE
  /* Appends a 'divide' symbol to the checked line. */
  self~SetLine(self~GetCheckedLine||'/')

::method BTIMES
  /* Append a 'multiply' symbol to the checked line. */
  self~SetLine(self~GetCheckedLine||'*')

::method BMINUS
  /* Appends a 'minus' symbol to the checked line. */
  self~SetLine(self~GetCheckedLine||'-')

::method BPLUS
  /* Appends a 'plus' symbol to the checked line. */
  self~SetLine(self~GetCheckedLine||'+')

::method BSQRT
  /* Displays the square root of the checked line.          */
  /* MATHERRNO is filled with additional information if the */
  /* RxMath-funtion detects an error.                       */
  self~SetLine(RxCalcSqrt(self~GetCheckedLine), MATHERRNO)

::method BLOG
  /* Displays the natural logarithm of the checked line */
  self~SetLine(RxCalcLog(self~GetCheckedLine), MATHERRNO)

::method BLOG10
  /* Displays the 10-base logarithm of the checked line */
  self~SetLine(RxCalcLog10(self~GetCheckedLine), MATHERRNO)

::method BPI
  /* Displays the number Pi */
  if self~justZero then
    self~SetLine(RxCalcPi(), MATHERRNO)
  else self~SetLine(self~GetLine||RxCalcPi(), MATHERRNO)

::method BBACKSPACE
  /* Delete the last character of the line */
  line = self~GetLine
  line = line~Left(line~Length - 1)
  if line = '' then     /* if the line is empty set it to 0 */
      self~SetLine(0)
  else self~SetLine(line)

::method BCLEAR
  /* Set the line to 0 */
  self~SetLine(0)

::method BCALC
  /* Interpret the current line = calculate the result */
  interpret 'calcResult =' self~GetLine
  self~SetLine(calcResult)

::method BSINUS
  /* Display the sine of the checked line */
  self~SetLine(RxCalcSin(self~getCheckedLine), MATHERRNO)

::method BCOSINUS
  /* Display the cosine of the checked line */
  self~SetLine(RxCalcCos(self~GetCheckedLine), MATHERRNO)

::method BTANGENS
  /* Display the tangent of the checked line */
  self~SetLine(RxCalcTan(self~GetCheckedLine), MATHERRNO)

::method BARCSIN
  /* Display the arc sine of the checked line */
  self~SetLine(RxCalcArcSin(self~GetCheckedLine), MATHERRNO)

::method BARCCOS
  /* Display the arc cosine of the checked line */
  self~SetLine(RxCalcArcCos(self~GetCheckedLine), MATHERRNO)

::method BARCTAN
  /* Display the arc tangent of the checked line */
  self~SetLine(RxCalcArcTan(self~GetCheckedLine), MATHERRNO)

::method Ok
  /* This is the method connected to our exit-button. You don't have to implement it, */
  /* but if you want to override the OK method, this is what you do. The button has   */
  /* to have the ID 1!                                                                */
  resOK = self~OK:super  /* make sure self~Validate is called and self~InitCode is set to 1 */
  self~Finished = resOK  /* 1 means close dialog, 0 means keep open */
  return resOK
