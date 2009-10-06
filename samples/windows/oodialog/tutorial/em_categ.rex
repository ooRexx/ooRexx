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
/* Name: EM_CATEG.REX                                                       */
/* Type: Object REXX Script                                                 */
/*                                                                          */
/* Description:  Sample to demonstrate the category dialog class            */
/*                                                                          */
/****************************************************************************/


 dlg = .EmployeeDialog~new(,,,,"TOPLINE WIZARD")
 if dlg~InitCode \= 0 then do; say "Dialog init did not work"; exit; end
 dlg~createcenter(280, 160, "Employee Dialog")
 dlg~execute("SHOWTOP")
 dlg~deinstall
 return

/*-------------------------------- requires -----------------------------------*/

::requires "ooDialog.cls"

/*-------------------------------- Employee Dialog ------------------------------*/

::class EmployeeDialog subclass CategoryDialog

::method Employees attribute
::method Emp_count attribute
::method Emp_current attribute


::method InitDialog
   self~InitDialog:super
   self~Employees = .array~new(10)
   self~Emp_count = 1
   self~Emp_current = 1

   self~City = "New York"
   self~Male = 1
   self~Female = 0
   self~AddCategoryComboEntry(22, "Munich", 1)
   self~AddCategoryComboEntry(22, "New York", 1)
   self~AddCategoryComboEntry(22, "San Francisco", 1)
   self~AddCategoryComboEntry(22, "Stuttgart", 1)
   self~AddCategoryListEntry(23, "Business Manager", 1)
   self~AddCategoryListEntry(23, "Software Developer", 1)
   self~AddCategoryListEntry(23, "Broker", 1)
   self~AddCategoryListEntry(23, "Police Man", 1)
   self~AddCategoryListEntry(23, "Lawyer", 1)

   self~connectEachSBEvent(44, "Emp_Previous", "Emp_Next")
   self~DisableCategoryItem(44, 1)
   self~SetCategoryListTabulators(101, 98, 198, 2)


::method InitCategories
   self~catalog['names'] = .array~of("Input", "List")
	/* set the width of the button row at the bottom to 35 */
   self~catalog['page']['btnwidth'] = 35
        /* change name of wizard buttons, default is &Backward and &Forward */
   self~catalog['page']['leftbtntext'] = "&Input"
   self~catalog['page']['rightbtntext'] = "&List"

::method Input                                      /* page 1 */
   self~loaditems("em_categ.rc", 100)
   self~connectButtonEvent(40, "CLICKED", "Print")
   self~connectButtonEvent(42, "CLICKED", "Add")

::method List                                       /* page 2 */
   self~loaditems("em_categ.rc", 101)


::method Print
    self~GetData
    if self~Male = 1 then title = "Mr."; else title = "Ms."
    if self~Married = 1 then addition = " (married) "; else addition = ""
    call infoDialog title self~Name addition || "A"x || "City:" self~City || "A"x || "Profession:" self~Profession

::method Add
    self~Employees[self~Emp_count] = .directory~new
    self~Employees[self~Emp_count]['NAME'] = self~getControlDataPage(21, 1)
    self~Employees[self~Emp_count]['CITY'] = self~getControlDataPage(22, 1)
    self~Employees[self~Emp_count]['PROFESSION'] = self~getControlDataPage(23, 1)
    if self~GetCategoryValue(31, 1) = 1 then sex = 1; else sex = 2
    self~Employees[self~Emp_count]['SEX'] = sex
    self~Employees[self~Emp_count]['MARRIED'] = self~getControlDataPage(41, 1)
    self~Emp_count = self~Emp_count +1
    self~Emp_current = self~Emp_count
    self~setControlDataPage(21, "", 1);
    self~SetSBRange(44, 1, self~Emp_count)
    self~SetSBPos(44, self~Emp_count)
    self~EnableCategoryItem(44, 1)


::method Set
    self~setControlDataPage(21, self~Employees[self~Emp_current]['NAME'], 1)
    self~setControlDataPage(22, self~Employees[self~Emp_current]['CITY'], 1)
    self~setControlDataPage(23, self~Employees[self~Emp_current]['PROFESSION'], 1)
    if self~Employees[self~Emp_current]['SEX'] = 1 then do
       self~setControlDataPage(31, 1, 1);self~setControlDataPage(32, 0, 1); end
    else do
       self~setControlDataPage(31, 0, 1);self~setControlDataPage(32, 1, 1); end
    self~setControlDataPage(41, self~Employees[self~Emp_current]['MARRIED'], 1)

::method Emp_Previous
   if self~Emp_count = 1 then return
   if self~Emp_current > 1 then do
       self~Emp_current = self~Emp_current - 1
       self~SetSBPos(44, self~Emp_current)
       self~Set
   end; else
       call TimedMessage "You reached the top!","Info",1000

::method Emp_Next
   if self~Emp_count = 1 then return
   if self~Emp_current < self~Emp_count-1 then do
       self~Emp_current = self~Emp_current + 1
       self~SetSBPos(44, self~Emp_current)
       self~Set
   end; else
       call TimedMessage "You reached the bottom!","Info",1000


::method FillList
   use arg id
   do i = 1 to self~Emp_count-1
       if self~Employees[i]['SEX'] = 1 then title = "Mr."; else title = "Ms."
       addstring = title self~Employees[i]['NAME']
       addstring = addstring || "9"x || self~Employees[i]['PROFESSION']
       addstring = addstring || "9"x || self~Employees[i]['CITY']
       self~AddCategoryListEntry(id, addstring, 2)
   end

::method PageHasChanged
   NewPage = self~CurrentCategory
   if NewPage = 1 then do
      self~Emp_current = self~GetCurrentCategoryListIndex(101, 2)
      if self~Emp_current > 0 then do
         self~SetSBPos(44, self~Emp_current)
         self~Set
      end
   end
   else do
      self~CategoryListDrop(101, 2)
      self~FillList(101)
   end

