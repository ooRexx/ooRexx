/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2011-2014 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                                        */
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
   Exercise07 The WowData component.			  	  v02-00 01Apr13

   Contains:       Classes: WowData.

   Pre-requisites: None.

   Description:    The data component of the "Words of Wisdom" app.

   Changes:
     v01-00 31May12: First version.
     v02-00 06Sep12: Second version - modified to use the Model-View Framework (MVF)
            21Jan13: Updated comments. No change in funtion.
            01Apr13: After ooDialog 4.2.2, Support folder moved to exercise
                     folder, so change to ::Requires needed.

------------------------------------------------------------------------------*/

/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  WowData							  v02-00 13Feb13
  -------
  Has access to WOW data, and returns a set to requester.

  Potential enhancement: Define the size of a set through configuration.

  Changes:
    v01-01 31May12: First version.
    v02-00 06Sep12: Second version - modified to use the Model-View Framework (MVF)
           13Feb13: Changes to comments only.
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::REQUIRES "..\Exercise07\Support\GenericFile.rex"			-- MVF

--::CLASS WowData PUBLIC
  ::CLASS WowData SUBCLASS GenericFile PUBLIC			-- v01-00-->MVF

  ::ATTRIBUTE created CLASS						-- MVF

  /*----------------------------------------------------------------------------
    newInstance - creates an instance of WowData. Since all data
                  components are singletons, MVF applies the instance
                  name "The".
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD newInstance CLASS PUBLIC					-- MVF
      use strict arg instanceName		-- the instanceName is "The".
      if self~created = "CREATED" then do	-- if not yet created
        WowDataId = self~new()
        return WowDataId
      end
      else do					-- if already created, then error.
        say ".WowData-newInstance-01 - Error: singleton component, so can't have more than one instance."
        return .false
      end

  /*----------------------------------------------------------------------------
    init - asks super to read the file from disk.
           Returns the number of records (exclusing the colums headers line)
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD init								-- MVF
    expose filename records
    filename = "Extras\Wow4\WowFile.txt"; columns = 1
    records = self~init:super(fileName, columns)
    return self


  /*----------------------------------------------------------------------------
    getRecord - asks super to read the file from disk.
                Normally for a 'named' component, MVF invokes 'getRecord'
                to ask the Data component for a single record (handled by the
                'GenericFile' superclass). For a ListModel, MVF invokes 'getFile'
                to get the whole file to display in a list.
                Here, although WowModel is a Model, not a ListModel, we want the
                whole file. And GenericFile holds the file in its 'fileAsDirectory'
                attribute. So we intercept the 'getRecord' message, and return
                the file.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD getRecord PUBLIC						-- MVF
    return self~fileAsDirectory


  /*----------------------------------------------------------------------------
    activate - Not used for a Data component in MVF		          v01-00
    reads initial Wow Set from disk (but not in this version)
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/*  ::METHOD activate
    expose arrWow
    arrWow = .array~new
    arrWow[1] = "Agnes Allen's Law:" -
	      	"Almost anything is easier to get into than out of."
    arrWow[2] = "Airplane Law:" -
    		"When the plane you are on is late," -
    		"the plane you want to transfer to is on time."
    arrWow[3] = "Fourteenth Corollary of Atwood's General Law of Dynamic Negatives:" -
  	      "No books are lost by loaning" -
  	      "except those you particularly wanted to keep."
    arrWow[4] = "Baker's Byroad: When you're over the hill, you pick up speed."
    arrWow[5] = "First Law of Bicycling:" -
  	      "No matter which way you ride, it's uphill and against the wind."
    arrWow[6] = "Brooks's Law:" -
  	      "Adding manpower to a late software project makes it later."
    arrWow[7] = "Grossman's Misquote of H. L. Mencken:" -
  	      "Complex problems have simple, easy-to-understand wrong answers."
    return */
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*----------------------------------------------------------------------------
    readWowSet - returns the Wow set as an array.			  v01-00
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  --::METHOD readWowSet
    --expose arrWow
    --return arrWow
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*============================================================================*/
