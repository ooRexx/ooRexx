/*----------------------------------------------------------------------------*/
/* Copyright (c) 2006-2007 Rexx Language Association. All rights reserved.    */
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

/*
   purpose: Demonstrate how one can interact with Jet (MS Access) databases using ActiveX/OLE
   needs:   MS Access resp. the JetEngine installed
*/

dbFileName=directory()"\ooRexxDemo.mdb"   -- define database file nam
call createDataBaseFile dbFileName        -- create database file, if necessary

conn=.oleobject~new("ADODB.Connection")   -- create an ADO conncection to the database
conn~provider="Microsoft.Jet.OLEDB.4.0"   -- tell the connection what driver to use

conn~open(dbFileName)            -- open a connection to the database

call dropTable       conn        -- make sure table is dropped
call createTable     conn        -- create a table in the database
call fillTable       conn        -- enter records to the table
call showTable       conn        -- show presently stored records
call deleteAndShow   conn        -- delete a record, show results
call updateAndShow   conn        -- update a few records, show results

conn~close                       -- close connection to the database

call compressAndRepair dbFileName


::routine createDataBaseFile
  parse arg dbFileName

  if stream(dbFileName, "c", "query exists")="" then  -- database file does not exist
  do
     appAccess = .OLEObject~new("Access.Application") -- create an instance
     appAccess~NewCurrentDatabase(dbFileName)         -- create the database file
     appAccess~quit                                   -- quit the application
     say "created database file" pp(dbFileName)
  end
  else
     say "database file" pp(dbFileName) "exists already"

  say center(" end of createDataBaseFile ", 70, "-")
  say


::routine dropTable
   use arg conn                  -- retrieve connection to database

   signal on any                 -- intercept any exception
   conn~execute("drop table myTable")     -- execute SQL statement
   say "dropped 'myTable'"
   say center(" end of dropTable ", 70, "-")
   say
   return
any:
   say "could not drop 'myTable'"
   say condition("C") condition("D")
   say center(" end of dropTable ", 70, "-")
   say
   return

::routine createTable
   use arg conn                  -- retrieve connection to database

   sql="create table myTable (id integer, name text (30) )"
   say "executing" pp(sql) "..."
   conn~execute(sql)
   say center(" end of createTable ", 70, "-")
   say

::routine fillTable
   use arg conn                  -- retrieve connection to database

      -- define a few names to insert into the database
   s=.set~of("Chip Davis",                   "Christian Michel",        "David Ashley",  -
             "Florian Gro&szlig;e-Coosmann", "Frank Clark",             "Gil Barmwater", -
             "Jan Engehausen",               "Lavrentios Servissoglou", "Lee Peedin",    -
             "Manfred Schweizer",            "Mark Hessling",           "Pam Taylor",    -
             "Reiner Micke",                 "Ren&eacute; Jansen",      "Rick McGuire",  -
             "Rony G. Flatscher",            "Stefan D&ouml;rsam",      "Uwe Berger",    -
             "Walter Pachl",                 "Mark Miesfeld"                             -
            )

   .local~tmp.nrRecs=s~items              -- save number of records in local environment
   .local~tmp.length=length(.tmp.nrRecs)  -- save length

   say "inserting" pp(.tmp.nrRecs) "record(s) into the table..."
   say

   i=0
   do name over s                -- iterate over collected items (arbitrary order)
       i=i+1                     -- calc unique id value
       sql="insert into myTable (id, name) values ("i~right(.tmp.length)", '"name"' )"
       say "  " i~right(.tmp.length)":" pp(sql)  -- show sql statement
       conn~execute(sql)            -- execute the statement
   end
   say center(" end of fillTable ", 70, "-")
   say


::routine showTable
   use arg conn                  -- retrieve connection to database

   rs=.oleObject~new("ADODB.Recordset")   -- create an ADO recordset object

   rs~CursorType =  1            -- .ole.const~adOpenKeyset
   rs~LockType   =  3            -- .ole.const~adLockOptimistic
   sql="select * from myTable order by id"
   rs~open(sql, conn)            -- open the table
   totRecs=rs~recordCount        -- get recordCount
   say "nr. of records:" pp(totRecs)":"
   say
   rs~moveFirst                  -- just make sure, it is pointing to the first record
   do i=1 to totRecs while rs~eof=.false  -- loop over record set
      say "  " i~right(.tmp.length)":"                             -
                 "id="pp(rs~fields["id"]~value~right(.tmp.length)) -
                 "name="pp(rs~fields["name"]~value)
      rs~moveNext
   end
   rs~close
   say center(" end of showTable ", 70, "-")
   say

::routine deleteAndShow
   use arg conn                  -- retrieve connection to database

   say "deleting a record from the table:"
   say
      -- delete row with an arbitrary value for the field "id"
   sql="delete from myTable where id="random(1,.tmp.nrRecs)
   say "   executing" pp(sql) "..."
   conn~execute(sql)
   say

   .local~tmp.nrRecs=.tmp.nrRecs-1        -- subtract 1 from total number of records
   .local~tmp.length=length(.tmp.nrRecs)  -- save length

   call showTable   conn         -- show presently stored records

::routine updateAndShow
   use arg conn                  -- retrieve connection to database

   say "updating a few records from the table:"
   say
      -- update some records randomly
   sql="update myTable set id=id*3 where id >" random(1,.tmp.nrRecs)
   say "   executing" pp(sql) "..."
   param = .OLEVariant~new(count)
   conn~execute(sql, param)
   say "  " pp(param~!varValue_) "record(s) affected."
   say

   call showTable   conn            -- show presently stored records


::routine compressAndRepair      -- Jet (= MS Access) databases tend to grow considerably
  parse arg dbFileName

  say "before compress & repair, database file size:" pp(stream(dbFileName, "c", "query size"))
  compressedName=SysTempFileName(dbFileName"-cmp???") -- get a unique new file name

/*
   -- Hint: if your MDB-database is secured with a database password, you must supply
   --       that password in the variable "pw":

  if pw="" then pwdString=""
           else pwdString=";Jet OLEDB:Database Password="pw

  fromString="Provider=Microsoft.Jet.OLEDB.4.0;Data Source="dbFileName     || pwdString
  toString  ="Provider=Microsoft.Jet.OLEDB.4.0;Data Source="compressedName || pwdString
*/

   -- Hint: MDB database files have no database password set
  fromString="Provider=Microsoft.Jet.OLEDB.4.0;Data Source="dbFileName
  toString  ="Provider=Microsoft.Jet.OLEDB.4.0;Data Source="compressedName


  jro=.oleObject~new("JRO.JetEngine")
  jro~compactDatabase(fromString, toString)     -- carry out compact & repair
  say "after  compress & repair, database file size:" pp(stream(compressedName, "c", "query size"))

      -- create a unique backup file name containing date and time of backup
  newBkpFileName=dbFileName"-bkp-"date("S")"-"time()~changestr(":","")

  say "renaming work files..."
      -- rename original database file name to backup file name
  cmdText="ren" dbFileName filespec("Name", newBkpFileName)
  say "DOS command:" pp(cmdText)
  address cmd cmdText               -- use ADDRESS CMD explicitly

      -- now rename new compressed file to original databse file name
  cmdText="ren" compressedName filespec("Name", dbFileName)
  say "DOS command:" pp(cmdText)
  cmdText                           -- this uses ADDRESS CMD implicitly
  say center(" end of compressAndRepair ", 70, "-")
   say


::routine pp
   return "[" || arg(1) || "]"


