/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2011-2014 Rexx Language Association. All rights reserved.    */
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
/* ooDialog User Guide

   Support - Model						 v01-00  27Feb13
   ----------------
   A superclass for the Model-View framework.

   v01-00 09Aug12: First version.
          11Jan13: Commented-out "say"s.
          31Jan13: Store model's data in 'myData'.
          27Feb13: Commented-out some "say" instructions.

  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

--::REQUIRES "ooDialog.cls"
--::REQUIRES "ObjectMgr.rex"

/*============================================================================*/

::CLASS 'Model' PUBLIC

  ::ATTRIBUTE wantList     CLASS PUBLIC		-- for List subclasses
  ::ATTRIBUTE myData

  /*----------------------------------------------------------------------------
    newInstance - must be invoked by subclass.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD newInstance CLASS PUBLIC
    expose noDataError				-- .true if data not found.
    use strict arg instanceName
    --say ".Model-newInstance-01: instanceName =" instanceName
    -- Check that the model's Data object is up and running. If not, then return .false:
    if noDataError = .true then return .false
    -- Now get the name of the Data component (FredModel or FredListModel --> FredData):
    -- Get my root name (i.e. the root name of the subclass):
    className = self~objectName		-- objectName for a class Foo is "The Foo class"
    className = className~upper()	-- When class name is in quotes, then it's mixed case.
    	    				-- Upper here to make everthing upper case for parse var.
    -- Handling Forms:
    -- If this is a "Form" then there's no data to get (the user will provide
    -- the data). So just create the Form Model (e.g. Order Form) and return.
    -- Assume that the instance name is the Form Number (e.g. for an OrderForm,
    -- the Form Number will be the new Order Number).
    p = className~pos("FORM")
    if p > 0 then do	-- if this is a "Form" component.
      instData = .Directory~new
      instData[formNumber] = instanceName
      formObject = self~new(instData)
      formObject~myData = instData		-- store instance data for subclasses to access.
      --say ".Model-newInstance-011: formObj, instanceName =" formObject||"," instanceName
      return formObject
    end
    -- End of Handling Forms.

    -- If there's  "LIST" in the name, then set "get all" for the file access
    --  (as opposed to the default of "get 1 record")
    getAllRecords = .false
    p = className~pos("LIST")
    if p > 0 then getAllRecords = .true
    -- if there's a "LIST" in the name, strip it out 'cos an xListModel gets data from xData
    parse var className . root1 "MODEL" .
    parse var root1 root "LIST"
    dataClassName = root||"Data"
    --say ".Model-newInstance-02, dataClassName =" dataClassName
    -- Get the id of Data component:
    objMgr = .local~my.ObjectMgr
    --say ".Model-newInstance-03: objMgr =" objMgr
    myDataId = objMgr~getComponentId(dataClassName, "The")
    if myDataId = .false then do		-- if instantiation failed
      say ".Model-newInstance-04: getting ID of Data Class failed."
      noDataError = .true
      return .false
    end
    -- Got my data id, now get data for this model instance.
    -- But distinguish between Entity Models and List Models - the former needs
    -- a single record, the latter a group of records.
    -- say ".Model-newInstance-05a: getAllRecords =" getAllRecords
    if getAllRecords then instData = myDataId~getFile()	-- returns a 2D array
    else instData = myDataId~getRecord(instanceName)		-- a directory
    -- say ".Model-newInstance-05b: array dimensions: =" instData~dimension
    if instData = .false then return .false	-- if ID (key) not found
    -- All is well, then make new instance:
    --say ".Model-newInstance-06: instData =" instData
    id = self~new(instData)
    id~myData = instData
    --say ".Model-newInstance-07: instData =" id
    return id
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


  /*----------------------------------------------------------------------------
    getInstanceName - For an "anonymous" instance only (e.g. CustomerList)
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD getInstanceName CLASS PUBLIC
    expose anonInstanceName
    if anonInstanceName = "ANONINSTANCENAME" then anonInstanceName = 1
    else anonInstanceName = anonInstanceName + 1
    return anonInstanceName


  /*----------------------------------------------------------------------------
    query - returns a Model's data.
            Standard protocol:
            Accept a .nil, directory, array, or string of names (case-sensitive).
            if .nil then return all fields; else return values for the names in
            the directory, array, or string. String is assumed to be data
            names separated by one or more spaces.
            All returns are a Directory containing names and values.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD query PUBLIC
    use arg dataNames
    --say "Model-query-01: dataNames:" dataNames
    dirReturn = .Directory~new
    select
      when dataNames = .nil | dataNames = "" then return self~myData

      when dataNames = "DATANAMES" then return self~myData

      -- Caller is requesting specific data items:
      when dataNames~isa(.Directory) then do
        --say "Model-query-02; dataNames =" dataNames
        do i over dataNames
      	  dirReturn[i] = self~myData[i]
        end
      end

      when dataNames~isa(.Array) then do
        do i over dataNames
          dirReturn[i] = self~myData[i]
        end
      end

      when dataNames~isa(.String) then do
        dataNames = dataNames~strip
        n = dataNames~countStr(" ")+1
        do i = 1 to n
          parse var dataNames name " " dataNames
          if name = " " then iterate     -- ignore extraneous leading spaces.
          dirReturn[name] = self~myData[name]
        end
      end

      otherwise return .false
    end

    return dirReturn

/*============================================================================*/


/*============================================================================*/
