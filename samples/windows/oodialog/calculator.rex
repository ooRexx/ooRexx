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
/* Sample that demonstrates the usage of RxMath and ooDialog                */
/*                                                                          */
/****************************************************************************/


/* create the dialog */
calcDlg = .Calculator~new

/* Set the defaults for arithmetic operations (optional) */
NUMERIC DIGITS 9            /* precision can be up to 16 digits for RxMath */
NUMERIC FORM   SCIENTIFIC   /* controls exponential notation */
NUMERIC FUZZ   0            /* number of digits ignored for numeric comparison */

/* Display the dialog */
if calcDlg~initCode = 0 then do
  rc = calcDlg~execute("SHOWTOP")
end

exit   /* leave program */

/* error handling: Display the cause of the error and restart the programm */
any:
  call errorDialog "Error" rc "occurred at line" sigl":" errortext(rc) "a"x condition("o")~message
  if calcDlg~isDialogActive then do
     calcDlg~finished = .true
     calcDlg~stopDialog
  end
  signal reStart

::requires "ooDialog.cls"

/* This requires loads the RxMath functions. */
::requires "rxmath" library

/* The Calculator dialog class */
::class 'Calculator' subclass UserDialog

::method init
  forward class (super) continue /* call parent constructor */
  InitRet = Result

  if self~load("rc\CALCULATOR.RC", ) \= 0 then do
     self~initCode = 1
     return 1
  end

  /* Connect dialog control items to class methods */
  self~connectButtonEvent("BNO1", "CLICKED", "BNO1")
  self~connectButtonEvent("BNO2", "CLICKED", "BNO2")
  self~connectButtonEvent("BNO3", "CLICKED", "BNO3")
  self~connectButtonEvent("BNO4", "CLICKED", "BNO4")
  self~connectButtonEvent("BNO5", "CLICKED", "BNO5")
  self~connectButtonEvent("BNO6", "CLICKED", "BNO6")
  self~connectButtonEvent("BNO7", "CLICKED", "BNO7")
  self~connectButtonEvent("BNO8", "CLICKED", "BNO8")
  self~connectButtonEvent("BNO9", "CLICKED", "BNO9")
  self~connectButtonEvent("BNO0", "CLICKED", "BNO0")
  self~connectButtonEvent("BSIGN", "CLICKED", "BSIGN")
  self~connectButtonEvent("BPOINT", "CLICKED", "BPOINT")
  self~connectButtonEvent("BDIVIDE", "CLICKED", "BDIVIDE")
  self~connectButtonEvent("BTIMES", "CLICKED", "BTIMES")
  self~connectButtonEvent("BMINUS", "CLICKED", "BMINUS")
  self~connectButtonEvent("BPLUS", "CLICKED", "BPLUS")
  self~connectButtonEvent("BSQRT", "CLICKED", "BSQRT")
  self~connectButtonEvent("BLOG", "CLICKED", "BLOG")
  self~connectButtonEvent("BLOG10", "CLICKED", "BLOG10")
  self~connectButtonEvent("BPI", "CLICKED", "BPI")
  self~connectButtonEvent("BBACKSPACE", "CLICKED", "BBACKSPACE")
  self~connectButtonEvent("BCLEAR", "CLICKED", "BCLEAR")
  self~connectButtonEvent("BCALC", "CLICKED", "BCALC")
  self~connectButtonEvent("BSINUS", "CLICKED", "BSINUS")
  self~connectButtonEvent("BCOSINUS", "CLICKED", "BCOSINUS")
  self~connectButtonEvent("BTANGENS", "CLICKED", "BTANGENS")
  self~connectButtonEvent("BARCSIN", "CLICKED", "BARCSIN")
  self~connectButtonEvent("BARCCOS", "CLICKED", "BARCCOS")
  self~connectButtonEvent("BARCTAN", "CLICKED", "BARCTAN")

  /* Initial values that are assigned to the object attributes */
  self~TLine= '0' /* set text-line to 0 initially */

  /* Add your initialization code here */
  return InitRet

::method initDialog
  expose tl
  tl = self~newEdit(TLine) /* get the EditControl object */

::method getLine
  /* Return the current text-line content, */
  /* or 0 if infinity has been reached.    */

  expose tl
  if tl~getText~Left(5) = 'ERROR' then
    return 0
  return tl~getText

::method setLine
  /* Set the argument as new text-line. If this is ERROR raise an */
  /* errorDialog with additional information.                    */

  expose tl
  use arg line, merror
  if line~left(5) = 'ERROR' then
    call errorDialog "RxCalc returned an error:" merror
  tl~setText(line)

::method justZero
  /* Return true if the current text-line is only one 0.   */
  /* Most functions will then ignore the current text-line */

  line = self~getLine
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
    self~setLine(number)
  else self~setLine(self~getLine||number)

::method BSIGN
  /* Toggles the sign of the leading number on the text line. */

  if self~justZero then
    self~setLine('-')
  else do
    select
      when left(self~getLine,1)='-' then
        self~setLine('+'||substr(self~getLine,2))
      when left(self~getLine,1)='+' then
        self~setLine('-'||substr(self~getLine,2))
      otherwise
        self~setLine('-'||self~getLine)
    end
  end

::method BPOINT
  /* Append a point.. */
  self~setLine(self~getLine||'.')

::method BDIVIDE
  /* Appends a 'divide' symbol to the checked line. */
  self~setLine(self~GetCheckedLine||'/')

::method BTIMES
  /* Append a 'multiply' symbol to the checked line. */
  self~setLine(self~GetCheckedLine||'*')

::method BMINUS
  /* Appends a 'minus' symbol to the checked line. */
  self~setLine(self~GetCheckedLine||'-')

::method BPLUS
  /* Appends a 'plus' symbol to the checked line. */
  self~setLine(self~GetCheckedLine||'+')

::method BSQRT
  /* Displays the square root of the checked line.          */
  /* MATHERRNO is filled with additional information if the */
  /* RxMath-funtion detects an error.                       */
  self~setLine(RxCalcSqrt(self~GetCheckedLine), MATHERRNO)

::method BLOG
  /* Displays the natural logarithm of the checked line */
  self~setLine(RxCalcLog(self~GetCheckedLine), MATHERRNO)

::method BLOG10
  /* Displays the 10-base logarithm of the checked line */
  self~setLine(RxCalcLog10(self~GetCheckedLine), MATHERRNO)

::method BPI
  /* Displays the number Pi */
  if self~justZero then
    self~setLine(RxCalcPi(), MATHERRNO)
  else self~setLine(self~getLine||RxCalcPi(), MATHERRNO)

::method BBACKSPACE
  /* Delete the last character of the line */
  line = self~getLine
  line = line~Left(line~Length - 1)
  if line = '' then     /* if the line is empty set it to 0 */
      self~setLine(0)
  else self~setLine(line)

::method BCLEAR
  /* Set the line to 0 */
  self~setLine(0)

::method BCALC
  /* Interpret the current line = calculate the result */
  interpret 'calcResult =' self~getLine
  self~setLine(calcResult)

::method BSINUS
  /* Display the sine of the checked line */
  self~setLine(RxCalcSin(self~getCheckedLine), MATHERRNO)

::method BCOSINUS
  /* Display the cosine of the checked line */
  self~setLine(RxCalcCos(self~GetCheckedLine), MATHERRNO)

::method BTANGENS
  /* Display the tangent of the checked line */
  self~setLine(RxCalcTan(self~GetCheckedLine), MATHERRNO)

::method BARCSIN
  /* Display the arc sine of the checked line */
  self~setLine(RxCalcArcSin(self~GetCheckedLine), MATHERRNO)

::method BARCCOS
  /* Display the arc cosine of the checked line */
  self~setLine(RxCalcArcCos(self~GetCheckedLine), MATHERRNO)

::method BARCTAN
  /* Display the arc tangent of the checked line */
  self~setLine(RxCalcArcTan(self~GetCheckedLine), MATHERRNO)

::method Ok
  /*
    This is the method connected to our exit-button. You don't have to implement it,
    ooDialog supplies a default implementation of the ok method.  Note that the ok
    method is always connected to the button with resource ID 1.  Note also, that
    once you invoke the superclass ok method, if validate() returns true, the dialog
    will close.

    This is what the default implementation of ok does: it invokes the validate()
    method.  If validate() returns false, then ok() does nothing and just returns.
    If validate() returns true then ok() sets self~initCode to 1 and
    self~finished to true, which ends the dialog.

    This over-ride sets the calculator display to 0 before ending the dialog.  Again,
    this is not necessary, it is just done to demonstrate how to over-ride the ok
    method
  */
  if \ self~validate then return 0

  self~setLine(0)
  self~initCode = 1
  self~finished = .true
  return self~finished
