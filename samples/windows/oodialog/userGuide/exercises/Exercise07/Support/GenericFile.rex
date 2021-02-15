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
   Exercise07: GenericFile					  v01-00 11Jan13

  Contains:	class "GenericFile".

  Pre-requisites: None.

  Desription: A component that opens and reads a text file that has a specific
              format.
  Outstanding problems: None reported.

  Changes:
  v01-00
    21Jly12: First version.
    25Aug12: Moved file open from init method to a separate method.
    16Dec12: Trivial correction of a couple of comments.
    11Jan13: Commented-out 'say' instructions.

  Constraints: Format of each record must be:
               recordId | field2 | field3 | .... | fieldn
               Note that the RecordId or Key must be in the first column.
  }
------------------------------------------------------------------------------*/


/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  GenericFile							  v01-01 24Aug12
  -----------
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::CLASS GenericFile PUBLIC

  ::ATTRIBUTE fileHeaders     GET PUBLIC	-- a 1D array of column labels/headers
  ::ATTRIBUTE fileHeaders     SET PRIVATE
  ::ATTRIBUTE fileRecords     GET PUBLIC	-- a 2D array of records
  ::ATTRIBUTE fileRecords     SET PRIVATE
  ::ATTRIBUTE fileArray       GET PUBLIC	-- a 1D array of raw data from the file.
  ::ATTRIBUTE fileArray       SET PRIVATE
  ::ATTRIBUTE fileAsDirectory GET PUBLIC 	-- a directory containing:
  					 	--   * headers: a 1D array
  					 	--   * records: a 2D array
  					 	--   * count:   the number of records
  ::ATTRIBUTE fileAsDirectory SET PRIVATE

  /*----------------------------------------------------------------------------
    Class Methods - none.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*----------------------------------------------------------------------------
    Instance Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    init - Given a filename and the number of columns in the file, loads the
           fileinto attribute 'fileArray', a 1-dimensional array where each item
           is a single "raw" file line or record.
           Returns the number of records in the file (or, if unsuccesful at reading
           the file, returns 0 (.false) if file doesn't exist, or -1 if no records
           in the file.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD init PUBLIC
    expose columns recordCount fileArray filename
    use arg filename, columns
    dirFile = self~readFile(fileName, columns)
    if dirFile = .false | dirFile = -1 then return .false
    self~fileAsDirectory = dirFile	-- store whole file in the attribute
    self~fileHeaders = dirFile[headers]	-- store fileHeaders in the attribute
    self~fileRecords = dirFile[records]	-- store fileRecords in the attribute
    recordCount      = dirFile[count]	-- store no. records in the attribute
    return recordCount


  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    readFile - Given a filename and the number of columns, reads a file and
               returns it in "fileAsDirectory" format.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD readFile PRIVATE
    expose recordCount
    use strict arg fileName, columns
    recordCount = 0
    --say "GenericFile-readFile-01: fileName =" fileName "columns =" columns
    file  = .stream~new(filename)
    --say "GenericFile-readFile-02: file~query(exists):" file~query("exists")
    if file~query("exists") = "" then do
      say "GenericFile-readFile-03: file" filename " does not exist."
      return .false
    end
    recordCount = file~lines
    if recordCount < 2 then do
      recordCount = 0
      return -1				-- Problem - no records, only headers - or nothing!.
    end
    recordCount = recordCount - 1	-- Exclude columns headers line from the count.
    arrRawfile = file~makearray 		-- read file into an array; each line is an array item
    file~close
    --say "GenericFile-readFile-04:" filearray~string
    self~fileArray = arrRawFile		-- (deprecated) store "raw" file array in attribute

    -- Now turn the raw file data into a directory:
    -- (1) get a 1D array of column headers or "field labels":
    fileHeaders = arrRawFile[1]
    arrFileHeaders = self~parseLine(fileHeaders, columns)
    self~fileHeaders = arrFileHeaders
    -- (2) get a 2D array of field values from data records:
    arrFileRecords = .Array~new
    do i=1 to recordCount		-- get a 2D array of file records
      arrRecord = self~parseLine(arrRawFile[i+1], columns)
      do j = 1 to columns
        arrFileRecords[i,j] = arrRecord[j]
      end
    end
    self~fileRecords = arrFileRecords
    -- (3) Build a directory containing headers, records, and lines (or record count).
    dirFile = .directory~new
    dirFile[headers] = arrFileHeaders
    dirFile[count]   = recordCount
    dirFile[records] = arrFileRecords
    return dirFile	-- Returns a directory containing the file and file info.


  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     parseLine - parse text from file line and place into an array.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD parseLine PRIVATE		-- returns a 1D array of fields
    use strict arg line, columns
    arr = .Array~new
    do i=1 to columns
      parse var line field "|" line
      field = field~strip
      arr[i] = field
    end
    return arr


  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    list - Lists a file in raw form on the console.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD list PUBLIC
    expose recordCount filename
    if recordCount = 0 then do
       return .false
    end
    say; say "GenericFile-list: records in File '"||filename||"'"
    say self~fileHeaders
    do i=1 to recordCount
      say self~fileRecords[i]
    end
    return .true


  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     getRecord - Given a "key", returns a directory containing the record data
                 associated with that key.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD getRecord PUBLIC
    -- Returns a directory containing the data for the record with the requested
    --  key (dataKey).
    expose recordCount columns
    use strict arg dataKey
    if dataKey = "" then return .false
    --say "GenericFile-getRecord-01: dataKey=" dataKey
    if recordCount = 0 then do
      --say "GenericFile-getRecord: recordCount is zero - no records!"
      return .false    	-- File empty or does not exist
    end

    found = .false
    do recordNo = 1 to recordCount
      --say "GenericFile-getRecord-02: self~fileRecords dims:" self~fileRecords~dimension
      recordId = self~fileRecords[recordNo,1]
      --say "GenericFile-getRecord-03: recordId, dataKey = <"||recordId||">" "<"||dataKey||">"
      if recordId = dataKey then do
        found = .true;  leave;
      end
    end
    if \found then return .false			-- Records exist, but specified record not found.

    dirData = .Directory~new
    do j=1 to columns
      header = self~FileHeaders[j]
      dirData[header] = self~fileRecords[recordNo,j]
    end
    -- say "GenericFile-getRecord-04: Results:"
    -- do i over dirData
      -- say "'"||i||"' = '"||dirData[i]||"'"
    -- end
    --say "GenericFile-getRecord-05: record ="
    return dirData


  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    getFile - returns file as a directory, with indices 'headers', 'records', 'count'.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD getFile PUBLIC
    --say "GenericFile-getFile-01: self~fileAsDirectory =" self~fileAsDirectory
    --do i over self~fileAsDirectory
      --say "GenericFile-getFile-02 index & type =" i i~objectName
    --end
    --arrData = self~fileAsDirectory[records]
    --say "GenericFile-getFile-03 - arrData =" arrData
    --say "GenericFile-getFile-04 - arrData[1,1] =" arrData[1,1]
    return self~fileAsDirectory


  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  ::METHOD update		-- INCOMPLETE!!!
    -- Given a record as a Directory, replaces the existing record with the
    -- new data and writes the whole file to disk.
    -- Returns .false (a) if a record with the provided key does not exist, or
    --                (b) if values for all fields are not provided.

    expose recordCount columns fileArray fileName
    use arg recordKey, dirRecord		-- record key + new record

    -- Find the record:
    do i=2 to recordCount
      record = fileArray[i]
      parse var record recordId .		-- assume recordId is first column
      if recordId = recordKey then do
        found = .true;  leave;
      end
    end
    if \found then return .false		-- Records exist, but specified record not found.

    -- found record is i'th. Now replace that array item with the new data:
    currentRecord = fileArray[i]
---- This method is only opartially completed.  !!!!!!!!!!!!
    newrecord = ""
    do j=1 to columns
      --fileArray[i]field =
    end

/*============================================================================*/

