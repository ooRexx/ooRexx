/* ========================================================================= */
/* csvStream ooRexx stream subclass for CSV file handling                    */
/* Version 1.08                                                   March 2009 */
/*                                                                           */
/* Amendments                                                                */
/* 1.01   Dec06 SN Headers inserted by close method / general tidy up        *//*{1.01}*/
/* 1.02 28Dec06 SN Allow specification of delimiter / qualifier              *//*{1.02}*/
/*                 Bugfix for non - header files looking for headers         *//*{1.02}*/
/* 1.03 29Dec06 SN Cope with no parm passed to CSVLineOut                    *//*{1.03}*/
/* 1.04 05Jan07 SN Accept STEM data with headers & fix firstcolumn bug       *//*{1.04}*/
/* 1.05 07Dec07 SN performance enhancements                                  *//*{1.05}*/
/* 1.06 21Dec07 SN As ooRexx 3.2 has lineend constant - remove discovery     *//*{1.06}*/
/* 1.07 21Apr08 SN provide stripoption for CSVLineIn                         *//*{1.07}*/
/* 1.08 23Mar09 SN Bugfix - gratitude to Bill Shipman                        *//*{1.08}*/
/*                                                                           */
/* ========================================================================= */

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2007 - 2008 Rexx Language Association. All rights reserved.  */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.ibm.com/developerworks/oss/CPLv1.0.htm                          */
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

::class CsvStream subclass Stream Public
/* ========================================================================= */
/* ------------------------------------------------------------------------- */
::Attribute FileHasHeaders        private -- copy of headersExist parm
::Attribute headers                       -- csvStreamHeader Object
::Attribute originalRawHeaders    private -- for comparison
::Attribute headerLineAbsent      private -- headersexist but absent
::Attribute OpenArgs              private -- args to open method
::Attribute values                        -- table for headered i/o
::Attribute rawText                       -- copy of last line read
::Attribute skipHeaders                   -- switch can be set before open
::Attribute lineEnd                       -- string used for line terminations
::Attribute Delimiter                     -- separates CSV fields              /*{1.02}*/
::Attribute Qualifier                     -- Surrounds literals                /*{1.02}*/
::Attribute StripOption get               -- optional stripping on lineIn      /*{1.07}*/
::Attribute StripOption set                                                    /*{1.07}*/
   expose stripOption                                                          /*{1.07}*/
   use arg stripOption                                                         /*{1.07}*/
                                                                               /*{1.07}*/
   if \stripOption~caselessmatchChar(1,'LTB N')                                /*{1.07}*/
   then raise syntax 40.904 array ("stripOption", 1, 'L,T,B, ,N', stripOption) /*{1.07}*/
::Attribute StripChar                                                          /*{1.07}*/
/* ------------------------------------------------------------------------- */
::method Init
/* ------------------------------------------------------------------------- */
use arg parms, headersExist
                                                        /* initialise values */
self~fileHasHeaders     =   (headersExist = .true                  ),
                          | 'HEADERS'~abbrev(headersexist~translate)
self~skipHeaders        = .true
self~values             = .nil     /* replaced with a table by headered read */
self~OriginalRawHeaders = ''
self~rawText            = ''
self~headerLineAbsent   = .false
self~delimiter          = ','     /* seperates fields in a CSV file          *//*{1.02}*/
self~qualifier          = '"'     /* surrounds literal fields                *//*{1.02}*/
self~openArgs           = .nil                                                 /*{1.05}*/
self~stripOption        = 'N'                                                  /*{1.07}*/
self~stripChar          = ' '                                                  /*{1.07}*/

self~init:super(parms)                        /* let stream class initialise */

self~lineEnd            = .endOfLine          /* line terminator for this os *//*{1.06}*/

/* ------------------------------------------------------------------------- */
::method Open
/* ------------------------------------------------------------------------- */
use arg args

self~openArgs = args~translate             /* Close needs to know open basis */

if self~fileHasHeaders = .true             /* read the headers into a table  */
then do
   self~headers = .csvStreamHeader~new           /* blank in case no headers */
   if self~open:super('read') = 'READY:'
   then do
      if self~chars > 0
      then do
         self~fileHasHeaders     = .false
         headersArray            = self~CsvLineIn      /* get header array   */
         self~fileHasHeaders     = .true

         do i = 1 to headersArray~last
            if headersArray[i] \= .nil
            then self~headers~field(i)~name = headersArray[i]
         end /* DO */

         self~OriginalRawHeaders = self~rawText
      end /* DO */
      else self~headerLineAbsent = .true
      self~close:super
   end /* DO */
   else self~headerLineAbsent = .true
end /* DO */

forward class (super) continue                              /* open the file */

if   (args~word(1)~translate \= 'WRITE') , /* move read pointer past headers */
&    (self~fileHasHeaders     = .true  ) ,
&    (self~skipHeaders        = .true  ) , /* < user may override this       */
&    (self~chars              > 0      )
then x = self~lineIn:super

/* ------------------------------------------------------------------------- */
::method csvLineIn
/* ------------------------------------------------------------------------- */
expose qualifier delimiter rawtext lineend                                     /*{1.05}*/

if self~openArgs = .nil then self~open                                         /*{1.05}*/

rawText       = ''                                                             /*{1.05}*/
inLiteral     = .false
fieldNo       = 1
fieldText     = ''
csvFields     = .array~new
literalFields = .set~new
blip          = qualifier                                                      /*{1.05}*/
blipBlip      = qualifier~copies(2)                                            /*{1.05}*/

do until (inLiteral  = .false) ,
   |     (self~chars = 0     )

   text = self~lineIn                              /* get a line of csv text */

   if   rawtext = '' ,                             /* not really a csv file  *//*{1.05}*/
   &    text~verify(delimiter||qualifier,'m')=0                                /*{1.05}*/
   then text = text||delimiter                                                 /*{1.05}*/

   if   rawText = ''                          /* maintain linetext attribute *//*{1.05}*/
   then rawText = text                                                         /*{1.05}*/
   else do                                    /* this is a multiline field   */
      rawText   = rawText || lineEnd || text                                   /*{1.05}*/
      fieldText = fieldText || lineEnd                                         /*{1.05}*/
   end /* Do */

   textlength = text~Length                                                    /*{1.05}*/

   i = 0                                                                       /*{1.05}*/
   do forever                                                                  /*{1.05}*/
      i = i + 1                                                                /*{1.05}*/
      if i > textLength then leave                                             /*{1.05}*/
                                                                               /*{1.05}*/
      select
         when text~matchChar(i,qualifier)                                      /*{1.05}*/
            then do
               inLiteral = \inLiteral                                          /*{1.05}*/
               fieldText = fieldText || qualifier                              /*{1.05}*/
            end /* DO */

         when (text~matchchar(i,delimiter) ,                                   /*{1.05}*/
               | text == '' | i = textlength) ,                                /*{1.05}*/
         &    (\inliteral                              )     /* end of field *//*{1.05}*/
            then do

               if \text~matchchar(i,delimiter)                                 /*{1.05}*/
               then fieldText = fieldText || text~substr(i,1)                  /*{1.05}*/

               /* if field encased in '"'s then strip them */
               if  (fieldText~length   > 1        )                ,           /*{1.08}*/
               ,   fieldText~matchChar(1,qualifier)                ,           /*{1.08}*/
               ,   fieldText~matchChar(fieldText~length,qualifier) ,           /*{1.08}*/
               then do
                  fieldText = fieldText~substr(2, fieldText~length - 2)
                  literalFields~put(fieldNo)
               end /* Do */

               /* '"' are represented in text as '""' */
               fieldText = fieldText~changeStr(blipBlip,blip)                  /*{1.02}*/

               /* N (for normal or none) means do not strip */                 /*{1.07}*/
               if \self~stripOption~caselessMatchChar(1,'N')                   /*{1.07}*/
               then fieldtext = fieldtext~strip(self~stripOption,self~stripChar) /*{1.07}*/

               csvFields[fieldNo] = fieldText
               fieldNo            = fieldNo + 1
               fieldText          = ''
            end /* DO */

         otherwise
            fieldText = fieldText || text~substr(i,1)                          /*{1.05}*/
      end /* select */

      if   (i = textlength   ) ,                  /* natural end of row?     *//*{1.05}*/
      &    (\inLiteral       ) ,
      &    (fieldText  \== '')
      then do                                                                  /*{1.05}*/
         text = text||delimiter                   /* implied field seperator *//*{1.05}*/
         textLength = textLength + 1                                           /*{1.05}*/
      end                                                                      /*{1.05}*/
   end /* DO */
end /* DO */


if self~fileHasHeaders = .true                     /* create table of values */
then do
   do fieldNo over literalFields
      self~headers~field(fieldNo)~literal= .true
   end /* DO */
   self~values = .table~new
   do i = 1 to self~Headers~last
      if   csvFields[i] = .nil
      then self~values~put('',self~headers~field(i)~name)
      else self~values~put(csvfields[i],self~headers~field(i)~name)
   end /* DO */
end /* DO */

return csvFields

/* ------------------------------------------------------------------------- */
::method csvLineOut
/* ------------------------------------------------------------------------- */
use arg data

blip          = self~qualifier
blipBlip      = self~qualifier~copies(2)

if symbol('DATA') = 'LIT'           /* no parm was passed so close CSVstream */
then do
   self~close
   return
end /* DO */

parse upper value data~class~string with . dataCollectionType .                /*{1.04}*/

select                                                                         /*{1.04}*/
   /* if we have been passed a table or stem, convert it to an array */
   when (self~fileHasHeaders = .true                       ),
   &    .set~of('TABLE','STEM')~hasIndex(dataCollectionType)                   /*{1.04}*/
   then do
      dataArray = .array~new
      do name over data
         if (name = 0) & (dataCollectionType = 'STEM') then iterate            /*{1.04}*/
         column = 0
         do i = 1 to self~headers~last
            if self~headers~field(i)~name = name
            then do
               column = i
               leave
            end /* DO */
         end /* DO */
         if column = 0                                /* unregistered column */
         then do
            column = self~headers~last + 1
            self~headers~field(column)~name = name
         end /* DO */
         dataArray[column] = data[name]                                        /*{1.04}*/
      end /* DO */
      data = dataArray
   end /* DO */

   when dataCollectionType = 'ARRAY'
      then nop

   when data~hasMethod('makeArray')
      then data = data~makearray

   otherwise
      raise syntax 93.953 array (1, "Array")
end /* select */

text = ''
do i = 1 to data~last

   if self~fileHasHeaders            /* force literal field even if numeric? *//*{1.02}*/
   then forceLiteral = (self~Headers~field(i)~literal = .true)                 /*{1.02}*/
   else forceLiteral = .false                                                  /*{1.02}*/

   select
      when data[i] = .nil                                   /*      no value */
         then text = text||self~delimiter                                      /*{1.02}*/
      when ( data[i]~datatype('n') ,                        /* numeric value */
        |    data[i] == ''        ),                                           /*{1.03}*/
      &    \forceLiteral                                                       /*{1.02}*/
         then text = text||self~delimiter||data[i]~strip                       /*{1.02}*/
      when data[i]~left(1)  = self~qualifier ,     /* literal already quoted *//*{1.02}*/
      &    data[i]~right(1) = self~qualifier ,                                 /*{1.02}*/
      &    data[i]~length > 1
      then do
         text = text                                                        ||,
                self~delimiter                                              ||,/*{1.02}*/
                self~qualifier                                              ||,/*{1.02}*/
                data[i]~substr(2,data[i]~length-2)~changeStr(blip,blipBlip) ||,/*{1.02}*/
                self~delimiter                                                 /*{1.02}*/
         if   self~fileHasHeaders                                              /*{1.02}*/
         then self~Headers~field(i)~literal = .true                            /*{1.02}*/
      end /* Do */
      otherwise                                             /* literal value */
         text = text                                                        ||,
                self~delimiter                                              ||,/*{1.02}*/
                self~qualifier                                              ||,/*{1.02}*/
                data[i]~changeStr(blip,blipBlip)                            ||,/*{1.02}*/
                self~qualifier                                                 /*{1.02}*/
         if   self~fileHasHeaders                                              /*{1.02}*/
         then self~Headers~field(i)~literal = .true                            /*{1.02}*/
   end /* select */
end /* DO */

parse var text 2 text                            /* remove leading delimiter */

return self~lineout(text)

/* ------------------------------------------------------------------------- */
::method close
/* ------------------------------------------------------------------------- */

self~close:super

if self~fileHasHeaders
then do                                               /* maintain headers    */
   headerText = ''                                    /* prepare header line */
   do i = 1 to self~headers~last
      if   self~headers~field(i) = .nil
      then headertext = headerText                                          ||,
                        self~delimiter                                         /*{1.02}*/
      else headertext = headerText                                          ||,
                        self~delimiter                                      ||,/*{1.02}*/
                        self~qualifier                                      ||,/*{1.02}*/
                        self~headers~field(i)~name                          ||,
                        self~qualifier                                         /*{1.02}*/
   end /* DO */
   parse var headerText . 2 headerText
   if headerText \= self~originalRawHeaders , /* headers need replacing      */
   &  self~openArgs~word(1) \= 'READ'         /* and file opened for writing */
   then do
      self~open:super('read')
      if self~headerLineAbsent = .false
      then x = self~linein                         /* obsolete header line   */
      entireText = self~charIn(,self~chars)
      self~close:super

      self~open:super('write replace')
      self~lineout(headertext)
      self~charout(entireText)
      self~close:super
   end /* DO */
end /* DO */

/* ------------------------------------------------------------------------- */
::method getHeaders
/* ------------------------------------------------------------------------- */
return self~Headers~copy

/* ------------------------------------------------------------------------- */
::method setHeaders
/* ------------------------------------------------------------------------- */
use arg newHeaders

if newHeaders~class~string \= 'The CSVSTREAMHEADER class'
then raise syntax 93.948 array (1, "CsvStreamHeader")

self~Headers = newHeaders~copy

/* ========================================================================= */
::class CsvStreamHeader
/* ========================================================================= */
/* ------------------------------------------------------------------------- */
::method FieldArray    Attribute  Private
/* ------------------------------------------------------------------------- */
::method init
/* ------------------------------------------------------------------------- */
self~FieldArray = .array~new

/* ------------------------------------------------------------------------- */
::method field
/* ------------------------------------------------------------------------- */
arg no

if self~fieldArray[no] = .nil
then do
   self~fieldArray[no] = .CsvStreamField~new
   self~fieldArray[no]~name = 'Field' no                     /* default name */
end

return self~FieldArray[no]

/* ------------------------------------------------------------------------- */
::method last
/* ------------------------------------------------------------------------- */
if self~fieldArray~last = .nil                                                 /*{1.04}*/
then return 0                                                                  /*{1.04}*/
else return self~fieldArray~last

/* ========================================================================= */
::class CsvStreamField
/* ========================================================================= */
/* ------------------------------------------------------------------------- */
::method name     Attribute
::method literal  Attribute
/* ------------------------------------------------------------------------- */

/* ========================================================================= */
