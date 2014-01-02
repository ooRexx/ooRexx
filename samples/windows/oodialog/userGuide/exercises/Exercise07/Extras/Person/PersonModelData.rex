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

   Samples:  Person Model and Data Classes	  	  	  v01-00 26May13

   Contains: classes "PersonModel" and "PersonData".

   Pre-requisites: Model-View Framework.

   Outstanding Problems:
   None.

   Changes:
   v01-00 01Oct12: First version.
          09Jan13: Removed or commented-out 'say' instructions.
          05Feb13: Removed 'query' method since it's available in the superclass.
          14Feb13: Removed (commented-out) 'return self' from the init method.
          01Apr13: After ooDialog 4.2.2, Support folder moved to exercise
                   folder, so change to ::Requires needed. Also, 'Samples' folder
                   changed name to 'Extras'.
          26May13: Corrected file path after re-factoring Extras folder.


------------------------------------------------------------------------------*/


::REQUIRES "..\Exercise07\Support\GenericFile.rex"
::REQUIRES "..\Exercise07\Support\Model.rex"


/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  PersonModel							  v01-00 09Jan13
  ------------

  The "model" part of the Person component - a simple "Model" class that
  illustrates use of the Model-View framework.
  See comments that include the string 'MFV'.

  Changes:
    v01-00 01Oct12: First version.
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::CLASS 'PersonModel' SUBCLASS Model PUBLIC

  ::METHOD newInstance CLASS PUBLIC
    use strict arg instanceName		-- Instance name of required instance is
    					-- provided by MVF as result of sending
    					-- a 'showModel' message to the Object Manager.
    forward class (super) continue	-- As part of MVF, super does the ~new
    					-- and provide's the model's instance
    					-- data as a param on the init.
    modelId = RESULT			-- MVF requires the model instance
    return modelId			-- ('modelId') to be returned.



  /*----------------------------------------------------------------------------
    init
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD init
    expose dirPerson
    use strict arg dirPerson	-- Super gets data from PersonData and passes
    				-- that data when it creates this instance with
    				-- '~new'.
    				-- The data is in a directory:

    self~myData = dirPerson	-- Store the data in my superclass.

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


  /*----------------------------------------------------------------------------
    test - a method to demonstrate Message Sender method store.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD test
    return 25

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */



/*============================================================================*/



/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  PersonData- The data resource for Persons. 			  v01-00 23Sep12

  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =*/

::CLASS 'PersonData' SUBCLASS GenericFile PUBLIC

  ::ATTRIBUTE created CLASS

  ::METHOD newInstance CLASS PUBLIC		-- Invoked by ObjectMgr
    --use strict arg instanceName
    if self~created = "CREATED" then do		-- If this is first time
      --say ".PersonData-newInstance-01."
      personDataId = self~new()			-- the object id of the PersonData component.
      self~created = .true
      return personDataId
    end
    else do
      say ".PersonData-newInstance-02 - Error - Singleton component, so can't have more than one instance."
      return .false
    end


  ::METHOD init PRIVATE
    fileName = "Extras\Person\PersonFile.txt"
    columns = 6					-- colums in the Persons "table"
    records = self~init:super(fileName, columns)
    --say "PersonData-init-01: records:" records


/*============================================================================*/



/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  PersonDT - A business data type for Person data.		  v00-01 05May12
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =*/

::CLASS PersonDT PUBLIC

  --		dtName		XML Name	Description
  --		---------	----------	-------------------------------
  --		ProductDT	product
  ::ATTRIBUTE	number		-- number	Person Number
  ::ATTRIBUTE	familyName	-- familyName	Person Family Name
  ::ATTRIBUTE	firstName	-- firstName	Person First Name
  ::ATTRIBUTE	dob		-- dob		Date of Birth (format: yymmdd)
  ::ATTRIBUTE   jobDescr	-- jobDescrip	Job Description
  ::ATTRIBUTE   baseSalary	-- baseSalary	Basic Salary

  ::METHOD makeDir
    dir = .Directory~new
    dir["number"]     = self~number
    dir["familyName"] = self~familyName
    dir["firstName"]  = self~firstName
    dir["dob"]        = self~dob
    dir["jobDescr"]   = self~jobDescr
    dir["baseSalary"] = self~baseSalary

    return dir

  ::METHOD list PUBLIC
    expose number name price uom description size
    say "---------------"
    say "PersonDT-List:"
    say "Number: " number "    Family Name:" familyName
    say "DOB:" dob "   First Name:" firstName
    say "Base Salary:" baseSalary
    say "Job:" jobDescr
    say "---------------"

/*============================================================================*/
