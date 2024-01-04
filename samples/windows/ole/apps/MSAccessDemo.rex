/*----------------------------------------------------------------------------*/
/* Copyright (c) 2006-2024 Rexx Language Association. All rights reserved.    */
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

/*
   purpose: Demonstrate how one can interact with a MS Access databases using ActiveX/OLE
   needs:   MS Access installed

   links:   <https://docs.microsoft.com/en-us/office/vba/api/overview/accesshttps://docs.microsoft.com/en-us/office/vba/api/overview/access>
            <https://docs.microsoft.com/en-us/office/vba/access/concepts/miscellaneous/concepts-access-vba-reference>
            <https://docs.microsoft.com/en-us/office/vba/api/overview/access/object-model>
*/

accessApp =.OleObject~new("Access.Application")
dbFileName=directory()"\ooRexxDemo.mdb"   -- define database file name
if SysFileExists(dbFileName) then         -- open existing database
   accessApp~openCurrentDatabase(dbFileName)
else
   accessApp~newCurrentDatabase(dbFileName)  -- create new database

conn=accessApp~currentProject~connection  -- get connection object

call dropTable       conn        -- make sure table is dropped
call createTable     conn        -- create a table in the database
call fillTable       conn        -- enter records to the table
call showTable       conn        -- show presently stored records
call deleteAndShow   conn        -- delete a record, show results
call updateAndShow   conn        -- update a few records, show results
conn~close                       -- close connection to the database

accessApp~closeCurrentDatabase   -- close database
call compressAndRepair accessApp, dbFileName -- compress database

say ".rexxInfo~architecture:" pp(.rexxInfo~architecture) "(bitness)"

/* ------------------------------------------------------------------------- */
::routine dropTable
   use arg conn                  -- fetch connection to database

   signal on any                 -- intercept any exception
   conn~execute("drop table myTable")     -- execute SQL statement
   say "dropped 'myTable'"
   say center(" end of dropTable ", 70, "-")
   say
   return
any:
   say "could not drop 'myTable' (does it exist?)"
   say center(" end of dropTable ", 70, "-")
   say
   return

/* ------------------------------------------------------------------------- */
::routine createTable
   use arg conn                  -- fetch connection to database

   sql="create table myTable (id integer, name text (30) )"
   say "executing" pp(sql) "..."
   conn~execute(sql)
   say center(" end of createTable ", 70, "-")
   say

/* ------------------------------------------------------------------------- */
::routine fillTable
   use arg conn                  -- fetch connection to database

      -- define a few names to insert into the database
   s=.set~of("Chip Davis",           "Christian Michel",             "David Ashley", -
             "Erich Steinb&ouml;ck", "Florian Gro&szlig;e-Coosmann", "Frank Clark",  -
             "Gil Barmwater",        "Jan Engehausen",               "Lavrentios Servissoglou", -
             "Jean-Louis Faucher",   "Kurt M&auml;rker",             "Lee Peedin",         -
             "Manfred Schweizer",    "Mark Hessling",                "Mark Miesfeld",      -
             "Mike F. Cowlishaw",    "Pam Taylor",                   "P.O. Jonsson",       -
             "Reiner Micke",         "Ren&eacute; Jansen",           "Rick McGuire",       -
             "Rony G. Flatscher",    "Stefan D&ouml;rsam",           "Uwe Berger",         -
             "Walter Pachl"                                                                -
            )

   -- an entry in the .local or .environment directory can be referenced by using an
   -- environment symbol, e.g. the entry "TOTAL.RECS" can be referred to with its
   -- environment symbol ".TOTAL.RECS" (note the leading dot) from any Rexx program
   .local~total.recs=s~items        -- save number of records in local environment
   say "inserting" pp(.total.recs) "record(s) into the table..."
   say

   do counter c name over s         -- iterate over collected items (arbitrary order)
      sql="insert into myTable (id, name) values ("right(c,2)", '"name"' )"
      say "  " right(c,2)":" pp(sql)  -- show sql statement
      conn~execute(sql)            -- execute the statement
   end
   say center(" end of fillTable ", 70, "-")
   say

/* ------------------------------------------------------------------------- */
::routine showTable
   use arg conn                  -- fetch connection to database

   sql="select * from myTable order by id"
   rs=conn~execute(sql)
   rs~moveFirst                  -- just make sure, it is pointing to the first record
   do counter c while rs~eof=.false  -- loop over record set
      say "  " right(c,2)                                -
                 "id="pp(rs~fields["id"]~value~right(2)) -
                 "name="pp(rs~fields["name"]~value)
      rs~moveNext
   end
   rs~close
   say center(" end of showTable ", 70, "-")
   say
   return

/* ------------------------------------------------------------------------- */
::routine deleteAndShow    -- delete a random row
   use arg conn                  -- fetch connection to database

   say "deleting a record from the table:"
   say
      -- delete row with an arbitrary value for the field "id"
   sql="delete from myTable where id="random(1,.total.recs)
   say "   executing" pp(sql) "..."

   param=.OLEVariant~new(count)
   conn~execute(sql, param)
   say "  " pp(param~!varValue_) "record deleted."
   say

   .local~total.recs=.total.recs-1  -- adjust total number of records

   call showTable   conn         -- show presently stored records

/* ------------------------------------------------------------------------- */
::routine updateAndShow
   use arg conn                  -- fetch connection to database

   say "updating a few records from the table:"
   say
      -- update some records randomly
   sql="update myTable set id=id*3 where id >" random(1,.total.recs)
   say "   executing" pp(sql) "..."
   param = .OLEVariant~new(count)
   conn~execute(sql, param)
   say "  " pp(param~!varValue_) "record(s) affected."
   say

   call showTable   conn         -- show presently stored records

/* ------------------------------------------------------------------------- */
::routine compressAndRepair
  use strict arg accessApp, dbFileName

  say "before compress & repair, database file size:" pp(stream(dbFileName, "c", "query size"))
  compressedName=SysTempFileName(dbFileName"-cmp???") -- get a unique new file name

  dbEngine=accessApp~DBEngine    -- get database engine
  dbEngine~compactDatabase(dbFileName, compressedName)   -- carry out compact & repair

  say "after  compress & repair, database file size:" pp(stream(compressedName, "c", "query size"))
      -- create a unique backup file name containing date and time of backup
  newBkpFileName=dbFileName"-bkp-"date("S")"-"time()~changestr(":","")

  say "renaming work files..."
      -- rename original database file name to backup file name
  cmdText="ren" '"'dbFileName'"' filespec("Name", newBkpFileName)
  say "DOS command:" pp(cmdText)
  address cmd cmdText            -- use ADDRESS CMD explicitly

      -- now rename new compressed file to original databse file name
  cmdText="ren" '"'compressedName'"' filespec("Name", dbFileName)
  say "DOS command:" pp(cmdText)
  cmdText                        -- this uses ADDRESS CMD implicitly
  say center(" end of compressAndRepair ", 70, "-")
  say

/* ------------------------------------------------------------------------- */
::routine pp      -- "pretty print" ;)
   return "[" || arg(1) || "]"
