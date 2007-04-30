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
/******************************************************************************/
/* REXX Kernel                                                  stream.c      */
/*                                                                            */
/* Stream processing (stream oriented file systems)                           */
/*                                                                            */
/******************************************************************************/

#define  INCL_REXX_STREAM              /* include stream definitions        */
#include "RexxCore.h"                    /* global REXX definitions           */
#include "StringClass.hpp"
#include "RexxNativeAPI.h"
#include "StreamNative.h"
#include "StreamCommandParser.h"

#define full_name_parameter(stream_info) SysQualifyStreamName(stream_info)

#ifdef JAPANESE
extern int sharedOpen;
#endif

long stream_query_line_position(REXXOBJECT self, STREAM_INFO *stream_info, long current_position);
long read_backward_by_line(REXXOBJECT self, STREAM_INFO *stream_info, long *line_count, long *current_line, long *current_position);
long read_forward_by_line(REXXOBJECT self, STREAM_INFO *stream_info,long *line_count, long *current_line, long *current_position);
long read_from_end_by_line(REXXOBJECT self, STREAM_INFO *stream_info,long *line_count, long *current_line, long *current_position);
int  set_line_position(REXXOBJECT self, STREAM_INFO *stream_info);
long reclength_token(TTS *ttsp, const char *TokenString, TOKENSTRUCT *tsp, void *userparms);
void table_fixup(TTS *ttsp, unsigned long parse_fields[]);
long unknown_offset(TTS *ttsp, const char *TokenString, TOKENSTRUCT *tsp, void *user_parms);
long unknown_tr(TTS *ttsp, const char *TokenString, TOKENSTRUCT *tsp, void *userparms);

#define MAX_COUNTBUFFER ((1024*1024*2)-1)

/********************************************************************************/
/*                                                                              */
/* Data area's for open routines                                                */
/*                                                                              */
/********************************************************************************/

#define c_stream_info "CSELF"          /* name of REXX stream info variable */
                                       /* name of REXX stream buffer var    */
#define c_stream_buffer "C_STREAM_BUFFER"
                                       /* get the stream information        */

#define get_stream_info() get_verified_stream_info((STREAM_INFO *)StreamBuffer)

inline STREAM_INFO * get_verified_stream_info(STREAM_INFO * StreamBuffer) {
    if (!StreamBuffer) send_exception(Error_Incorrect_call);
    return StreamBuffer; }


#define temp_buffer(length) (PCHAR)buffer_address(RexxBuffer(length))
#define get_buffer(length)  allocate_stream_buffer(stream_info, length)

#define errcode Error_Incorrect_call

const long one = 1;
const long zero = 0;
const long minus_one = -1;

/*****************************************************************/
/* declares needed for command open                              */
/*****************************************************************/
#define oflag         Parse_Fields[0]
#define pmode         Parse_Fields[1]
#define fdopen_long   Parse_Fields[2]
#define i_binary      Parse_Fields[3]
#define i_nobuffer    Parse_Fields[4]
#define rdonly        Parse_Fields[5]
#define shared        Parse_Fields[6]

#define oflag_index       0
#define pmode_index       1
#define fdopen_long_index 2
#define i_binary_index    3
#define i_nobuffer_index  4
#define rdonly_index      5
#define shared_index      6

const int o_creat = O_CREAT;
const int o_rdonly = O_RDONLY;
const int o_wronly = O_WRONLY;
const int o_rdwr = O_RDWR;
const int rdwr_creat = O_RDWR | O_CREAT;
const int wr_creat = O_WRONLY | O_CREAT;
const int o_append = O_APPEND;


const int sh_denyno = SH_DENYNO;
const int sh_denyrd = SH_DENYRD;
const int sh_denywr = SH_DENYWR;

#if defined( O_BINARY )
const int o_binary = O_BINARY;
#else
const int o_binary = 0;
#endif

#if defined( O_SYNC )
const int o_sync = O_SYNC;
#endif
#if defined( O_RSHARE )
const int o_rshare = O_RSHARE;
#endif
#if defined( O_NSHARE )
const int o_nshare = O_NSHARE;
#endif
#if defined( O_DELAY )
const int o_delay = O_DELAY;
#endif

const int o_trunc = O_TRUNC;

const char c_read[] = "r";
const char c_default_read[] = "rb";
const char c_write[] = "w";
const char c_default_write[] = "wb";
const char c_both[] = "w+";
const char c_default_both[] = "r+b";
const char c_append[] = "a+";
const char c_wr_append[] = "a";
const char c_binary[] = "b";

const int s_iwrite = S_IWRITE;
const int s_iread  = S_IREAD;
const int iread_iwrite = S_IREAD | S_IWRITE;

/*****************************************************************/
/* declares needed for command seek/position                     */
/*****************************************************************/
#define from_current    Parse_Fields[0]
#define from_start      Parse_Fields[1]
#define from_end        Parse_Fields[2]
#define forward         Parse_Fields[3]
#define backward        Parse_Fields[4]
#define by_line         Parse_Fields[5]
#define position_flags  Parse_Fields[6]

#define from_current_index    0
#define from_start_index      1
#define from_end_index        2
#define forward_index         3
#define backward_index        4
#define by_line_index         5
#define position_flags_index  6

const long operation_read = 0x01;
const long operation_write = 0x02;
const long operation_nocreate = 0x04;
const long position_by_char = 0x04;
const long position_by_line = 0x08;
const long position_offset_specified = 0x10;

/*****************************************************************/
/* declares needed for command query seek/position               */
/*****************************************************************/

#define query_position_flags Parse_Fields[0]

#define query_position_flags_index 0

const long query_read_position = 0x01;
const long query_write_position = 0x02;
const long query_char_position = 0x04;
const long query_line_position = 0x08;
const long query_system_position = 0x10;

PCHAR  allocate_stream_buffer(
    STREAM_INFO *stream_info,          /* target stream information block   */
    LONG  length)                      /* length of buffer required         */
/******************************************************************************/
/* Function:  Get a buffer for a stream operation                             */
/******************************************************************************/
{
  REXXOBJECT   buffer;                 /* retrieve buffer                   */

                                       /* can we reuse the current buffer?  */
  if (stream_info->bufferAddress != NULL && stream_info->bufferLength >= length)
    return stream_info->bufferAddress; /* return the existing one           */
  if (length < default_buffer_size)    /* smaller than the minimum?         */
    length = default_buffer_size;      /* get the larger one                */
  buffer = RexxBuffer(length);         /* get a buffer                      */
                                       /* save the start address            */
  stream_info->bufferAddress = buffer_address(buffer);
  stream_info->bufferLength = length;  /* and the length                    */
  RexxVarSet(c_stream_buffer, buffer); /* associate with this instance      */
  return stream_info->bufferAddress;   /* return the starting address       */
}

void openStream(
    STREAM_INFO *stream_info,          /* target stream information block   */
    INT          openFlags,            /* _sopen flags                      */
    INT          openMode,             /* _sopen mode                       */
    const char  *fdopenMode,           /* fdopen mode information           */
    INT          sharedFlag )          /* flag for shared open              */
/******************************************************************************/
/* Function:  Open a stream in a specific mode                                */
/******************************************************************************/
{
                                       /* first try a shared open           */
  if ((stream_info->fh == 0) || (stream_info->fh == -1))
#ifdef WIN32
    /* CreateFile of COM ports on 95 requires specific conditions */
    if (RUNNING_95)
    {
        DCB dcb;
        HANDLE osf;

        if (!strnicmp(stream_info->full_name_parameter, "com", 3)
          && (stream_info->full_name_parameter[3] > '0') && (stream_info->full_name_parameter[3] <= '9'))
        {
            openFlags &=~o_creat;      /* COM ports require OPEN_EXISTING... */
            stream_info->fh = _sopen(stream_info->full_name_parameter, openFlags, sharedFlag, openMode);  /* ... and exclusive access */
            if (stream_info->fh != -1)
                osf = (HANDLE)_get_osfhandle(stream_info->fh);
            else osf = NULL;
            /* The following functions must be called, otherwise reading from COM port won't work */
            /* Note that the dcb is not modified but still Get and Set must be called, otherwise ReadFile hangs */
            if (osf && GetCommState(osf, &dcb))
                SetCommState(osf, &dcb);
        }
        else stream_info->fh = _sopen(stream_info->full_name_parameter, openFlags, sharedFlag, openMode);  /* allow sharing for normal files */
    }
    else
#endif
#if defined( O_NSHARE ) && defined( O_RSHARE )
  switch (sharedFlag)
  {
    case SH_DENYRW:
      openFlags |= O_NSHARE;
      break;
    case SH_DENYWR:
      openFlags |= O_RSHARE;
      break;
  }
#endif

#ifdef WIN32
    stream_info->fh = _sopen(stream_info->full_name_parameter, openFlags|_O_NOINHERIT , sharedFlag, openMode);
#else
    stream_info->fh = _sopen(stream_info->full_name_parameter, openFlags, sharedFlag, openMode);
#endif
  if (stream_info->fh != -1)           /* have a handle?                    */
                                       /* now get the FILE information      */
    stream_info->stream_file = fdopen(stream_info->fh, fdopenMode);
}

LONG stream_size(
    STREAM_INFO *stream_info)          /* target stream information block   */
/******************************************************************************/
/* Function:  Get the size of a stream                                        */
/******************************************************************************/
{
   struct stat stat_info;              /* stream information block          */

                                       /* not set the size yet?             */
   if (stream_info->pseudo_stream_size == 0) {
                                       /* try to get file information       */
     if (get_fstat(stream_info->fh, &stat_info) != 0) {
       /* get file size failed, try using stat instead */
       if (get_stat(stream_info->full_name_parameter, &stat_info) == 0)
         stream_info->pseudo_stream_size = stat_info.st_size;
       else                            /* transient stream, just use 1      */
         stream_info->pseudo_stream_size = 1;
     }
     else {                            /* get the size                      */
       stream_info->pseudo_stream_size = stat_info.st_size;
                                       /* possible multithread problem?     */
       if (stat_info.st_size == 0 && stat_info.st_mode&S_IFREG) {
                                       /* try for the external stat info    */
         if (get_stat(stream_info->full_name_parameter, &stat_info) == 0)
                                       /* get the new size                  */
           stream_info->pseudo_stream_size = stat_info.st_size;
       }
     }
   }
                                       /* return the new size               */
   return stream_info->pseudo_stream_size;
}

void stream_error(
    REXXOBJECT   self,                 /* target stream object              */
    STREAM_INFO *stream_info,          /* current stream information        */
    INT          error_code,           /* new error information             */
    REXXOBJECT   result )              /* notready return result            */
/******************************************************************************/
/* Function:   Raise a notready condition for stream errors                   */
/******************************************************************************/
{
  stream_info->error = error_code;     /* set the error information         */
                                       /* place this in an error state      */
  stream_info->state = stream_error_state;
  if (stream_info->stream_file) {      /* clear any errors if file is open  */
    clearerr(stream_info->stream_file);/* clear any errors                  */
  } /* endif */                        /* change for defect 31, CHM         */
                                       /* raise this as a notready condition*/
  RexxRaiseCondition("NOTREADY", RexxString(stream_info->name_parameter), self, result);
}

void stream_eof(
    REXXOBJECT   self,                 /* target stream object              */
    STREAM_INFO *stream_info,          /* current stream information        */
    REXXOBJECT   result )              /* notready return result            */
/******************************************************************************/
/* Function:   Raise a notready condition for stream end-of-file              */
/******************************************************************************/
{
  stream_info->error = 0;              /* set the error information         */
                                       /* place this in an eof state        */
  stream_info->state = stream_eof_state;
                                       /* raise this as a notready condition*/
  RexxRaiseCondition("NOTREADY", RexxString(stream_info->name_parameter), self, result);
}

void stream_check_eof(
    REXXOBJECT   self,                 /* target stream object              */
    STREAM_INFO *stream_info,          /* current stream information        */
    INT          error_code,           /* new error information             */
    REXXOBJECT   result )              /* notready return result            */
/******************************************************************************/
/* Function:   Check for an eof or error condition and raise appropriate      */
/*             notready condition                                             */
/******************************************************************************/
{
  if (feof(stream_info->stream_file))  /* get an EOF condition?             */
                                       /* raise that notready condition     */
    stream_eof(self, stream_info, result);
  else
                                       /* raise an error condition          */
    stream_error(self, stream_info, errno, result);
}

void get_stream_type(                  /* read a line from an I/O stream    */
    STREAM_INFO *stream_info,          /* current stream information        */
    BOOL         binary )              /* binary flag                       */
/******************************************************************************/
/* Function:   get the type of a stream                                       */
/******************************************************************************/
{
   stream_info->flags.binary = FALSE;  /* reset the binary flag             */
                                       /* and the transient flag            */
   stream_info->flags.transient = FALSE;
   if (!_transient) {                  /* not transient?                    */
     if (binary) {                     /* is it binary?                     */
                                       /* tag it as such                    */
       stream_info->flags.binary = TRUE;
                                       /* if reclength was not entered      */
       if (!stream_info->stream_reclength)
                                       /* use the dataset size as reclength */
         if (!(stream_info->stream_reclength = stream_size(stream_info)))
                                       /* raise an error for zero size      */
           send_exception(Error_Incorrect_call);
     }
   }
   else {
                                       /* record this as transient          */
     stream_info->flags.transient = TRUE;
     if (binary) {                     /* binary specified?                 */
                                       /* tag it as such                    */
       stream_info->flags.binary = TRUE;
                                       /* if reclength was not entered      */
       if (!stream_info->stream_reclength)
                                       /* use one as the reclength          */
         stream_info->stream_reclength = 1;
     }
   }
}

/********************************************************************************************/
/* close_stream                                                                             */
/********************************************************************************************/
void close_stream(
    OSELF        self,                 /* target stream object              */
    STREAM_INFO *stream_info )         /* stream information block          */
{
   int rc = 0;                         /* close return code                 */

   if (stream_info->flags.bstd) {      /* standard stream?                  */
#if defined(AIX) || defined(LINUX)
     if (stream_info->fh!=stdin_handle) { /* don't flush stdin              */
#endif
                                       /* return the flush information      */
     rc = buffer_flush;                /* flush the buffer                  */
     if (rc != 0)                      /* did this work?                    */
                                       /* go raise a notready condition     */
       stream_error(self, stream_info, rc, RexxInteger(rc));
#if defined(AIX) || defined(LINUX)
     }
#endif
   }
   else if (stream_info->stream_file) {/* is this a file?                   */
     rc = close_the_stream;            /* do the close                      */
     if (rc != 0) {                    /* have an error?                    */
                                       /* clear any errors                  */
       clearerr(stream_info->stream_file);
       rc = close_the_stream;          /* try the close again               */
       if (rc != 0)                    /* have an error?                    */
                                       /* go raise a notready condition     */
         stream_error(self, stream_info, rc, RexxInteger(rc));
     }
     stream_info->flags.open = FALSE;  /* this is now closed                */
     stream_info->fh = -1;             /* closed is not 0 but -1 */
     stream_info->stream_file = NULL;  /* mark stream as closed  */
                                       /* don't really know                 */
     stream_info->state = stream_unknown_state;
   }
}

/********************************************************************************************/
/* std_open                                                                                 */
/********************************************************************************************/
CSTRING std_open(
  STREAM_INFO *stream_info,            /* stream information                */
  CSTRING      ts )                    /* open parameters                   */
{
                                       /* is this standard in?              */
   if (!stricmp(stream_info->name_parameter,"STDIN") ||
       !stricmp(stream_info->name_parameter,"STDIN:")) {
     stream_info->stream_file = SysBinaryFilemode(stdin, TRUE);
     stream_info->fh = 0;              /* this is handle zero               */
     stream_info->flags.read_only = 1; /* this is a read-only file          */
   }
                                       /* standard out?                     */
   else if (!stricmp(stream_info->name_parameter,"STDOUT") ||
            !stricmp(stream_info->name_parameter,"STDOUT:")) {
     stream_info->stream_file = SysBinaryFilemode(stdout, FALSE);
     stream_info->fh = 1;              /* this is file handle one           */
     stream_info->flags.append = 1;    /* this is an append only file       */
   }
   else {                              /* must be standard error            */
                                       /* flag it,                          */
     stream_info->stream_file = SysBinaryFilemode(stderr, FALSE);
     stream_info->fh = 2;              /* tag as handle number 2            */
     stream_info->flags.append = 1;    /* also opened for append            */
   }
                                       /* copy name into stream info block  */
   strcpy(stream_info->full_name_parameter,stream_info->name_parameter);
                                       /* nobuffer requested?               */
   if (ts != NO_CSTRING && !stricmp(ts,"NOBUFFER"))
     stream_info->flags.nobuffer = 1;  /* tag this                          */
   else
     stream_info->flags.nobuffer = 0;  /* buffering is used                 */
   stream_info->flags.open = TRUE;     /* this is now open                  */
                                       /* this is now ready                 */
   stream_info->state = stream_ready_state;

   if (_transient)                     /* transient stream?                 */
                                       /* record this as transient          */
     stream_info->flags.transient = TRUE;
   return "READY:";                    /* return successful open            */
}

/********************************************************************************************/
/* handle_open                                                                              */
/********************************************************************************************/
CSTRING handle_open(
  REXXOBJECT   self,                   /* target stream                     */
  STREAM_INFO *stream_info,            /* stream information                */
  CSTRING      ts)                     /* open command string               */
{

/* fields that parse will fill in */
unsigned long Parse_Fields[7] = {
  0,                                       /* oflag       */
  0,                                       /* fdopen_long */
  0,                                       /* i_binary    */
  0,                                       /* i_nobuffer  */
 };

/* Action table for open parameters */
ATS  OpenActionread[] = {
      {ME,       sizeof(oflag), (void *)oflag_index, errcode,(void *)&o_creat,0},
      {BitOr,    sizeof(oflag), (void *)oflag_index, errcode,(void *)&o_rdonly,0},
      {CopyItem, sizeof(long),  (void *)rdonly_index, errcode,(void *)&one,0},
      {CopyItem, sizeof(c_read), (void *)fdopen_long_index, errcode,(void *)c_read,0},
      {0,0,0,0,0,0}
     };
ATS OpenActionwrite[] = {
      {ME,       sizeof(oflag), (void *)oflag_index, errcode,(void *)&o_rdwr,0},
      {MF,       sizeof(long),  (void *)rdonly_index, errcode,0,0},
      {BitOr,    sizeof(oflag), (void *)oflag_index, errcode,(void *)&wr_creat,0},
      {CopyItem, sizeof(c_write), (void *)fdopen_long_index, errcode,(void *)c_write,0},
      {0,0,0,0,0,0}
     };
ATS OpenActionboth[] = {
      {ME,       sizeof(oflag), (void *)oflag_index, errcode,(void *)&o_wronly,0},
      {MF,       sizeof(long),  (void *)rdonly_index, errcode,0,0},
      {BitOr,    sizeof(oflag), (void *)oflag_index, errcode,(void *)&rdwr_creat,0},
      {CopyItem, sizeof(c_both), (void *)fdopen_long_index, errcode,(void *)c_both,0},
      {0,0,0,0,0,0}
     };
ATS OpenActionnobuffer[] = {
      {CopyItem,sizeof(i_nobuffer), (void *)i_nobuffer_index, errcode,(void *)&one,0},
      {0,0,0,0,0,0}
     };
ATS OpenActionbinary[] = {
      {MF,sizeof(i_binary), (void *)i_binary_index, errcode,0,0},
      {BitOr,sizeof(oflag), (void *)oflag_index, errcode,(void *)&o_binary,0},
      {CopyItem,sizeof(i_binary), (void *)i_binary_index, errcode,(void *)&one,0},
      {0,0,0,0,0,0}
     };
ATS OpenActionreclength[] = {
      {MI,sizeof(i_binary), (void *)i_binary_index, errcode,(void *)&one,0},
      {CallItem,sizeof(int),0, errcode,0,reclength_token},
      {0,0,0,0,0,0}
     };

/* Token table for open parameters */
TTS  tts[] = {
      {"READ",3,     OpenActionread,0},
      {"WRITE",1,    OpenActionwrite,0},
      {"BOTH",2,     OpenActionboth,0},
      {"NOBUFFER",3, OpenActionnobuffer,0},
      {"BINARY",2,   OpenActionbinary,0},
      {"RECLENGTH",3,OpenActionreclength,0},
      {"\0",0,NULL, unknown_tr}
    };

char fdopen_type[4];

TTS *ttsp;

   char   char_buffer;                 /* single character buffer           */
   CHAR   work[30];                    /* error message buffer              */

   i_nobuffer = 0;                     /* set default handle parameters     */
   i_binary = 0;
   oflag = 0;
   fdopen_long = 0;
   fdopen_type[0] = '\0';
   ttsp = tts;

                                       /* initialize the stream info        */
                                       /* structure in case this is not the */
                                       /* first open for this stream        */
   strcpy(stream_info->full_name_parameter,"\0");
   stream_info->stream_file = NULL;
   stream_info->pseudo_stream_size = 0;
   stream_info->stream_reclength = 0;
   stream_info->flags.read_only = 0;
   stream_info->flags.write_only = 0;
   stream_info->flags.read_write = 0;
   stream_info->flags.append = 0;
   stream_info->flags.bstd = 0;
   stream_info->char_read_position = 1;
   stream_info->char_write_position = 1;
   stream_info->line_read_position = 1;
   stream_info->line_write_position = 1;
   stream_info->line_read_char_position = 1;
   stream_info->line_write_char_position = 1;
   stream_info->flags.nobuffer = 0;
   stream_info->flags.last_op_was_read = 1;

                                             /* copy into the full name from the name */
   strcpy(stream_info->full_name_parameter,stream_info->name_parameter);

                                       /* change the offsets to addresses in the tables */
   table_fixup(ttsp, Parse_Fields);
   if (NO_CSTRING != ts) {             /* have parameters?                  */
                                                 /* call the parser to setup the input information */
                                                 /* the input string should be upper cased         */
     if (parser(ttsp, ts, (void *)(&stream_info->stream_reclength)) != 0)
                                       /* this is an error                  */
       send_exception(Error_Incorrect_call);
   }
/********************************************************************************************/
/*           move over parse parameters into the stream info block                          */
/*           put the fdopen parameter into the correct casting to use for this code         */
/********************************************************************************************/

   strcpy(fdopen_type,(const char *)&fdopen_long);


                                                  /* set up read only flag                       */
   if (rdonly)
      stream_info->flags.read_only = 1;
                                                  /* set up read write flag                       */
   else if (oflag & o_wronly)
      stream_info->flags.read_write = 1;
                                                 /* read/write/both/append not specified default both */
   else {
      stream_info->flags.read_write = 1;
      strcpy(fdopen_type,c_both);
   }
                                                /* make sure all streams except transient are opened */
                                                /*  in binary mode                                   */
   if ((!i_binary && !_transient) || i_binary)
      strcat(fdopen_type,c_binary);
                                       /* open the stream by the handle     */
   if (NULL == open_stream_by_handle(fdopen_type)) {
     sprintf(work, "ERROR:%d", errno); /* format the error return           */
                                       /* go raise a notready condition     */
     stream_error(self, stream_info, errno, RexxString(work));
   }
                                             /* if nobuffer requested set it in the stream block */
   if (i_nobuffer)
     stream_info->flags.nobuffer = 1;
   else
     stream_info->flags.nobuffer = 0;
/********************************************************************************************/
/*          if it is a persistant stream put the write character pointer at the end         */
/*   need to check if the last character is end of file and if so write over it             */
/*   if the stream was created it will have a size of 0 but this will mess up the logic     */
/*          so set it to one                                                                */
/********************************************************************************************/

   if ((!_transient) && (stream_info->flags.write_only | stream_info->flags.read_write)) {
     if (stream_size(stream_info)) {
       if (!set_stream_position(stream_size(stream_info)-1))
                                            /* if this isn't a repeat of the last op then         */
                                            /* flush the buffer before doing the operation        */
         if (!stream_info->flags.last_op_was_read) {
           buffer_flush;
           stream_info->flags.last_op_was_read = ~stream_info->flags.last_op_was_read;
         }
         if (binary_read(&char_buffer,1) && ctrl_z == char_buffer)
           stream_info->char_write_position = stream_size(stream_info);
         else
                 {
           stream_info->char_write_position = stream_size(stream_info) + 1;
                   /* error on Windows so we had to put in that */
                                       /* explicitly set the position       */
                   set_stream_position(stream_info->char_write_position - 1);
                   /* I'm not sure here */
                 }
     }
     stream_info->line_write_position = 0;
     stream_info->line_write_char_position = 0;
   }
   stream_info->flags.open = TRUE;     /* this is now open                  */
                                       /* this is now ready                 */
   stream_info->state = stream_ready_state;
                                       /* go process the stream type        */
   get_stream_type(stream_info, i_binary);
   return "READY:";                    /* return success                    */
}

void implicit_open(
    REXXOBJECT   self,                 /* target stream object              */
    STREAM_INFO *stream_info,          /* current stream information        */
    LONG         type,                 /* type of open                      */
    REXXOBJECT   result )              /* notready condition return value   */
/******************************************************************************/
/* Function:   Handle implicit opens on first stream access                   */
/******************************************************************************/
{
   CHAR   work[30];                    /* error message buffer              */
   char   char_buffer;                 /* single character buffer           */
   struct stat stat_info;              /* stream information block          */

   if (stream_info->flags.bstd) {      /* standard stream?                  */
     std_open(stream_info, NULL);      /* handle as a standard stream       */
     return;                           /* finished                          */
   }
   else if (stream_info->flags.handle_stream) {
                                       /* do a handle open                  */
     handle_open(self, stream_info, NULL);
     return;                           /* finished                          */
   }

                                       /* initialize the stream info        */
                                       /*  in case this is not the first    */
                                       /* open for this stream              */
   strcpy(stream_info->full_name_parameter,"\0");
   stream_info->stream_file = NULL;
   stream_info->pseudo_stream_size = 0;
   stream_info->pseudo_lines = 0;
   stream_info->pseudo_max_lines = 0;
   stream_info->stream_reclength = 0;
   stream_info->flags.read_only = 0;
   stream_info->flags.write_only = 0;
   stream_info->flags.read_write = 1;  /* try for read/write                */
   stream_info->flags.bstd = 0;
   stream_info->flags.append = 0;
   stream_info->char_read_position = 1;
   stream_info->char_write_position = 1;
   stream_info->line_read_position = 1;
   stream_info->line_write_position = 1;
   stream_info->line_read_char_position = 1;
   stream_info->line_write_char_position = 1;
   stream_info->flags.nobuffer = 0;
   stream_info->flags.last_op_was_read = 1;
   stream_info->flags.transient = FALSE;
   stream_info->flags.binary = FALSE;
   full_name_parameter(stream_info);   /* get the fully qualified name      */


                                       /* first try for read/write          */
   if (type == operation_nocreate)     /* open file without create          */
#ifdef JAPANESE
      if (sharedOpen)
         open_the_stream_shared(o_rdwr | o_binary, iread_iwrite, c_default_both,SH_DENYNO);
      else
         open_the_stream(o_rdwr | o_binary, iread_iwrite, c_default_both);
#else
      open_the_stream(o_rdwr | o_binary, iread_iwrite, c_default_both);
#endif
   else
#ifdef JAPANESE
     if (sharedOpen)
       open_the_stream_shared(rdwr_creat | o_binary, iread_iwrite, c_default_both,SH_DENYNO);
     else
       open_the_stream(rdwr_creat | o_binary, iread_iwrite, c_default_both);
#else
       open_the_stream(rdwr_creat | o_binary, iread_iwrite, c_default_both);
#endif
                                       /* if there was an open error and    */
                                       /*  we have the info to try again -  */
                                       /*  doit                             */
   if ((stream_info->stream_file == NULL) ) {
     stream_info->flags.read_write = 0;/* turn off the read/write flag      */
     if (type == operation_write) {    /* this a write operation?           */
                                       /* try opening again                 */
#ifdef JAPANESE
     if (sharedOpen)
       open_the_stream_shared(o_rdwr | o_binary, iread_iwrite, c_default_write,SH_DENYNO);
     else
       open_the_stream(o_rdwr | o_binary, iread_iwrite, c_default_write);
#else
       open_the_stream(o_rdwr | o_binary, iread_iwrite, c_default_write);
                                       /* turn on the write only flag       */
#endif
       stream_info->flags.write_only = 1;
     }
     else {                            /* read operation                    */
                                       /* try opening again                 */
#ifdef JAPANESE
     if (sharedOpen)
       open_the_stream_shared(o_rdonly | o_binary, s_iread, c_default_read,SH_DENYNO);
     else
       open_the_stream(o_rdonly | o_binary, s_iread, c_default_read);
#else
       open_the_stream(o_rdonly | o_binary, s_iread, c_default_read);
#endif
                                       /* turn on the read only flag        */
       stream_info->flags.read_only = 1;
     }
                                       /* if there was an error             */
     if (stream_info->stream_file == NULL) {
       if (result == NULLOBJECT) {     /* no result given?                  */
                                       /* format the error return           */
         sprintf(work, "ERROR:%d", errno);
         result = RexxString(work);    /* go raise a notready condition     */
       }
                                       /* go raise a notready condition     */
       stream_error(self, stream_info, errno, result);
     }
   }

   fstat(stream_info->fh, &stat_info); /* get the file information          */
   if (stat_info.st_mode&S_IFCHR) {    /* is this a device?                 */
     set_nobuffer;                     /* turn off buffering                */

#if defined(WIN32)
     /* reset _bufsiz to 1 character for COM ports */
     if (!strnicmp(stream_info->name_parameter, "COM", 3) &&
         atoi(&stream_info->name_parameter[3]))
       stream_info->stream_file->_bufsiz = 1;
#endif
   }
                                       /* persistent writeable stream?      */
   if (!_transient && !stream_info->flags.read_only) {
     if (stream_size(stream_info)) {   /* existing stream?                  */
                                       /* try to set to the end             */
       if (!set_stream_position(stream_size(stream_info) - 1)) {
                                       /* read the last character, and if   */
                                       /* it is an EOF character            */
         if (binary_read(&char_buffer,1) && ctrl_z == char_buffer)
                                       /* position to overwrite it          */
           stream_info->char_write_position = stream_size(stream_info);
         else {                        /* write at the very end             */
           stream_info->char_write_position = stream_size(stream_info) + 1;
                   /* error on Windows so we had to put in that */
                                       /* explicitly set the position       */
                   set_stream_position(stream_info->char_write_position - 1);
                 }
       }
     }
                                       /* set default line positioning      */
     stream_info->line_write_position = 0;
     stream_info->line_write_char_position = 0;
   }
   stream_info->flags.open = TRUE;     /* this is now open                  */
                                       /* this is now ready                 */
   stream_info->state = stream_ready_state;
   get_stream_type(stream_info, FALSE);/* go process the stream type        */

}

void read_setup(                       /* setup for a read operation        */
    REXXOBJECT   self,                 /* target stream object              */
    STREAM_INFO *stream_info,          /* current stream information        */
    REXXOBJECT   result )              /* notready condition return value   */
/******************************************************************************/
/* Function:   Do common stream read operation setup and checking             */
/******************************************************************************/
{
   long tell_position;                 /* current stream position           */

   if (!stream_info->flags.open)       /* not open yet?                     */
                                       /* do the open                       */
     implicit_open(self, stream_info, operation_nocreate, result);
                                       /* reset to a ready state            */
   stream_info->state = stream_ready_state;

#if defined(AIX) || defined(LINUX)
   if (!stream_info->flags.bstd ||     /* no standard stream?               */
       stream_info->fh!=stdin_handle) {/* no stdin ?                        */
#endif
                                       /* get the current stream position   */
   tell_position = tell_stream_position;
                                       /* at the correct position?          */
   if (tell_position != -1 && (stream_info->char_read_position - 1) != tell_position) {
                                       /* do a seek to char_read_position   */
     if (set_stream_position(stream_info->char_read_position - 1))
                                       /* go raise a notready condition     */
       stream_error(self, stream_info, errno, result);
   }

#if defined(AIX) || defined(LINUX)
   }
#endif
                                       /* if this isn't a repeat of the last*/
                                       /* operation, then we need to flush  */
                                       /* the buffer before reading         */
   if (!stream_info->flags.last_op_was_read) {
     buffer_flush;                     /* flush the buffer                  */
                                       /* set the read flag on              */
     stream_info->flags.last_op_was_read = TRUE;
   }
}

                                       /* begin common read setup           */
#define setup_read_stream(result) read_setup(self, stream_info, result)


void write_setup(                      /* setup for a write operation       */
    REXXOBJECT   self,                 /* target stream object              */
    STREAM_INFO *stream_info,          /* current stream information        */
    LONG         result )              /* notready return result            */
/******************************************************************************/
/* Function:   Do common stream write operation setup and checking            */
/******************************************************************************/
{
   long tell_position;                 /* current stream position           */


   if (!stream_info->flags.open)       /* not open yet?                     */
                                       /* do the open                       */
     implicit_open(self, stream_info, operation_write, RexxInteger(result));
                                       /* reset to a ready state            */
   stream_info->state = stream_ready_state;
                                       /* get the current stream position   */
   tell_position = tell_stream_position;
                                       /* at the correct position?          */
   if (tell_position != -1 && (stream_info->char_write_position - 1) != tell_position) {

      if  (!stream_info->flags.append){/* not opened for append?            */
                                       /* set stream back to write position */
        if (set_stream_position(stream_info->char_write_position - 1))
                                       /* go raise a notready condition     */
          stream_error(self, stream_info, errno, RexxInteger(result));
      }
   }
                                       /* if this isn't a repeat of the last*/
                                       /* operation, then we need to flush  */
                                       /* the buffer before reading         */
   if (stream_info->flags.last_op_was_read) {
     buffer_flush;                     /* flush the buffer                  */
                                       /* set the read flag on              */
     stream_info->flags.last_op_was_read = FALSE;
   }
}

                                       /* do common write setup             */
#define setup_write_stream(result)  write_setup(self, stream_info, result)

LONG write_stream_line(                /* write a line to an I/O stream     */
    STREAM_INFO *stream_info,          /* current stream information        */
    CHAR        *buffer,               /* buffer to write                   */
    LONG         length )              /* length to write                   */
/******************************************************************************/
/* Function:   write a line to a stream                                       */
/******************************************************************************/
{
   LONG   result;                      /* residual character count          */

   result = line_write(buffer, length);/*   write out the buffer            */
                                       /* make sure there wasn't an error   */
   if (ferror(stream_info->stream_file))
     stream_info->error = errno;       /* save any error information        */
                                       /*   update the line_write_position  */
   stream_info->char_write_position += result;
                                       /*   update the stream size          */
   update_stream_size(stream_info->char_write_position);
   if (stream_info->flags.nobuffer)    /* no buffering requested?           */
     buffer_flush;                     /* force it out                      */
   return length - result;             /* return the residual count         */
}

REXXOBJECT read_stream_line(           /* read a line from an I/O stream    */
    REXXOBJECT   self,                 /* target stream object              */
    STREAM_INFO *stream_info,          /* current stream information        */
    CHAR        *buffer,               /* buffer to for output              */
    LONG         length,               /* length to read                    */
    BOOL         update_position )     /* update the read position          */
/******************************************************************************/
/* Function:   read a line from a stream                                      */
/******************************************************************************/
{
   LONG       result;                  /* residual character count          */
   REXXOBJECT string;                  /* returned string                   */

/* This is a patch for a defect in the Microsoft C++ Runtime-Library
   If an odd number of bytes is written to a device without buffering,
   the last character is internally bufferedand the IOWRT flag is set.
   While this flag is set, read won't work and flush doesn't remove
   the flag. */
#ifdef WIN32
   if ((stream_info->stream_file->_flag & _IOWRT) && (stream_info->flags.transient)
          && (stream_info->flags.read_write))
       stream_info->stream_file->_flag &=~_IOWRT;
#endif

   result = binary_read(buffer,length);/*  issue the read                   */
                                       /* make sure there wasn't an error   */
   if (ferror(stream_info->stream_file))
                                       /* raise an error condition          */
     stream_error(self, stream_info, errno, OREF_NULLSTRING);
   if (result == 0)                    /* work ok?                          */
                                       /* must be an eof condition          */
     stream_eof(self, stream_info, OREF_NULLSTRING);
   else {
                                       /* create a result string            */
     string = RexxStringL(buffer, result);
     if (update_position)              /* need to move read position?       */
                                       /* update the read position          */
       stream_info->char_read_position += result;
     if (result != length)             /* not get it all?                   */
                                       /* go raise a notready condition     */
       stream_eof(self, stream_info, string);
   }
   return string;                      /* return the string                 */
}

LONG read_stream_buffer(               /* read a buffer of data             */
    STREAM_INFO *stream_info,          /* current stream information        */
    BOOL         non_binary,           /* binary/non-binary read            */
    CHAR        *buffer,               /* buffer to for output              */
    LONG         length )              /* length to read                    */
/******************************************************************************/
/* Function:   read a buffer of data from a stream                            */
/******************************************************************************/
{
   LONG       result;                  /* residual character count          */

   stream_info->error = 0;             /* clear any error information       */
   if (non_binary) {                   /* not a binary request?             */
                                       /*  issue a non_binary_read          */
     if (non_binary_read(buffer, length) != NULL)
       result = strlen(buffer);        /* get the read length               */
     else
       result = 0;                     /* nothing returned                  */
   }
   else
                                       /*  use a binary read                */
     result = binary_read(buffer,length);
                                       /* make sure there wasn't an error   */
   if (ferror(stream_info->stream_file))
     stream_info->error = errno;       /* save any error information        */
   return result;                      /* return the read count             */
}

INT get_file_statistics(               /* read a line from an I/O stream    */
    STREAM_INFO *stream_info,          /* current stream information        */
    struct stat *stat_info )           /* returned stat information         */
/******************************************************************************/
/* Function:   get file statitics                                             */
/******************************************************************************/
{
                                       /* not opened as a handle?           */
   if (!stream_info->flags.handle_stream) {
     full_name_parameter(stream_info); /* expand the name                   */
                                       /* return the stream size            */
                                       /* get the name statistics           */
     return get_stat(stream_info->full_name_parameter, stat_info);
   }
   else                                /* have a handle                     */
                                       /* get information via fstat         */
     return get_fstat(stream_info->fh, stat_info);
}

void complete_line(
    REXXOBJECT   self,                 /* target stream object              */
    STREAM_INFO *stream_info)          /* current stream information        */
/******************************************************************************/
/* Function:   write out the rest of a line                                   */
/******************************************************************************/
{
  PCHAR   buffer;                      /* write buffer                      */
  LONG    write_length;                /* length to write out               */

                                       /* not on a line boundary?           */
  if (0 != ((stream_info->char_write_position % stream_info->stream_reclength) - 1)) {
                                       /* calculate length to write out     */
    write_length = stream_info->stream_reclength - ((stream_info->char_write_position % stream_info->stream_reclength) - 1);
    buffer = get_buffer(write_length); /* get a write buffer                */
    memset(buffer, ' ', write_length); /* fill buffer with blanks           */
                                       /*  keep the write out the info      */
    if (write_stream_line(stream_info, buffer, write_length) != 0)
                                       /* raise this as a notready condition*/
      RexxRaiseCondition("NOTREADY", RexxString(stream_info->name_parameter), self, IntegerOne);
  }
}

long write_fixed_line(
    REXXOBJECT   self,                 /* target stream object              */
    STREAM_INFO *stream_info,          /* current stream information        */
    CHAR        *data,                 /* data to write out                 */
    LONG         length )              /* length to read                    */
/******************************************************************************/
/* Function:   write out a fixed length record                                */
/******************************************************************************/
{
  LONG   write_length;                 /* total length to write             */
  PCHAR  buffer;                       /* temporary write buffer            */

                                       /* calculate the length needed       */
  write_length = stream_info->stream_reclength - ((stream_info->char_write_position % stream_info->stream_reclength) - 1);
  buffer = get_buffer(write_length);   /* get a write buffer                */
  memset(buffer, ' ', write_length);   /* fill buffer with blanks           */
  memcpy(buffer, data, length);        /* move the line_out into the buffer */
                                       /* write out the info                */
  return write_stream_line(stream_info, buffer, write_length);
}

void set_char_read_position(
    REXXOBJECT   self,                 /* target stream object              */
    STREAM_INFO *stream_info,          /* current stream information        */
    LONG         position,             /* target position                   */
    REXXOBJECT   result )              /* notready return result            */
/******************************************************************************/
/* Function:   move the character position to a fixed offset                  */
/******************************************************************************/
{
  if (position != NO_LONG) {           /* have a position specified?        */

    if (stream_info->flags.transient)  /* trying to move a transient stream?*/
                                       /* this is an error                  */
      send_exception(Error_Incorrect_method_stream_type);
    if (position < 1)                  /* too small?                        */
                                       /* report an error also              */
      send_exception1(Error_Incorrect_method_positive, RexxArray2(IntegerOne, RexxInteger(position)));
                                       /* make sure we're within the bounds */
    if (stream_size(stream_info) >= position) {
                                       /* try to move to the new position   */
      if (set_stream_position(position - 1))
                                       /* go raise appropriate notready     */
        stream_check_eof(self, stream_info, errno, result);
                                       /* set the new read position         */
      stream_info->char_read_position = position;
    }
    else
                                       /* beyond bounds, raise an EOF       */
      stream_eof(self, stream_info, result);
  }
}

void set_line_read_position(
    REXXOBJECT   self,                 /* target stream object              */
    STREAM_INFO *stream_info,          /* current stream information        */
    LONG         position,             /* target position                   */
    REXXOBJECT   result )              /* default failure result            */
/******************************************************************************/
/* Function:   move the character position to a fixed offset                  */
/******************************************************************************/
{
  if (position != NO_LONG) {           /* have a position specified?        */

    if (stream_info->flags.transient)  /* trying to move a transient stream?*/
                                       /* this is an error                  */
      send_exception(Error_Incorrect_method_stream_type);
    if (position < 1)                  /* too small?                        */
                                       /* report an error also              */
      send_exception1(Error_Incorrect_method_positive, RexxArray2(IntegerOne, RexxInteger(position)));
    if (position == 1) {               /* going to the start?               */
                                       /* set the position to the beginning */
      stream_info->line_read_char_position = 1;
      stream_info->line_read_position = 1;
      stream_info->char_read_position = 1;
                                       /* try to move to the new position   */
      if (set_stream_position(stream_info->char_read_position - 1))
                                       /* go raise appropriate notready     */
        stream_check_eof(self, stream_info, errno, result);
    }
                                       /* moving binary lines?              */
    else if (stream_info->flags.binary) {
                                       /* calculate the new position        */
      stream_info->char_read_position = stream_info->stream_reclength * (position - 1) + 1;
                                       /* try to move to the new position   */
      if (set_stream_position(stream_info->char_read_position - 1))
                                       /* go raise appropriate notready     */
        stream_check_eof(self, stream_info, errno, result);
    }
    else {                             /* moving a non-binary stream        */
                                       /* already at this line?             */
      if (position == stream_info->line_read_position)
        return;                        /* nothing to move                   */
                                       /* moving forward?                   */
      if (stream_info->line_read_position > 0 && position > stream_info->line_read_position)
                                       /* just moving forward a little      */
        position = position - stream_info->line_read_position;
      else {
        position--;                    /* make a relative offset from front */
                                       /* set the position to the beginning */
        stream_info->line_read_char_position = 1;
        stream_info->line_read_position = 1;
      }
                                       /* now go read forward the proper    */
                                       /* number of lines                   */
      if (read_forward_by_line(self, stream_info, &position, &stream_info->line_read_position, &stream_info->line_read_char_position) == 0)
                                       /* go raise appropriate notready     */
        stream_eof(self, stream_info, result);
                                       /* fix up the character read position*/
      stream_info->char_read_position = stream_info->line_read_char_position;
                                       /* try to move to the new position   */
      if (set_stream_position(stream_info->char_read_position - 1))
                                       /* go raise appropriate notready     */
        stream_check_eof(self, stream_info, errno, result);
    }
  }
}

void set_char_write_position(
    REXXOBJECT   self,                 /* target stream object              */
    STREAM_INFO *stream_info,          /* current stream information        */
    LONG         position,             /* target position                   */
    REXXOBJECT   result )              /* default failure result            */
/******************************************************************************/
/* Function:   move the character position to a fixed offset                  */
/******************************************************************************/
{
  if (position != NO_LONG) {           /* have a position specified?        */

    if (stream_info->flags.transient)  /* trying to move a transient stream?*/
                                       /* this is an error                  */
      send_exception(Error_Incorrect_method_stream_type);
    if (position < 1)                  /* too small?                        */
                                       /* report an error also              */
      send_exception1(Error_Incorrect_method_positive, RexxArray2(IntegerOne, RexxInteger(position)));
                                       /* try to move to the new position   */
    if (set_stream_position(position - 1))
                                       /* go raise appropriate notready     */
      stream_check_eof(self, stream_info, errno, result);
                                       /* set the new write position        */
    stream_info->char_write_position = position;
  }
}

void set_line_write_position(
    REXXOBJECT   self,                 /* target stream object              */
    STREAM_INFO *stream_info,          /* current stream information        */
    LONG         position,             /* target position                   */
    REXXOBJECT   result )              /* default failure result            */
/******************************************************************************/
/* Function:   move the character position to a fixed offset                  */
/******************************************************************************/
{
  if (position != NO_LONG) {           /* have a position specified?        */

    if (stream_info->flags.transient)  /* trying to move a transient stream?*/
                                       /* this is an error                  */
      send_exception(Error_Incorrect_method_stream_type);
    if (position < 1)                  /* too small?                        */
                                       /* report an error also              */
      send_exception1(Error_Incorrect_method_positive, RexxArray2(IntegerOne, RexxInteger(position)));
    if (position == 1) {               /* going to the start?               */
                                       /* set the position to the beginning */
      stream_info->line_write_char_position = 1;
      stream_info->line_write_position = 1;
      stream_info->char_write_position = 1;
                                       /* try to move to the new position   */
      if (set_stream_position(stream_info->char_write_position - 1))
                                       /* go raise appropriate notready     */
        stream_check_eof(self, stream_info, errno, result);
    }
                                       /* moving binary lines?              */
    else if (stream_info->flags.binary) {
                                       /* calculate the new position        */
      stream_info->char_write_position = stream_info->stream_reclength * (position - 1) + 1;
                                       /* try to move to the new position   */
      if (set_stream_position(stream_info->char_write_position - 1))
                                       /* go raise appropriate notready     */
        stream_check_eof(self, stream_info, errno, result);
    }
    else {                             /* moving a non-binary stream        */
                                       /* already at this line?             */
      if (position == stream_info->line_write_position)
        return;                        /* nothing to move                   */
                                       /* moving forward?                   */
      if (stream_info->line_write_position > 0 && position > stream_info->line_write_position) {
                                       /* just moving forward a little      */
        position = position - stream_info->line_write_position;
      }
      else {
        position--;                    /* make a relative offset from front */
                                       /* set the position to the beginning */
        stream_info->line_write_char_position = 1;
        stream_info->line_write_position = 1;
      }
                                       /* now go read forward the proper    */
                                       /* number of lines                   */
      if (read_forward_by_line(self, stream_info, &position, &stream_info->line_write_position, &stream_info->line_write_char_position) == 0)
                                       /* go raise appropriate notready     */
        stream_eof(self, stream_info, result);
                                       /* fix up character write position   */
      stream_info->char_write_position = stream_info->line_write_char_position;
                                       /* try to move to the new position   */
      if (set_stream_position(stream_info->char_write_position - 1))
                                       /* go raise appropriate notready     */
        stream_check_eof(self, stream_info, errno, result);
    }
  }
}

LONG scan_forward_lines(               /* move forward a number of lines    */
  PCHAR  buffer,                       /* start of buffer                   */
  LONG   length,                       /* buffer length                     */
  LONG  *count,                        /* count to move                     */
  PCHAR  end_char,                     /* end-of-line marker                */
  LONG   end_size )                    /* size of end-of-line marker        */
/******************************************************************************/
/* Function: move forward a number of lines in a buffer                       */
/******************************************************************************/
{
  PCHAR   scan_pointer;                /* scanning pointer                  */
  PCHAR   last_scan;                   /* last scan position                */
  PCHAR   endptr;                      /* end of the buffer                 */
//CHAR    delimiters[4];               /* search delimiters                 */
//CHAR    delimiters[] = { ctrl_z, nl, '\0' };  /* delimiters               */
  CHAR    delimiters[] = { nl, '\0' }; /* delimiters                        */
  LONG    buffer_index;                /* current buffer position           */

  buffer_index = 0;                    /* get the buffer index              */
//delimiters[0] = ctrl_z;              /* fill in the EOF character         */
//delimiters[1] = end_char[0];         /* fill in first line_end character  */
//delimiters[2] = end_char[1];         /* add the string end                */
//delimiters[3] = '\0';                /* add the string end                */

  endptr = buffer + length;            /* set the end location              */
  last_scan = buffer;                  /* save the start position           */
                                       /* scan for a important character    */
  scan_pointer = mempbrk(buffer, delimiters, length - buffer_index);
  while (scan_pointer != NULL) {       /* found an important one?           */
    switch (*scan_pointer) {           /* process the located character     */

      case nl:                         /* either a new line or carriage     */
//    case cr:                         /* return character                  */
                                       /* full carriage return?             */
//      if (memcmp(scan_pointer, end_char, end_size) == 0) {
          (*count)--;                  /* count the line                    */
                                       /* step over the line end character  */
//        scan_pointer += end_size;    /* match checked size!    */
          scan_pointer++;              /* - match checked size!             */
          last_scan = scan_pointer;    /* save the last position            */
          if (*count == 0)             /* count exhausted?                  */
                                       /* return the offset                 */
            return scan_pointer - buffer + 1;
//      }
//      else {                         /* is it a single NL or CR?          */
//        (*count)--;                  /* count the line                    */
//        scan_pointer++;              /* step over it                      */
//        last_scan = scan_pointer;    /* save the last position            */
//        if (*count == 0)             /* count exhausted?                  */
//                                     /* return the offset                 */
//          return scan_pointer - buffer + 1;
//      }
        break;

//    case ctrl_z:                     /* end of file character             */
//                                     /* return if this is the last char.  */
//      if ( scan_pointer == endptr - 1 ) {
//        if (last_scan != scan_pointer)    /* a real line before this?     */
//          (*count)--;                     /* count this line              */
//        return scan_pointer - buffer + 1; /* return the offset            */
//      }
//      (*count)--;                    /* count this line                   */
//      scan_pointer++;
//      last_scan = scan_pointer;      /* save the last position            */
//      if (*count == 0)               /* count exhausted?                  */
//                                     /* return the offset                 */
//        return scan_pointer - buffer + 1;

      case '\0':                       /* null character                    */
        scan_pointer++;                /* just step over this               */
        break;
    }
    length = endptr - scan_pointer;    /* calculate a new length            */
                                       /* do the next scan                  */
    scan_pointer = mempbrk(scan_pointer, delimiters, length - buffer_index );
  }
//if (last_scan != endptr - end_size)  /* last thing at the buffer end?     */
  if (last_scan != endptr - 1 )        /* last thing at the buffer end?     */
    (*count)--;                        /* have an unterminated line         */
  return endptr - buffer + 1;          /* return the final count            */
}

LONG count_stream_lines(               /* count lines in a buffer           */
  PCHAR  buffer,                       /* start of buffer                   */
  LONG   length,                       /* buffer length                     */
  PCHAR  end_char,                     /* end-of-line marker                */
  LONG   end_size )                    /* size of end-of-line marker        */
/******************************************************************************/
/* Function: Return count of lines found in a buffer                          */
/******************************************************************************/
{
  PCHAR   scan_pointer;                /* scanning pointer                  */
  PCHAR   last_scan;                   /* last scan position                */
  PCHAR   endptr;                      /* end of the buffer                 */
  LONG    linecount;                   /* current linecount                 */
//CHAR    delimiters[4];               /* search delimiters                 */
//CHAR    delimiters[] = { ctrl_z, nl, '\0' };  /* delimiters               */
  CHAR    delimiters[] = { nl, '\0' }; /* delimiters                        */

//delimiters[0] = ctrl_z;              /* fill in the EOF character         */
//delimiters[1] = end_char[0];         /* fill in first line_end character  */
//delimiters[2] = end_char[1];         /* add the string end                */
//delimiters[3] = '\0';                /* add the string end                */

  linecount = 0;                       /* no lines yet                      */
  endptr = buffer + length;            /* set the end location              */
  last_scan = buffer;                  /* save the start position           */
                                       /* scan for a important character    */
  scan_pointer = mempbrk(buffer, delimiters, length);
  while (scan_pointer != NULL) {       /* found an important one?           */
    switch (*scan_pointer) {           /* process the located character     */

      case nl:                         /* either a new line or carriage     */
//    case cr:                         /* return character                  */
                                       /* full carriage return?             */
//      if (memcmp(scan_pointer, end_char, end_size) == 0) {
          linecount++;                 /* count the line                    */
                                       /* step over the line end character  */
//        scan_pointer += end_size;    /* match checked length!             */
          scan_pointer++;              /* - match checked length!           */
          last_scan = scan_pointer;    /* save the last position            */
//      }
//      else {                         /* is it a single NL or CR?          */
//        scan_pointer++;              /* step over it                      */
//        linecount++;                 /* count the line                    */
//        last_scan = scan_pointer;    /* save the last position            */
//      }
        break;

//    case ctrl_z:                     /* end of file character             */
//                                     /* return if this is the last char.  */
//      if ( scan_pointer == endptr - 1 ) {
//        if (last_scan != scan_pointer) /* a real line before this?        */
//          linecount++;                 /* count this line                 */
//        return linecount;
//      }
//      linecount++;                   /* count this line                   */
//      scan_pointer++;
//      last_scan = scan_pointer;      /* save the last position            */
//      break;

      case '\0':                       /* null character                    */
        scan_pointer++;                /* just step over this               */
        break;
    }
    length = endptr - scan_pointer;    /* calculate a new length            */
                                       /* do the next scan                  */
    scan_pointer = mempbrk(scan_pointer, delimiters, length);
  }
  if (last_scan != endptr)             /* last thing at the buffer end?     */
    linecount++;                       /* have an unterminated line         */
  return linecount;                    /* return the final count            */
}

REXXOBJECT read_variable_line(
    REXXOBJECT   self,                 /* target stream object              */
    STREAM_INFO *stream_info,          /* current stream information        */
    PCHAR        end_char,             /* end-of-line marker                */
    LONG         end_size )            /* size of end-of-line marker        */
/******************************************************************************/
/* Function:   read in a variable length record                               */
/******************************************************************************/
{
   PCHAR   read_buffer;                /* buffer used for reading           */
   PCHAR   new_buffer;                 /* extended buffer allocation        */
   PCHAR   scan_pointer;               /* location of a delimiter character */
   LONG    read_count;                 /* count of characters read          */
   LONG    read_index;                 /* location of the next read         */
   LONG    line_length;                /* current line_length               */
   LONG    buffer_length;              /* current length of the buffer      */
   LONG    buffer_index;               /* index within the buffer           */
   LONG    current_buffer_size;        /* current buffer size               */
   CHAR    delimiters[4];              /* search delimiters                 */
   long    read_buffer_size;           /* buffer size                       */
   long    delimiterLength;            /* length of the delimiter           */

   if (stream_info->flags.transient)   /* transient stream?                 */
     read_buffer_size = 256;           /* read a larger buffer              */
   else
     read_buffer_size = 128;           /* use smaller buffer for files      */

   read_buffer = get_buffer(read_buffer_size);
                                       /* record current buffer size        */
   current_buffer_size = read_buffer_size;
   line_length = 0;                    /* set the initial line length       */
   buffer_index = 0;                   /* get the buffer index              */
   read_index = 0;                     /* get the buffer index              */
   buffer_length = 0;                  /* actual buffer length read         */
// delimiters[0] = ctrl_z;             /* fill in the EOF character         */
   delimiters[0] = '\r';               /* fill in first line_end character  */
   delimiters[1] = '\n';               /* and the line feed character       */
   delimiters[2] = '\0';               /* add the string end                */
   line_length = -1;                   /* set the "not found" indicator     */
   delimiterLength = 0;                /* no delimiter found yet            */
                                       /* read stream into read buffer      */
                                       /* beg. code simplification          */
   read_count = read_stream_buffer(stream_info, stream_info->flags.transient, read_buffer, read_buffer_size);
   if (read_count > 0) {
     buffer_length += read_count;      /* adjust the length read            */

     while (delimiterLength == 0) {    /* scan for a important character    */
       /* scan all data read to remove single character delimiters */
       scan_pointer = mempbrk(&read_buffer[buffer_index], delimiters, buffer_length - buffer_index );

       /* now we need a special handling if we find a single     */
       /* delimiter at the end of the buffer, we just ignore it and read    */
       /* another block to make sure no delimiter is crossing the buffer    */
       /* boundary                                                          */
       if ( scan_pointer == read_buffer + current_buffer_size - 1 )
          scan_pointer = NULL;

       if (scan_pointer != NULL) {     /* found an important one?           */
                                       /* calculate the new index           */
         buffer_index = scan_pointer - read_buffer;
         switch (*scan_pointer) {      /* process the located character     */

//         case ctrl_z:                /* if EOF character                  */
//                                     /* record the line length            */
//           line_length = buffer_index;
//           delimiterLength = 1;      /* exit the loop                     */
//
//           /* classic rexx sets eof immediately for character devices     */
//           if (stream_info->flags.transient)
//           {
//               stream_info->error = 0;                /* set the error information  */
//               stream_info->state = stream_eof_state; /* place this in an eof state */
//           }
//           break;                    /* nothing else required             */

           case '\0':                  /* null character                    */
                                       /* just step over the position       */
             buffer_index++;
             continue;                 /* go around again                   */

//         case nl:                    /* new line character                */
           case cr:                    /* carriage return                   */
                                       /* record the line length            */
             line_length = scan_pointer - read_buffer;
                                       /* check for \r\n                    */
//           if (memcmp(scan_pointer, &delimiters[1], 2) == 0)
             if ( *(scan_pointer +1) == nl )
               delimiterLength = 2;    /* get the whole thing               */
             else
             {
               buffer_index++;
               continue;               /* go around again                   */
             }
             break;                    /* finished reading                  */

           case nl:                    /* new line character                */
                                       /* record the line length            */
             line_length = scan_pointer - read_buffer;
             delimiterLength = 1;      /* just the single delimiter char    */

             break;                    /* finished reading                  */
         }
       }
       else {
                                       /* set the new buffer index          */
         buffer_index = buffer_length - end_size + 1;
                                       /* is next char an EOF?              */
//       if (end_size > 1 && *(read_buffer + buffer_index) == ctrl_z) {
//         line_length = buffer_index;
//         delimiterLength = 1;        /* finished reading                  */
//       }
         /* the following situation can happen on Windows 95 when pressing
            Ctrl+Z after some characters where typed before */
//       else if (feof(stream_info->stream_file) && (buffer_index < current_buffer_size))
         if (feof(stream_info->stream_file) && (buffer_index < current_buffer_size))
         {
           line_length = buffer_length;
           delimiterLength = 1;
           stream_info->error = 0;              /* set the error information         */
                                                /* place this in an eof state        */
           stream_info->state = stream_eof_state;
         }

         else {                        /* need to read more info            */
                                       /* save the index for reading        */
           read_index = current_buffer_size;
           read_buffer_size *=2;       /* duplicate buffer to reduce fragmentation on large lines */
                                       /* without, large lines may cause system resources exhausted */

                                       /* get a new buffer                  */
           new_buffer = temp_buffer(current_buffer_size + read_buffer_size);
                                       /* copy the old info over            */
/*         memcpy(new_buffer, read_buffer, current_buffer_size);            */
           memcpy(new_buffer, read_buffer, buffer_length);
                                       /* bump the buffer size              */
           current_buffer_size += read_buffer_size;
           read_buffer = new_buffer;   /* and switch the pointers           */
                                       /* now read more from the stream     */
/*         read_count = read_stream_buffer(stream_info, stream_info->flags.transient, read_buffer + read_index, read_buffer_size); */
           read_count = read_stream_buffer(stream_info, stream_info->flags.transient, read_buffer + buffer_length, read_buffer_size);
/*         if (read_count == 0) break; make a last run to set values, buffer_index must be set -1 */
           buffer_length += read_count;/* adjust the length read            */
           buffer_index--;             /* fixing x0A linein problem */
         }
       }
     } /* endwhile */
   }                                   /* end code simplification           */
   if (read_count == 0) {              /* nothing read?                     */
     if (stream_info->error != 0)      /* have a read problem?              */
                                       /* raise a notready condition        */
       stream_error(self, stream_info, stream_info->error, OREF_NULLSTRING);
     if (buffer_length == 0)           /* failure on the first read?        */
                                       /* this is an eof condition          */
       stream_eof(self, stream_info, OREF_NULLSTRING);
   }
   if (line_length == -1) {            /* no ending character found?        */
     line_length = buffer_length;      /* get the buffer length             */
     if (line_length == 1) {           /* exactly one character read?       */
//     if (*read_buffer == ctrl_z) {   /* actually the EOF character?       */
//                                     /* step past this                    */
//       stream_info->char_read_position++;
//                                     /* this is an eof condition          */
//       stream_eof(self, stream_info, OREF_NULLSTRING);
//     }
                                       /* potentially a line end?           */
//     else if (*read_buffer == end_char[0])
       if (*read_buffer == end_char[0])
         line_length = 0;              /* actually a null string            */
     }
                                       /* set the new read position         */
     stream_info->char_read_position += buffer_length;
   }
   else {                              /* found an end character            */
     buffer_index = line_length;       /* set the current end point         */
     if (delimiterLength != 0)         /* was it a linend character?        */
                                       /* step past the line end            */
       stream_info->char_read_position += buffer_index + delimiterLength;
     else                              /* step over the EOF                 */
       stream_info->char_read_position += buffer_index + 1;
   }
                                       /* need to keep the line read spot?  */
   if (stream_info->line_read_position) {
                                       /* set the location                  */
     stream_info->line_read_char_position = stream_info->char_read_position;
     stream_info->line_read_position++;/* we've moved one line              */
   }
                                       /* return the new string             */
   return RexxStringL(read_buffer, line_length);
}

/********************************************************************************************/
/* stream_charin                                                                            */
/********************************************************************************************/
RexxMethod4(REXXOBJECT, stream_charin,
     OSELF, self,                      /* target stream object              */
     BUFFER, StreamBuffer,             /* stream information block          */
     long,  position,                  /* input position                    */
     long,  read_length )              /* length to read                    */
{

   STREAM_INFO *stream_info;           /* stream information                */
   char        *buffer;                /* read buffer                       */
   OREF         result;                /* returned result string            */

   stream_info = get_stream_info();    /* get the stream block              */
   setup_read_stream(OREF_NULLSTRING); /* do needed setup                   */
   if (position != NO_LONG)            /* have a position?                  */
                                       /* set the proper position           */
     set_char_read_position(self, stream_info, position, OREF_NULLSTRING);
   if (read_length == 0)               /* nothing to read?                  */
     return OREF_NULLSTRING;           /* just return a null string         */
   else if (read_length == NO_LONG)    /* no read length specified?         */
     read_length = 1;                  /* use the default length            */
   else if (read_length < 0)           /* no read requested?                */
                                       /* this is a bad count               */
     send_exception(Error_Incorrect_method);
   buffer = get_buffer(read_length);   /* get a read buffer                 */
                                       /*  issue the read                   */
   result = (OREF)read_stream_line((RexxObject *)self, stream_info, buffer, read_length, !stream_info->flags.transient || stream_info->flags.binary);
                                       /*  reset the line positionals       */
   stream_info->line_read_char_position = 0;
   stream_info->line_read_position = 0;
   stream_info->pseudo_lines = 0;      /* reset the pseudo line count       */
   stream_info->pseudo_max_lines = 0;  /* reset the pseudo max line count   */
                                       /* this a binary transient stream?   */
   if (stream_info->flags.binary && stream_info->flags.transient)
                                       /*  make sure the count doesn't go   */
                                       /* over the reclength                */
     stream_info->char_read_position %= stream_info->stream_reclength;
   return result;                      /* return the result                 */
}

/********************************************************************************************/
/* stream_charout                                                                           */
/********************************************************************************************/
RexxMethod4(long, stream_charout,
     OSELF,  self,                     /* target stream object              */
     BUFFER, StreamBuffer,             /* stream information block          */
     STRING, string,                   /* string to write out               */
     long,   position )                /* output position                   */
{
   STREAM_INFO *stream_info;           /* stream information                */
   long result;                        /* read result                       */
   long  slength = 0;                  /* string length                     */
   PCHAR sdata;                        /* string data pointer               */

   stream_info = get_stream_info();    /* get the stream block              */

   if (string == NULLOBJECT) {         /* nothing to write?                 */
     setup_write_stream(0);            /* do needed setup                   */
     if (position == NO_LONG)          /* no positioning either?            */
       close_stream(self, stream_info);/* go close this up                  */
     else
                                       /* set the proper position           */
       set_char_write_position(self, stream_info, position, RexxInteger(slength));
     return 0;                         /* no residual                       */
   }
   slength = string_length(string);    /* get the string length             */
   sdata = string_data(string);        /* and the string pointer            */
   setup_write_stream(slength);        /* do needed setup                   */
   if (position != NO_LONG)            /* have a position?                  */
                                       /* set the proper position           */
     set_char_write_position(self, stream_info, position, RexxInteger(slength));
                                       /*  keep the write out the info      */
   result = write_stream_line(stream_info, sdata, slength);
   if (result != 0)                    /* not write everything?             */
                                       /* raise a notready condition        */
     stream_error(self, stream_info, stream_info->error, RexxInteger(result));
   reset_line_position                 /* reset the line counts             */
   stream_info->pseudo_lines = 0;      /* reset pseudo line count           */
   stream_info->pseudo_max_lines = 0;  /* reset pseudo max line count       */
   return 0;                           /*  return the remaining write count */
}

/********************************************************************************************/
/* stream_linein                                                                            */
/********************************************************************************************/
RexxMethod4(REXXOBJECT, stream_linein,
     OSELF, self,                      /* target stream object              */
     BUFFER, StreamBuffer,             /* stream information block          */
     long,  position,                  /* input position                    */
     long,  count )                    /* count of lines to read            */
{
   STREAM_INFO *stream_info;           /* stream information                */
   char        *buffer;                /* buffer pointer                    */
   long         read_length;           /* length read                       */
   REXXOBJECT   result;                /* read result                       */

   stream_info = get_stream_info();    /* get the stream block              */
   if (count != NO_LONG) {             /* no read length specified?         */
     if (count != 1 && count != 0)     /* count out of range?               */
                                       /* this is a bad count               */
       send_exception(Error_Incorrect_method);
   }
   setup_read_stream(OREF_NULLSTRING); /* do needed setup                   */
   if (position != NO_LONG)            /* have a position?                  */
                                       /* set the proper position           */
     set_line_read_position(self, stream_info, position, OREF_NULLSTRING);
   if (count == 0)                     /* nothing to read?                  */
   {
     if ( position > 0 )               /* calculate line position           */
       stream_info->pseudo_lines =
              stream_info->pseudo_max_lines -
              stream_info->line_read_position + 1;
     return OREF_NULLSTRING;           /* just return a null string         */
   }
   if (stream_info->flags.binary) {    /* binary stream?                    */
                                       /*  get buffer the size of reclength */
                                       /*   minus the charin's              */
     read_length = stream_info->stream_reclength -
         ((stream_info->char_read_position % stream_info->stream_reclength) == 0 ? 0 :
         (stream_info->char_read_position % stream_info->stream_reclength) - 1);
     buffer = get_buffer(read_length); /* get a read buffer                 */
                                       /*  issue the read                   */
     result = read_stream_line(self, stream_info, buffer, read_length, TRUE);
     if (stream_info->flags.transient) /* transient stream?                 */
                                       /*  make sure the count doesn't go   */
                                       /* over the reclength                */
       stream_info->char_read_position %= stream_info->stream_reclength;
   }
   else {                              /* non-binary stream                 */
     if (stream_info->flags.transient) /* transient stream?                 */
                                       /* use transient line ends           */
       result = read_variable_line(self, stream_info, nbt_line_end, nbt_line_end_size);
     else
                                       /* use persistent linend data        */
       result = read_variable_line(self, stream_info, line_end, line_end_size);
   }
   if (stream_info->pseudo_lines)      /* have a pseudo line count?         */
     if ( position > 0 )               /* calculate line position           */
       stream_info->pseudo_lines =
              stream_info->pseudo_max_lines -
              stream_info->line_read_position + 1;
     else
       stream_info->pseudo_lines--;    /* we have one less line             */
     return result;                    /* return the string                 */
}

/********************************************************************************************/
/* stream_lines                                                                             */
/********************************************************************************************/
RexxMethod3(long, stream_lines,
     OSELF, self,                      /* target stream object              */
     BUFFER, StreamBuffer,             /* stream information block          */
     CSTRING, strQuickFlag )           /* quick count for BIF         */
{
   STREAM_INFO *stream_info;           /* stream information                */
   long        quickflag=0;

   if (strQuickFlag != 0)
      if (stricmp(strQuickFlag,"n") == 0)
        quickflag = 1;
      else
        if (stricmp(strQuickFlag,"c") != 0 && *strQuickFlag != '\0' )
          send_exception(Error_Incorrect_method);
   stream_info = get_stream_info();    /* get the stream block              */
   if (!stream_info->flags.open)       /* not open yet?                     */
                                       /* do the open                       */
     implicit_open(self, stream_info, operation_nocreate, IntegerZero);
                                       /* reset to a ready state            */
// stream_info->state = stream_ready_state;

   /* special handling for STDIN */
   if (stream_info->flags.bstd && (stream_info->fh == 0))
   {
     if (SysFileIsDevice(stream_info->fh))
#if defined(AIX) || defined(LINUX)
//     return SysPeekSTDIN(stream_info);
       return SysPeekSTD(stream_info);
#else
       return SysPeekKeyboard();
#endif
     else if (stream_info->flags.transient){/* transient stream?            */
       if (stream_info->state == stream_eof_state)/* had an EOF?            */
         return 0;                     /* the lines are zero                */
       return 1;                       /* always return one                 */
     }
   }
   else {
                                       /* write only stream?                */
     if (!stream_info->flags.read_only && !stream_info->flags.read_write)
       return 0;                       /* lines is always zero              */

     if (stream_info->flags.transient) { /* transient stream?               */
       if (stream_info->state == stream_eof_state)/* had an EOF?            */
         return 0;                       /* the lines are zero              */
       return 1;                         /* otherwise always return one     */
     }
   } /* endif */

   if (stream_info->flags.binary) {    /* opened as a binary stream?        */
     long fudge = 0;                   /* calculation fudge factor          */
                                       /* at the last line?                 */
     if (stream_size(stream_info) == stream_info->char_read_position - 1)
       return 0;                       /* this is zero                      */
                                       /* have a partially read line?       */
     if (stream_size(stream_info) % stream_info->stream_reclength)
        fudge = 1;                     /* add a fudge factor                */
                                       /* return remaining line count       */
     return ((stream_size(stream_info) / stream_info->stream_reclength)  + fudge) -
         ((stream_info->char_read_position-1) / stream_info->stream_reclength);
   }
   else {                              /* non-binary persistent stream      */
     long lines = 0;                   /* count of lines                    */
     long buffer_size;                 /* size of the buffer                */
     char *buffer;                     /* read buffer                       */
     long buffer_count;                /* amount of data in the buffer      */

                                       /* if the stream size is equal to the*/
                                       /*  current position return zero     */
     if (stream_info->char_read_position > stream_size(stream_info))
       return 0;                       /* this is always zero               */
     if (stream_info->pseudo_lines)    /* have a pseudo line count?         */
                                       /* just use it                       */
       //return stream_info->pseudo_lines;
       return (quickflag == 0) ? stream_info->pseudo_lines : 1;
     setup_read_stream(IntegerZero);   /* do needed setup                   */

     /* if the LINES method is called for the BIF then a quick   */
     /* check is ok, else do the full line count as previously              */
     if ( quickflag == 1 ) {
       CHAR    cReadChar;

       /* if we are not yet at the end of the file return 1 */
       if (stream_info->char_read_position < stream_size(stream_info))
         return 1;

       buffer_count = read_stream_buffer(stream_info, FALSE, &cReadChar, 1);
       if (stream_info->error != 0)      /* have a read problem?            */
                                         /* raise a notready condition      */
         stream_error(self, stream_info, stream_info->error, IntegerZero);
                                         /* count the lines in the buffer   */
//     if ( (buffer_count != 0) && (cReadChar != ctrl_z) )
       if (buffer_count != 0)
         lines = 1;
       else
         lines = 0;
     } else {
                                         /* calculate a large buffer        */
       buffer_size = (stream_size(stream_info) - stream_info->char_read_position) + 2;
       /* read very large files in chunks */
       if (buffer_size > MAX_COUNTBUFFER) {
         /* treat large read requests */
         long chunk = MAX_COUNTBUFFER;
         long remain = buffer_size;
                                        /* using malloc instead of internal */
                                        /* Rexx memory will save big        */
                                        /* objects from being allocated     */
                                        /* the buffer is one byte bigger    */
                                        /* than normal read-in amount to    */
                                        /* ensure that '0d0a'x will not be  */
                                        /* cut in half (one extra byte will */
                                        /* be read in that case)            */
         buffer = (char*) malloc(sizeof(char)*(MAX_COUNTBUFFER+1));
         while (remain) {
           if (remain > chunk) {
             buffer_count = read_stream_buffer(stream_info, FALSE, buffer, chunk);
             remain -= buffer_count;    /* subtract number of read bytes    */
           }
           else {
             buffer_count = read_stream_buffer(stream_info, FALSE, buffer, remain);
             chunk = buffer_count;
             remain = 0;
           }

           if (stream_info->error != 0) {
             free(buffer);
             stream_error(self, stream_info, stream_info->error, IntegerZero);
           }
                                        /* check if buffer ends with '0d'x  */
           if (remain) {                /* (might be a truncated '0d0a'x!)  */
             if (buffer[chunk-1] == 0x0d) {
                                        /* read next byte also (completing  */
                                        /* a possible '0d0a'x sequence)     */
               read_stream_buffer(stream_info, FALSE, buffer+chunk, 1);
               remain--;
               buffer_count++;
                                        /* this check is needed, because    */
                                        /* count_stream_lines() implies line*/
                                        /* end in a block!                  */
               if (buffer[chunk] != 0x0a) lines--;
             }
                                        /* see comment above                */
             else if (buffer[chunk-1] != 0x0a) lines--;
           }
                                        /* sum up the lines                 */
           lines += count_stream_lines(buffer, buffer_count, line_end, line_end_size);
           stream_info->pseudo_lines = lines;
         }
         free(buffer);
         if ( stream_info->pseudo_stream_size >= stream_info->line_read_char_position )
            stream_info->pseudo_max_lines =
                       lines + stream_info->line_read_position -
                       ( stream_info->line_read_position > 0 );
         else
            lines = 0;
       } else {
         /* the "old" code will be executed if file is acceptably small */
         buffer = temp_buffer(buffer_size);/* get a buffer                    */
                                           /* read from stream up to the end  */
         buffer_count = read_stream_buffer(stream_info, FALSE, buffer, buffer_size);
         if (stream_info->error != 0)      /* have a read problem?            */
                                           /* raise a notready condition      */
           stream_error(self, stream_info, stream_info->error, IntegerZero);
                                           /* count the lines in the buffer   */
         lines = count_stream_lines(buffer, buffer_count, line_end, line_end_size);
         stream_info->pseudo_lines = lines; /* record the latest line count   */
         if ( stream_info->pseudo_stream_size >= stream_info->line_read_char_position )
            stream_info->pseudo_max_lines =    /* record the latest line count */
                       lines + stream_info->line_read_position -
                       ( stream_info->line_read_position > 0 );
         else
            lines = 0;
       }
     } /* endif */
     return lines;                     /* return the line count             */
   }
}

/********************************************************************************************/
/* stream_chars                                                                             */
/********************************************************************************************/
RexxMethod2(long, stream_chars,
     OSELF, self,                      /* target stream object              */
     BUFFER, StreamBuffer )            /* stream information block          */
{
   STREAM_INFO *stream_info;           /* stream information                */
   long rc;                            /* remaining characters              */

   stream_info = get_stream_info();    /* get the stream block              */
   if (!stream_info->flags.open)       /* not open yet?                     */
                                       /* do the open                       */
     implicit_open(self, stream_info, operation_nocreate, IntegerZero);
                                       /* reset to a ready state            */
// stream_info->state = stream_ready_state;

   /* special handling for STDIN */
   if (stream_info->flags.bstd && (stream_info->fh == 0))
   {
     if (SysFileIsDevice(stream_info->fh))
#if defined(AIX) || defined(LINUX)
       return SysPeekSTD(stream_info);
#else
       return SysPeekKeyboard();
#endif
     else if (stream_info->flags.transient){/* transient stream?            */
       if (stream_info->state == stream_eof_state)/* had an EOF?            */
         return 0;                     /* the lines are zero                */
       return 1;                       /* always return one                 */
     }
   }
   else {
                                       /* write only stream?                */
     if (!stream_info->flags.read_only && !stream_info->flags.read_write)
       return 0;                       /* always zero characters            */
     if (stream_info->flags.transient) /* transient stream?                 */
       return 1;                       /* always return one                 */
   } /* endif */
                                       /* return the remaining count        */
   /* check for a negative return value and set it to 0 if neces.*/
   rc = stream_size(stream_info) - (stream_info->char_read_position-1);
   if ( rc < 0 )
     rc = 0;
   return rc;
}

/********************************************************************************************/
/* stream_lineout                                                                           */
/********************************************************************************************/
RexxMethod4(long, stream_lineout,
     OSELF,  self,                     /* target stream object              */
     BUFFER, StreamBuffer,             /* stream information block          */
     STRING, string,                   /* string to write out               */
     long,   position)                 /* position to write out             */
{
   STREAM_INFO *stream_info;           /* stream information                */
   long  slength = 0;                  /* string length                     */
   PCHAR sdata;                        /* string data pointer               */
   LONG  result;                       /* returned result                   */

   stream_info = get_stream_info();    /* get the stream block              */
   if (string == NULLOBJECT) {         /* nothing to write?                 */
     setup_write_stream(0);            /* do needed setup                   */
     if (stream_info->flags.binary)    /* opened in binary mode?            */
                                       /* complete the line                 */
       complete_line(self, stream_info);
     if (position == NO_LONG)          /* no positioning either?            */
       close_stream(self, stream_info);/* go close this up                  */
     else
                                       /* set the proper position           */
       set_line_write_position(self, stream_info, position, RexxInteger(slength));
     return 0;                         /* no residual                       */
   }
   setup_write_stream(1);              /* do needed setup                   */
   if (position != NO_LONG)            /* have a position?                  */
                                       /* set the proper position           */
     set_line_write_position(self, stream_info, position, IntegerOne);
/*
   else                         // added by IH to get lines added to the end
     set_stream_position(stream_info->char_write_position-1);    // up to here
*/
   reset_line_position;                /* reset the line counters           */

   if (stream_info->flags.binary) {    /* opened in binary mode?            */
     slength = string_length(string);  /* get the string length             */
     sdata = string_data(string);      /* and the string pointer            */
                                       /* if the line_out is longer than    */
                                       /* reclength plus any char out data  */
                                       /*  raise a syntax error - invalid   */
                                       /* call to routine                   */
     if (stream_info->stream_reclength < (signed)slength + ((stream_info->char_write_position % stream_info->stream_reclength) - 1))
                                       /* this is an error                  */
       send_exception(Error_Incorrect_call);
                                       /* same length as record length?     */
     if (stream_info->stream_reclength == (signed)slength)
                                       /* just write out the line as is     */
       result = write_stream_line(stream_info, sdata, stream_info->stream_reclength);
     else                              /* write out the line                */
       result = write_fixed_line(self, stream_info, sdata, slength);
     if (result != 0)                  /* not write everything?             */
                                       /* raise a notready condition        */
       stream_error(self, stream_info, stream_info->error, IntegerOne);
     return  0;                        /* the line was written              */
   }
   else {                              /* non-binary linein                 */
     if (stream_info->pseudo_lines) {  /* have a pseudo-line count?         */
                                       /* appending?                        */
       if (stream_info->flags.append ||
           stream_info->char_write_position == stream_size(stream_info))
       {
         ++stream_info->pseudo_lines;  /* update the count                  */
         ++stream_info->pseudo_max_lines;  /* update the max count          */
       }
       else
         stream_info->pseudo_lines = 0;/* reset this to zero                */
     }
     slength = string_length(string);  /* get the string length             */
     sdata = string_data(string);      /* and the string pointer            */
                                       /*  keep the write out the info      */
     result = write_stream_line(stream_info, sdata, slength);
     if (result == 0) {                /* write the line ok?                */
//     if (stream_info->flags.std) {   /* need to fudge the linend?         */
//       result = write_stream_line(stream_info, std_line_end, std_line_end_size);
//                                     /* adjust by one                     */
//       stream_info->char_write_position += 1;
//     }
//     else                            /* use normal linends                */
       result = write_stream_line(stream_info, line_end, line_end_size);
     }
                                       /* need to adjust line positions?    */
     if (stream_info->line_write_position) {
       ++stream_info->line_write_position;
       stream_info->line_write_char_position = stream_info->char_write_position;
     }
                                       /* all written?                      */
     if (result != 0)                  /* not write everything?             */
                                       /* raise a notready condition        */
       stream_error(self, stream_info, stream_info->error, IntegerOne);
     return 0;                         /* line written correctly            */
   }
}

/********************************************************************************************/
/* stream_close                                                                             */
/********************************************************************************************/
RexxMethod2(CSTRING, stream_close,
    OSELF, self,                       /* target stream object              */
    BUFFER, StreamBuffer )             /* stream information block          */
{
   STREAM_INFO *stream_info;           /* stream information                */

   stream_info = get_stream_info();    /* get the stream block              */
   if (stream_info == NULL)            /* not properly initialized?         */
     return "";                        /* just get out of here              */

   if (!stream_info->flags.open) {     /* not open yet?                     */
     stream_info->state = stream_unknown_state;
     return "";                        /* return empty string               */
   }
   close_stream(self, stream_info);    /* go close the stream               */

   // Free file buffer so it can be collected next time garbage collection
   // is invoked.
   if (stream_info->bufferAddress)
   {
      stream_info->bufferAddress = NULL;
      stream_info->bufferLength = 0;
      RexxVarSet(c_stream_buffer, OREF_NULL);  // stream object now loses reference to buffer object
   }

   return "READY:";                    /* return the success indicator      */
}

/********************************************************************************************/
/* stream_flush                                                                             */
/********************************************************************************************/
RexxMethod2(CSTRING, stream_flush,
    OSELF, self,                       /* target stream object              */
    BUFFER, StreamBuffer )             /* stream information block          */
{
   STREAM_INFO *stream_info;           /* stream information                */
   CHAR         work[30];              /* error information buffer          */

   stream_info = get_stream_info();    /* get the stream block              */
   if (buffer_flush != 0) {            /* try to flush                      */
     sprintf(work, "ERROR:%d", errno); /* format the error return           */
                                       /* go raise a notready condition     */
     stream_error(self, stream_info, errno, RexxString(work));
   }
   return "READY:";                    /* return success indicator          */
}

/********************************************************************************************/
/* stream_open - open a stream                                                              */
/********************************************************************************************/
RexxMethod3(CSTRING, stream_open,
     OSELF, self,                      /* target stream object              */
     BUFFER, StreamBuffer,             /* stream information block          */
     CSTRING, ts)                      /* open command string               */
{

/* fields that parse will fill in */
unsigned long Parse_Fields[7] = {
  0,                                   /* oflag                             */
  0,                                   /* pmode                             */
  0,                                   /* fdopen_long                       */
  0,                                   /* i_binary                          */
  0,                                   /* i_nobuffer                        */
  0,                                   /* rdonly                            */
  0                                    /* shared                            */
 };

/* Action table for open parameters */
ATS  OpenActionread[] = {
      {ME,       sizeof(oflag), (void *)oflag_index, errcode,(void *)&o_creat,0},
      {BitOr,    sizeof(oflag), (void *)oflag_index, errcode,(void *)&o_rdonly,0},
      {CopyItem, sizeof(long),  (void *)rdonly_index, errcode,(void *)&one,0},
      {BitOr,    sizeof(pmode), (void *)pmode_index, errcode,(void *)&s_iread,0},
      {CopyItem, sizeof(c_read), (void *)fdopen_long_index, errcode,(void *)c_read,0},
      {0,0,0,0,0,0}
     };
ATS OpenActionwrite[] = {
      {ME,       sizeof(oflag), (void *)oflag_index, errcode,(void *)&o_rdwr,0},
      {MF,       sizeof(long),  (void *)rdonly_index, errcode,0,0},
      {BitOr,    sizeof(oflag), (void *)oflag_index, errcode,(void *)&wr_creat,0},
      {BitOr,    sizeof(pmode), (void *)pmode_index, errcode,(void *)&s_iwrite,0},
      {CopyItem, sizeof(c_write), (void *)fdopen_long_index, errcode,(void *)c_write,0},
      {0,0,0,0,0,0}
     };
ATS OpenActionboth[] = {
      {ME,       sizeof(oflag), (void *)oflag_index, errcode,(void *)&o_wronly,0},
      {MF,       sizeof(long),  (void *)rdonly_index, errcode,0,0},
      {BitOr,    sizeof(oflag), (void *)oflag_index, errcode,(void *)&rdwr_creat,0},
      {BitOr,    sizeof(pmode), (void *)pmode_index, errcode,(void *)&iread_iwrite,0},
      {CopyItem, sizeof(c_both), (void *)fdopen_long_index, errcode,(void *)c_both,0},
      {0,0,0,0,0,0}
     };
ATS OpenActionappend[] = {
      {MF,       sizeof(long),  (void *)rdonly_index, errcode,0,0},
      {BitOr,    sizeof(oflag), (void *)oflag_index, errcode,(void *)&o_append,0},
      {CopyItem, sizeof(c_append), (void *)fdopen_long_index, errcode,(void *)c_append,0},
      {0,0,0,0,0,0}
     };
ATS OpenActionreplace[] = {
      {BitOr,sizeof(oflag), (void *)oflag_index, errcode,(void *)&o_trunc,0},
      {0,0,0,0,0,0}
     };
ATS OpenActionnobuffer[] = {
      {CopyItem,sizeof(i_nobuffer), (void *)i_nobuffer_index, errcode,(void *)&one,0},
      {0,0,0,0,0,0}
     };
ATS OpenActionbinary[] = {
      {MF,sizeof(i_binary), (void *)i_binary_index, errcode,0,0},
      {BitOr,sizeof(oflag), (void *)oflag_index, errcode,(void *)&o_binary,0},
      {CopyItem,sizeof(i_binary), (void *)i_binary_index, errcode,(void *)&one,0},
      {ConcatItem,sizeof(c_binary) - 1, (void *)fdopen_long_index, errcode,(void *)c_binary,0},
      {0,0,0,0,0,0}
     };
ATS OpenActionreclength[] = {
      {MI,sizeof(i_binary), (void *)i_binary_index, errcode,(void *)&one,0},
      {CallItem,sizeof(int),0, errcode,0,reclength_token},
      {0,0,0,0,0,0}
     };

ATS OpenActionshared[] = {
      {CopyItem,sizeof(shared), (void *)shared_index, errcode,(void *)&sh_denyno,0},
      {0,0,0,0,0,0}
     };

ATS OpenActionsharedread[] = {
      {CopyItem,sizeof(shared), (void *)shared_index, errcode,(void *)&sh_denywr,0},
      {0,0,0,0,0,0}
     };

ATS OpenActionsharedwrite[] = {
      {CopyItem,sizeof(shared), (void *)shared_index, errcode,(void *)&sh_denyrd,0},
      {0,0,0,0,0,0}
     };

#if defined( O_SYNC )
ATS OpenActionautosync[] = {
      {BitOr,sizeof(oflag), (void *)oflag_index, errcode,(void *)&o_sync,0},
      {0,0,0,0,0,0}
     };
#endif

#if defined( O_RSHARE )
ATS OpenActionshareread[] = {
      {BitOr,sizeof(oflag), (void *)oflag_index, errcode,(void *)&o_rshare,0},
      {0,0,0,0,0,0}
     };
#endif

#if defined( O_NSHARE )
ATS OpenActionnoshare[] = {
      {BitOr,sizeof(oflag), (void *)oflag_index, errcode,(void *)&o_nshare,0},
      {0,0,0,0,0,0}
     };
#endif

#if defined( O_DELAY ) && defined( O_RSHARE ) && defined( O_NSHARE )
ATS OpenActiondelay[] = {
      {MI,   sizeof(oflag), (void *)oflag_index, errcode,(void *)&o_rshare,0},
      {MI,   sizeof(oflag), (void *)oflag_index, errcode,(void *)&o_nshare,0},
      {BitOr,sizeof(oflag), (void *)oflag_index, errcode,(void *)&o_delay,0},
      {0,0,0,0,0,0}
     };
#endif

/* Token table for open parameters */
TTS  tts[] = {
      {"READ",3,      OpenActionread,0},
      {"WRITE",1,     OpenActionwrite,0},
      {"BOTH",2,      OpenActionboth,0},
      {"APPEND",2,    OpenActionappend,0},
      {"REPLACE",3,   OpenActionreplace,0},
      {"NOBUFFER",3,  OpenActionnobuffer,0},
      {"BINARY",2,    OpenActionbinary,0},
      {"RECLENGTH",3, OpenActionreclength,0},
      {"SHARED",6,    OpenActionshared,0},
      {"SHAREREAD",6, OpenActionsharedread,0},
      {"SHAREWRITE",6,OpenActionsharedwrite,0},

#if defined( O_SYNC )
      {"AUTOSYNC",2, OpenActionautosync,0},
#endif
#if defined( O_RSHARE )
      {"SHAREREAD",1,OpenActionshareread,0},
#endif
#if defined( O_NSHARE )
      {"NOSHARE",3,  OpenActionnoshare,0},
#endif
#if defined( O_DELAY )
      {"DELAY",1,    OpenActiondelay,0},
#endif
      {"\0",0,NULL, unknown_tr}
    };

long second_oflag;
long second_pmode;
char fdopen_type[4];
char second_fdopen[4];

TTS *ttsp;

   STREAM_INFO *stream_info;           /* stream information                */
   struct stat stat_info;              /* file statistics                   */
   CHAR   work[30];                    /* work buffer                       */
   char   char_buffer;                 /* single character buffer           */

   i_nobuffer = 0;                     /* default to buffered               */
   i_binary = 0;                       /* non-binary,                       */
   oflag = 0;                          /* clear the flags                   */
   pmode = 0;                          /* and the protection mode           */
   second_oflag = 0;                   /* for the secondary open mode as    */
   second_pmode = 0;                   /* well                              */
   fdopen_long = 0;                    /* clear the open types              */
   shared = SH_DENYRW;                 /* def. open is non shared           */
#ifdef JAPANESE
   if (sharedOpen) shared = SH_DENYNO;
#endif
   fdopen_type[0] = '\0';
   second_fdopen[0] = '\0';
   ttsp = tts;

   stream_info = get_stream_info();    /* get the stream block              */
   if (stream_info->flags.open)        /* already open?                     */
     close_stream(self, stream_info);  /* go close the stream               */
   if (stream_info->flags.bstd)        /* standard stream?                  */
     return std_open(stream_info, ts); /* handle as a standard stream       */
   else if (stream_info->flags.handle_stream)
                                       /* do a handle open                  */
     return handle_open(self, stream_info, ts);


                                          /* initialize the stream info structure           */
                                          /*  in case this is not the first open for this stream */
   strcpy(stream_info->full_name_parameter,"\0");
   stream_info->stream_file = NULL;
   stream_info->pseudo_stream_size = 0;
   stream_info->pseudo_lines = 0;
   stream_info->pseudo_max_lines = 0;
   stream_info->stream_reclength = 0;
   stream_info->flags.read_only = 0;
   stream_info->flags.write_only = 0;
   stream_info->flags.read_write = 0;
   stream_info->flags.bstd = 0;
   stream_info->flags.append = 0;
   stream_info->char_read_position = 1;
   stream_info->char_write_position = 1;
   stream_info->line_read_position = 1;
   stream_info->line_write_position = 1;
   stream_info->line_read_char_position = 1;
   stream_info->line_write_char_position = 1;
   stream_info->flags.nobuffer = 0;
   stream_info->flags.last_op_was_read = 1;
   stream_info->flags.transient = FALSE;
   stream_info->flags.binary = FALSE;

   table_fixup(ttsp, Parse_Fields);    /* fix up the parsing tables         */
   if (NO_CSTRING != ts) {             /* have parameters?                  */
                                       /* go process the syntax             */
     if (parser(ttsp, ts, (void *)(&stream_info->stream_reclength)) != 0)
                                       /* this is an error                  */
       send_exception(Error_Incorrect_call);
   }
                                       /* save the open parameters in the   */
                                       /* stream block                      */
   strcpy(fdopen_type,(const char *)&fdopen_long);
   full_name_parameter(stream_info);   /* get the fully qualified name      */

                                       /* if replace and binary specified,  */
                                       /* but not reclength, give back a    */
                                       /* syntax error - don't know what to */
                                       /* do                                */
   if (i_binary && (oflag & o_trunc) && !stream_info->stream_reclength)
                                       /* this is an error                  */
     send_exception(Error_Incorrect_call);
                                       /* if write and append specified     */
                                       /* make sure append won              */
   if ((oflag & o_append) && (oflag & o_creat)) {
     if (oflag & rdwr_creat)           /* need to replace?                  */
       strcpy(fdopen_type,c_append);   /* this is create append             */
     else                              /* this is a write append            */
       strcpy(fdopen_type,c_wr_append);
     if (i_binary)                     /* binary file?                      */
       strcat(fdopen_type,c_binary);   /* add on the binary part            */
   }
                                       /* read/write/both/append not        */
                                       /* specified default both            */
   if (!(oflag & (o_wronly | rdwr_creat )) && !rdonly) {
      oflag |= o_rdwr | rdwr_creat;    /* set this up for read/write mode   */
      if (!(oflag & o_append)) {       /* not append mode?                  */
        strcpy(fdopen_type,c_both);    /* open both ways                    */
        if (i_binary)                  /* need it in binary mode?           */
          strcat(fdopen_type,c_binary);/* tack on the binary indicator      */
      }
      pmode = iread_iwrite;            /* save the pmode info               */
   }
/********************************************************************************************/
/*NonBinary persistant streams are opened binary. In order to know if it was user specified */
/*            or not the i_binary field is used. If the user specified binary the stream    */
/*            will be treated as binary, otherwise it will be treated as non - binary.      */
/*            The difference is how imbedded nulls and line end's are handled.              */
/*            If there is a non persistant stream type that does not end in a colon the     */
/*            following check will need to be changed                                       */
/********************************************************************************************/
    if (!i_binary &&
       !(stream_info->name_parameter[strlen(stream_info->name_parameter)-1] == ':')) {
      strcat(fdopen_type,c_binary);    /* add on the binary indicator for   */
      oflag |= o_binary;               /* devices and flip the binary flag  */
    }
   if (rdonly) {                       /* read-only stream?                 */
                                       /* check if the stream exists        */
     if (get_stat(stream_info->full_name_parameter,&stat_info)) {
                                       /* format the error return           */
       sprintf(work, "ERROR:%d", errno);
                                       /* go raise a notready condition     */
       stream_error(self, stream_info, errno, RexxString(work));
     }
     stream_info->flags.read_only = 1; /* set the read_only flag            */
                                       /* and clear all of the write        */
                                       /* information                       */
     stream_info->char_write_position = 0;
     stream_info->line_write_position = 0;
     stream_info->line_write_char_position = 0;
   }
   if (oflag & o_rdwr)                 /* read/write specified?             */
                                       /* set the flag                      */
      stream_info->flags.read_write = 1;
    if (oflag & o_append)              /* appending?                        */
      stream_info->flags.append = 1;   /* flag it also                      */
                                       /* if write only specified           */
                                       /*      - try both first             */
   if (oflag & o_wronly) {
                                       /* set both flags                    */
      stream_info->flags.read_write = 1;
      stream_info->flags.write_only = 1;
      second_oflag = oflag;            /* copy the open information         */
      second_pmode = pmode;
      if (!(oflag & o_append)) {       /* no appending?                     */
        strcpy(fdopen_type,c_both);    /* open both read and write          */
        strcpy(second_fdopen,c_write); /* then write only                   */
      }
      else {
        strcpy(fdopen_type,c_append);  /* open for appending in both        */
                                       /* ways                              */
        strcpy(second_fdopen,c_wr_append);
      }
      if (oflag & o_binary) {          /* binary requested?                 */
        strcat(fdopen_type,c_binary);  /* add the binary flag to both       */
        strcat(second_fdopen,c_binary);
      }
      oflag &= ~o_wronly;              /* turn off the write only flag      */
      oflag |= rdwr_creat;             /* and turn on the read/write        */
      pmode = iread_iwrite;            /* set the new pmode                 */
   }

   /* if opening a printer port, select second open flags */
   /* to open the file in write-only mode                                   */
   if ( memicmp(stream_info->full_name_parameter, "\\DEV\\LPT", 8) == 0 ) {
     /* this is a printer port on OS/2 (PRN will be converted to the        */
     /* current LPTx port)                                                  */
     second_oflag = o_binary | wr_creat;
     second_pmode = s_iwrite;
     strcpy(second_fdopen, c_default_write);    /* wb */
   } /* endif */
                                       /* now open the stream               */
   open_the_stream_shared(oflag,pmode,fdopen_type,shared);
                                       /* if there was an open error and    */
                                       /*  we have the info to try again -  */
                                       /*  doit                             */
   if ((stream_info->stream_file == NULL) && second_oflag) {
                                       /* try opening again                 */
     open_the_stream_shared(second_oflag,second_pmode,second_fdopen,shared);
     stream_info->flags.read_write = 0;/* turn off the read/write flag      */
     stream_info->flags.write_only = 1;/* turn on the write only flag       */
   }
                                       /* if there was an error             */
   if (stream_info->stream_file == NULL) {
     sprintf(work, "ERROR:%d", errno); /* format the error return           */
                                       /* go raise a notready condition     */
     stream_error(self, stream_info, errno, RexxString(work));
   }

   fstat(stream_info->fh, &stat_info); /* get the file information          */
                                       /* is this a device or is            */
                                       /* no buffering requested?           */
   if (stat_info.st_mode&S_IFCHR || i_nobuffer)
     set_nobuffer;                     /* turn it off                       */

/********************************************************************************************/
/*          if it is a persistant stream put the write character pointer at the end         */
/*   need to check if the last character is end of file and if so write over it             */
/*   if the stream was created it will have a size of 0 but this will mess up the logic     */
/*          so set it to one                                                                */
/********************************************************************************************/
   stream_size(stream_info);           /* set up the pseudo stream size     */
                                       /* persistent writeable stream?      */
   if (!_transient && (oflag & (o_wronly | rdwr_creat))) {
     if (stream_size(stream_info)) {   /* existing stream?                  */
                                       /* try to set to the end             */
       if (!set_stream_position(stream_size(stream_info) - 1)) {
                                       /* read the last character, and if   */
                                       /* it is an EOF character            */
         if (binary_read(&char_buffer,1) && ctrl_z == char_buffer)
                                       /* position to overwrite it          */
           stream_info->char_write_position = stream_size(stream_info);
         else                          /* write at the very end             */
         {
           stream_info->char_write_position = stream_size(stream_info) + 1;
                   /* error on Windows so we had to put in that */
                                                                           /* explicitly set the position       */
                   set_stream_position(stream_info->char_write_position - 1);
         }
       }
     }
                                       /* set default line positioning      */
     stream_info->line_write_position = 0;
     stream_info->line_write_char_position = 0;
   }
   stream_info->flags.open = TRUE;     /* this is now open                  */
                                       /* this is now ready                 */
   stream_info->state = stream_ready_state;
                                       /* go process the stream type        */
   get_stream_type(stream_info, i_binary);
   return "READY:";                    /* return success                    */
}

/********************************************************************************************/
/* handle_set                                                                               */
/*           sets the handle into the stream info block                                     */
/********************************************************************************************/
RexxMethod2(long, handle_set,
     BUFFER, StreamBuffer,             /* stream information block          */
     int, fh)                          /* target file handle                */
{

   STREAM_INFO *stream_info;           /* stream information                */

   stream_info = get_stream_info();    /* get the stream block              */
   stream_info->fh = fh;               /* set the handler                   */
                                       /* this is a handle                  */
   stream_info->flags.handle_stream = 1;
   return stream_info->fh;             /* and return it as a result         */
}

/********************************************************************************************/
/* std_set                                                                                  */
/*           tags this as a standard I/O stream                                             */
/********************************************************************************************/
RexxMethod1(REXXOBJECT, std_set,
     BUFFER, StreamBuffer )            /* stream information block          */
{
   STREAM_INFO *stream_info;           /* stream information                */

   stream_info = get_stream_info();    /* get the stream block              */
   stream_info->flags.bstd = TRUE;     /* turn on the std flag              */
   return NULLOBJECT;                  /* and return nothing as a result    */
}

/********************************************************************************************/
/* stream_position                                                                          */
/********************************************************************************************/
RexxMethod3(long, stream_position,
     OSELF, self,                      /* target stream object              */
     BUFFER, StreamBuffer,             /* stream information block          */
     CSTRING, ts)                      /* position command string           */
{

unsigned long Parse_Fields[7] = {      /* fields filled in by parse         */
  0,                                   /* from_current                      */
  0,                                   /* from_start                        */
  0,                                   /* from_end                          */
  0,                                   /* forward                           */
  0,                                   /* backward                          */
  0,                                   /* by_line                           */
  0                                    /* position_flags                    */
 };

long position_offset;                  /* filled in by offset routine       */
CHAR   work[30];                       /* work buffer                       */

/* parameter structure for unknown offset routine called from parser */
POSITION_PARMS Parse_Parms = {
   (long *)&position_flags,
  &position_offset
 };

/* Action table for position parameters */
ATS  Direction_From_Start[] = {
      {MF,       sizeof(from_end), (void *) from_end_index, errcode,0,0},
      {MF,       sizeof(forward), (void *) forward_index, errcode,0,0},
      {MF,       sizeof(backward), (void *) backward_index, errcode,0,0},
      {MF,       sizeof(from_start), (void *) from_start_index, errcode,0,0},
      {CopyItem, sizeof(from_start), (void *) from_start_index, errcode,(void *)&one,0},
      {CopyItem, sizeof(forward), (void *) forward_index, errcode,(void *)&one,0},
      {CopyItem, sizeof(backward), (void *) backward_index, errcode,(void *)&one,0},
      {0,0,0,0,0,0}
     };
ATS Direction_From_End[] = {
      {MF,       sizeof(from_start), (void *) from_start_index, errcode,0,0},
      {MF,       sizeof(forward), (void *) forward_index, errcode,0,0},
      {MF,       sizeof(backward), (void *) backward_index, errcode,0,0},
      {MF,       sizeof(from_end), (void *) from_end_index, errcode,0,0},
      {CopyItem, sizeof(from_end), (void *) from_end_index, errcode,(void *)&one,0},
      {CopyItem, sizeof(backward), (void *) backward_index, errcode,(void *)&minus_one,0},
      {0,0,0,0,0,0}
     };
ATS Direction_Forward[] = {
      {MF,       sizeof(from_start), (void *) from_start_index, errcode,0,0},
      {MF,       sizeof(from_end), (void *) from_end_index, errcode,0,0},
      {MF,       sizeof(backward), (void *) backward_index, errcode,0,0},
      {MF,       sizeof(forward), (void *) forward_index, errcode,0,0},
      {CopyItem, sizeof(forward), (void *) forward_index, errcode,(void *)&one,0},
      {CopyItem, sizeof(backward), (void *) backward_index, errcode,(void *)&one,0},
      {CopyItem, sizeof(from_current), (void *) from_current_index, errcode,(void *)&one,0},
      {0,0,0,0,0,0}
     };
ATS Direction_Backward[] = {
      {MF,       sizeof(from_start), (void *) from_start_index, errcode,0,0},
      {MF,       sizeof(from_end), (void *) from_end_index, errcode,0,0},
      {MF,       sizeof(forward), (void *) forward_index, errcode,0,0},
      {MF,       sizeof(backward), (void *) backward_index, errcode,0,0},
      {CopyItem, sizeof(backward), (void *) backward_index, errcode,(void *)&minus_one,0},
      {CopyItem, sizeof(from_current), (void *) from_current_index, errcode,(void *)&one,0},
      {0,0,0,0,0,0}
     };
ATS Operation_Read[] = {
      {ME,   sizeof(position_flags), (void *) position_flags_index, errcode,(void *)&operation_write,0},
      {BitOr,sizeof(position_flags), (void *) position_flags_index, errcode,(void *)&operation_read,0},
      {0,0,0,0,0,0}
     };
ATS Operation_Write[] = {
      {ME,   sizeof(position_flags), (void *) position_flags_index, errcode,(void *)&operation_read,0},
      {BitOr,sizeof(position_flags), (void *) position_flags_index, errcode,(void *)&operation_write,0},
      {0,0,0,0,0,0}
     };
ATS Position_By_Char[] = {
      {ME,   sizeof(position_flags), (void *) position_flags_index, errcode,(void *)&position_by_line,0},
      {BitOr,sizeof(position_flags), (void *) position_flags_index, errcode,(void *)&position_by_char,0},
      {0,0,0,0,0,0}
     };
ATS Position_By_Line[] = {
      {ME,   sizeof(position_flags), (void *) position_flags_index, errcode,(void *)&position_by_char,0},
      {BitOr,sizeof(position_flags), (void *) position_flags_index, errcode,(void *)&position_by_line,0},
      {0,0,0,0,0,0}
     };

/* Token table for position parameters */
TTS  tts[] = {
      {"=",1,     Direction_From_Start,0},
      {"<",1,     Direction_From_End,0},
      {"+",1,     Direction_Forward,0},
      {"-",1,     Direction_Backward,0},
      {"READ",1,  Operation_Read,0},
      {"WRITE",1, Operation_Write,0},
      {"CHAR",1,  Position_By_Char,0},
      {"LINE",1,  Position_By_Line,0},
      {"\0",0,NULL, unknown_offset}
    };

TTS *ttsp;

   STREAM_INFO *stream_info;           /* stream information                */
   long new_position = 0;              /* new stream position               */
   int result = 0;                     /* returned result                   */
   position_offset = 0;                /* the position offset               */
   position_flags = 0;                 /* positioning flags                 */
   from_start = 0;                     /* from_start flag                   */
   from_end = 0;                       /* from_end flag                     */
   forward = 0;                        /* forward movement                  */
   backward = 0;                       /* backward movement                 */
   from_current = 0;                   /* move from current position        */
   ttsp = tts;                         /* set up for parser call            */
   long retpos = 0;                    /* position to return                */

   stream_info = get_stream_info();    /* get the stream block              */

   table_fixup(ttsp, Parse_Fields);    /* fix up the parse fields           */
   if (NO_CSTRING != ts) {             /* have parameters?                  */
                                       /* call the parser to fix up         */
     if (parser(ttsp, ts, (void *)(&Parse_Parms)) != 0)
                                       /* this is an error                  */
       send_exception(Error_Incorrect_call);
   }

   if (stream_info->flags.transient)   /* trying to move a transient stream?*/
                                       /* this is an error                  */
     send_exception(Error_Incorrect_method_stream_type);

/********************************************************************************************/
/*           set up the defaults for parameters not specified                               */
/********************************************************************************************/
                                       /* position offset must be specified */
   if (!(position_flags & position_offset_specified))
                                       /* this is an error                  */
     send_exception1(Error_Incorrect_call_noarg, RexxArray2(RexxString("SEEK"),RexxString("offset")));
                                       /* if direction was not specified    */
                                       /*   default from start (absolute)   */
   if (0 == from_start + from_end + forward + labs((signed)backward)) {
     forward = 1;                      /* this is forward movement          */
     from_start = 1;                   /* from the start                    */
                                       /* for forward movement, backward is */
                                       /* set to 1 ... -1 indicates backward*/
     backward = 1;                     /* movement                          */
   }
                                       /* if read or write was not specified*/
                                       /* check the open flags for read and */
                                       /* set read. check for write and set */
                                       /* write. if open both then set both */
                                       /* flags                             */
   if (!(position_flags & operation_read) && !(position_flags & operation_write)) {
     if (stream_info->flags.read_only) /* opened read only?                 */
                                       /* move the read position            */
       position_flags |= operation_read;
                                       /* opened write only?                */
     else if (stream_info->flags.write_only)
                                       /* move the write position           */
       position_flags |= operation_write;
    else {
       position_flags |= operation_read | operation_write;

       /* set both stream pointers to last active position          */
       if (stream_info->flags.last_op_was_read) {
         stream_info->char_write_position = stream_info->char_read_position;
         stream_info->line_write_position = stream_info->line_read_position;
       } else {
         stream_info->char_read_position = stream_info->char_write_position;
         stream_info->line_read_position = stream_info->line_write_position;
       }
    }
   }
                                       /* if the write stream is being      */
                                       /* repositioned                      */
   if (position_flags & operation_write) {
     if (stream_info->flags.append)    /* opened append?                    */
       return 0;                       /* cause a notready condition        */
                                       /* else flush the buffer             */
     buffer_flush;
   }
                                       /* if positioning by line and write  */
                                       /* only stream, raise notready       */
                                       /* because we can't do reads         */
   if ((position_flags & position_by_line) && !(stream_info->flags.read_write || stream_info->flags.read_only))
     return 0;                         /* raise the notready                */
                                       /* if moving the read position -     */
   if (position_flags & operation_read)
   {
     stream_info->pseudo_lines = 0;    /* reset the pseudo lines            */
     stream_info->pseudo_max_lines = 0;  /* reset the pseudo lines          */
   }                                   /* if char or line not specified -   */
                                       /* default to char                   */
   if (!(position_flags & position_by_char) && !(position_flags & position_by_line))
     position_flags |= position_by_char;
   if (from_end) {                     /* if setting from the end           */
                                       /* resetting read?                   */
     if (position_flags & operation_read) {
                                       /* move the character position       */
       stream_info->char_read_position = stream_size(stream_info);
                                       /* and reset the line position       */
       stream_info->line_read_position = stream_info->line_read_char_position = 0;
     }

     if (position_flags & operation_write) {
                                       /* move the write position           */
       stream_info->char_write_position = stream_size(stream_info);
                                       /* and reset the line position       */
       stream_info->line_write_position = stream_info->line_write_char_position = 0;
     }
   }
                                       /* character positioning?            */
   if (position_flags & position_by_char) {
     reset_line_position               /* reset all line positioning        */
   }
                                       /* positioning non-binary line?      */
   else if (!stream_info->flags.binary) {
                                       /* go set up line positions          */
     if (!(set_line_position(self, stream_info)))
       return -1;                      /* and return all errors             */
   }
/********************************************************************************************/
/*           time to start doing some actual positioning                                    */
/********************************************************************************************/
   if (!stream_info->stream_file) {    /* check file existence              */
     sprintf(work, "ERROR:%d", ENOENT);
                                       /* raise notready condition          */
     stream_error(self, stream_info, ENOENT, RexxString(work));
   }
                                       /* if offset was specified as zero   */
                                       /* set the system position according */
                                       /* to the specified operation and    */
                                       /* positioning parms                 */
   if (position_offset == 0) {
                                       /* moving the read position?         */
     if (position_flags & operation_read) {
                                       /* try to move this                  */
       if (set_stream_position(stream_info->char_read_position - 1))
                                       /* go check the failure              */
         stream_check_eof(self, stream_info, errno, IntegerZero);
                                       /* moving the character position?    */
       if (position_flags & position_by_char)
                                       /* just return current position      */
         retpos = stream_info->char_read_position;
       else {
         if (stream_info->flags.binary)/* opened in binary mode?            */
                                       /* return calculated line position   */
           retpos = (stream_info->char_read_position/stream_info->stream_reclength) +
             (stream_info->char_read_position % stream_info->stream_reclength ? 1 : 0);
         else                          /* return setup line position        */
           retpos = stream_info->line_read_position;
       }
     }
                                       /* moving the write position         */
     if (position_flags & operation_write) {
                                       /* try to move this                  */
       if (set_stream_position(stream_info->char_write_position - 1))
                                       /* go check the failure              */
         stream_check_eof(self, stream_info, errno, IntegerZero);
                                       /* moving the character position?    */
       if (position_flags & position_by_char)
                                       /* just return current position      */
         retpos = stream_info->char_write_position;
       else {
         if (stream_info->flags.binary)/* opened in binary mode?            */
           retpos = (stream_info->char_write_position/stream_info->stream_reclength) +
               (stream_info->char_write_position % stream_info->stream_reclength ? 1 : 0);
         else                          /* return setup line position        */
           retpos = stream_info->line_write_position;
       }
     }

     return retpos;
   }
                                       /* if the offset was specified as one*/
                                       /* set the system stream position to */
                                       /* the beginning of the stream       */
   if ((position_offset == 1) && (from_start)) {
     if (set_stream_position(0))       /* try to move the position          */
                                       /* go check the failure              */
       stream_check_eof(self, stream_info, errno, IntegerZero);
                                       /* moving the read position?         */
     if (position_flags & operation_read) {
                                       /* reset both the character and line */
                                       /* read positions                    */
       stream_info->char_read_position = 1;
       stream_info->line_read_char_position = 1;
       stream_info->line_read_position = 1;
                                       /* and return the current position   */
       retpos = stream_info->char_read_position;
     }

     if (position_flags & operation_write) {
                                       /* reset both the character and line */
                                       /* read positions                    */
       stream_info->char_write_position = 1;
       stream_info->line_write_char_position = 1;
       stream_info->line_write_position = 1;
                                       /* and return the current position   */
       retpos = stream_info->char_write_position;
     }

     return retpos;
   }
                                       /* whatever the offset is now,       */
                                       /*       go for it                   */
                                       /* moving the character position?    */
   if (position_flags & position_by_char) {
                                       /* moving the read position?         */
     if (position_flags & operation_read) {
                                       /* make sure we're within the bounds */
       new_position = (((position_offset - from_start) * (signed)backward) +
           (--stream_info->char_read_position * from_current) + (stream_size(stream_info) * from_end));
                                       /* try to move to the new position   */
       if (set_stream_position(new_position))
                                       /* go check the failure              */
         stream_check_eof(self, stream_info, errno, IntegerZero);
                                       /* set the new read position         */
       stream_info->char_read_position = ++new_position;
                                       /* and return it                     */
       retpos = stream_info->char_read_position;
     }
                                       /* moving character write position   */
     if (position_flags & operation_write) {
                                       /* check the stream bounds           */
       new_position = (((position_offset - from_start) * (signed)backward) +
           (--stream_info->char_write_position * from_current) + (stream_size(stream_info) * from_end));
                                       /* try to move to the new position   */
       if (set_stream_position(new_position))
                                       /* go check the failure              */
         stream_check_eof(self, stream_info, errno, IntegerZero);
                                       /* set the new read position         */
       stream_info->char_write_position = ++new_position;
                                       /* and return it                     */
       retpos = stream_info->char_write_position;
     }

     return retpos;
   }
   if (stream_info->flags.binary) {    /* moving binary lines?              */
                                       /* read positioning?                 */
     if (position_flags & operation_read) {
                                       /* calculate the new position        */
       stream_info->char_read_position = (stream_info->stream_reclength * (position_offset - from_start) * (signed)backward) +
           (stream_info->char_read_position * from_current) + ((stream_size(stream_info) + 1) * from_end);
                                       /* try to move to the new position   */
       if (from_start)
       {
         stream_info->char_read_position++;
       }
       if (set_stream_position(stream_info->char_read_position - 1))
                                       /* go check the failure              */
         stream_check_eof(self, stream_info, errno, IntegerZero);
                                       /* return the line position          */
       retpos = (stream_info->char_read_position/stream_info->stream_reclength) +
           (stream_info->char_read_position % stream_info->stream_reclength ? 1 : 0);
     }
                                       /* moving the write position         */
     if (position_flags & operation_write) {
                                       /* calculate the new position        */
       stream_info->char_write_position = (stream_info->stream_reclength * (position_offset - from_start) * (signed)backward) +
           (stream_info->char_write_position * from_current) + ((stream_size(stream_info) + 1) * from_end);
                                       /* try to move to the new position   */
       if (from_start)
       {
         stream_info->char_write_position++;
       }
       if (set_stream_position(stream_info->char_write_position - 1))
                                       /* go check the failure              */
         stream_check_eof(self, stream_info, errno, IntegerZero);
                                       /* return the line position          */
       retpos = (stream_info->char_write_position/stream_info->stream_reclength) +
           (stream_info->char_write_position % stream_info->stream_reclength ? 1 : 0);
     }
   }
   else {                              /* moving a non-binary stream        */
                                       /* if from the beginning it is not   */
                                       /* an offset but an absolute         */
                                       /* position, so only position to that*/
                                       /* line                              */
     if (from_start)
       position_offset--;              /* reduce the offset 1               */
                                       /* read positioning?                 */
     if (position_flags & operation_read) {
                                       /* if positioning is forward, do line*/
       if (forward) {                  /* reads until the line is reached   */

                                       /* if positioning is forward from    */
                                       /* start, do line reads from the     */
                                       /* beginning of the stream until the */
         if (from_start) {             /* line is reached                   */
                                       /* set the position to the beginning */
           stream_info->line_read_char_position = 1;
           stream_info->line_read_position = 1;
         }
                                       /* now go read forward the proper    */
                                       /* number of lines                   */
         result = read_forward_by_line(self, stream_info, &position_offset, &stream_info->line_read_position, &stream_info->line_read_char_position);
       }
       else {
                                       /* if positioning from end, read     */
                                       /* rlines backward from end until    */
                                       /* line is reached                   */
         if (from_end)
           result = read_from_end_by_line(self, stream_info, &position_offset,
               &stream_info->line_read_position, &stream_info->line_read_char_position);
         else
                                       /*  positioning is from current      */
                                       /* backward--read lines backward     */
                                       /* from current until line reached   */
         result = read_backward_by_line(self, stream_info, &position_offset,
             &stream_info->line_read_position, &stream_info->line_read_char_position);
       }
                                       /* fix up the character read position*/
       stream_info->char_read_position = stream_info->line_read_char_position;
       retpos = result;                /* return the read result            */
     }
                                       /* moving the write position         */
     if (position_flags & operation_write) {
                                       /* if positioning is forward, do line*/
       if (forward) {                  /* reads until the line is reached   */

                                       /* if positioning is forward from    */
                                       /* start, do line reads from the     */
                                       /* beginning of the stream until the */
         if (from_start) {             /* line is reached                   */
                                       /* set the position to the beginning */
           stream_info->line_write_char_position = 1;
           stream_info->line_write_position = 1;
         }
                                       /* now go read forward the proper    */
                                       /* number of lines                   */
         result = read_forward_by_line(self, stream_info, &position_offset, &stream_info->line_write_position, &stream_info->line_write_char_position);
       }
       else {
                                       /* if positioning from end, read     */
                                       /* rlines backward from end until    */
                                       /* line is reached                   */
         if (from_end)
           result = read_from_end_by_line(self, stream_info, &position_offset,
               &stream_info->line_write_position, &stream_info->line_write_char_position);
         else
                                       /*  positioning is from current      */
                                       /* backward--read lines backward     */
                                       /* from current until line reached   */
         result = read_backward_by_line(self, stream_info, &position_offset,
             &stream_info->line_write_position, &stream_info->line_write_char_position);
       }
                                       /* fix up the character read position*/
       stream_info->char_write_position = stream_info->line_write_char_position;
       retpos = result;                /* return the read result            */
     }
   }

   return retpos;
}

/********************************************************************************************/
/* stream_query_line_position                                                                  */
/*             read from the current position a requested number of lines                   */
/*             return the char position at the end of the line reads                        */
/*                   or a zero if eof reached first                                         */
/********************************************************************************************/

long stream_query_line_position(REXXOBJECT self, STREAM_INFO *stream_info, long current_position)
{
   char *buffer;                       /* file read buffer                  */
   long buffer_count;                  /* count read                        */
   long extra;                         /* extra line count                  */

   setup_read_stream(IntegerZero);     /* do additional setup               */
   extra = 0;                          /* no extra to count                 */
   if (current_position == 0)          /* at the beginning?                 */
     current_position = 1;             /* this is actually the start        */
                                       /* get a buffer                      */
   buffer = temp_buffer(current_position);
   set_stream_position(0);             /* set position to the beginning     */
                                       /* read from stream up to position   */
   buffer_count = read_stream_buffer(stream_info, FALSE, buffer, current_position);
   if (buffer_count < current_position)/* is this beyound the end?          */
     extra = 1;                        /* add in one extra line             */
   if (stream_info->error != 0)        /* have a read problem?              */
                                       /* raise a notready condition        */
     stream_error(self, stream_info, stream_info->error, IntegerZero);
                                       /* return the line count             */
   return count_stream_lines(buffer, buffer_count, line_end, line_end_size) + extra;
}

/********************************************************************************************/
/* stream_query_position                                                                    */
/********************************************************************************************/
RexxMethod3(REXXOBJECT, stream_query_position,
     OSELF, self,                      /* target stream object              */
     BUFFER, StreamBuffer,             /* stream information block          */
     CSTRING, ts)                      /* query command string              */
{

/* fields that parse will fill in */
unsigned long Parse_Fields[1] = {
  0                                    /* query_position_flags              */
 };

/* Action table for query position parameters */

ATS Query_System_Position[] = {
      {MF,   sizeof(query_position_flags), (void *)query_position_flags_index, errcode,(void *)&one,0},
      {BitOr,sizeof(query_position_flags), (void *)query_position_flags_index, errcode,(void *)&query_system_position,0},
      {0,0,0,0,0,0}
     };
ATS Query_Read_Position[] = {
      {ME,   sizeof(query_position_flags), (void *)query_position_flags_index, errcode,(void *)&query_write_position,0},
      {ME   ,sizeof(query_position_flags), (void *)query_position_flags_index, errcode,(void *)&query_system_position,0},
      {BitOr,sizeof(query_position_flags), (void *)query_position_flags_index, errcode,(void *)&query_read_position,0},
      {0,0,0,0,0,0}
     };
ATS Query_Write_Position[] = {
      {ME,   sizeof(query_position_flags), (void *)query_position_flags_index, errcode,(void *)&query_read_position,0},
      {ME   ,sizeof(query_position_flags), (void *)query_position_flags_index, errcode,(void *)&query_system_position,0},
      {BitOr,sizeof(query_position_flags), (void *)query_position_flags_index, errcode,(void *)&query_write_position,0},
      {0,0,0,0,0,0}
     };
ATS Query_Char_Position[] = {
      {ME,   sizeof(query_position_flags), (void *)query_position_flags_index, errcode,(void *)&query_line_position,0},
      {BitOr,sizeof(query_position_flags), (void *)query_position_flags_index, errcode,(void *)&query_char_position,0},
      {0,0,0,0,0,0}
     };
ATS Query_Line_Position[] = {
      {ME,   sizeof(query_position_flags), (void *)query_position_flags_index, errcode,(void *)&query_char_position,0},
      {BitOr,sizeof(query_position_flags), (void *)query_position_flags_index, errcode,(void *)&query_line_position,0},
      {0,0,0,0,0,0}
     };

/* Token table for open parameters */
TTS  tts[] = {
      {"SYS",1,   Query_System_Position,0},
      {"READ",1,  Query_Read_Position,0},
      {"WRITE",1, Query_Write_Position,0},
      {"CHAR",1,  Query_Char_Position,0},
      {"LINE",1,  Query_Line_Position,0},
      {"\0",0,NULL, unknown_tr}
    };

/* dummy parameter structure for call to parser */
DUMMY_PARMS dummy_parms;

TTS *ttsp;

   STREAM_INFO *stream_info;           /* stream information                */
   ttsp = tts;
   query_position_flags = 0;           /* clear the position_flags          */

   stream_info = get_stream_info();    /* get the stream block              */

   table_fixup(ttsp, Parse_Fields);    /* fix up the parser tables          */
   if (NO_CSTRING != ts) {             /* have parameters?                  */
                                       /* parse the command string          */
     if (parser(ttsp, ts, (void *)(&dummy_parms)) != 0)
                                       /* this is an error                  */
       send_exception(Error_Incorrect_call);
   }

   if (!stream_info->flags.open)       /* unopened stream?                  */
     return OREF_NULLSTRING;           /* this is a null string             */

   if (stream_info->flags.transient)   /* checking a transient stream?      */
     return IntegerOne;                /* always at position one            */

                                       /* querying the system position?     */
   if (query_position_flags & query_system_position)
                                       /* get directly from the stream      */
     return RexxInteger(tell_stream_position);
                                       /* no read position to query?        */
   if (stream_info->flags.write_only && !stream_info->flags.read_write)
     return IntegerZero;               /* just return zero                  */

                                       /* no method specified?              */
   if (!query_position_flags && (query_read_position | query_write_position))

     if (stream_info->flags.write_only)/* this a write-only stream?         */
                                       /* return the write position         */
       query_position_flags |= query_write_position;
                                       /* asking for the write position?    */
   if (query_position_flags & query_write_position) {
                                       /* check if line or char             */
     if (query_position_flags & query_line_position) {
       if (stream_info->flags.binary)  /* binary stream?                    */
                                       /* return calculated stream position */
         return RexxInteger((stream_info->char_write_position/stream_info->stream_reclength) +
             (stream_info->char_write_position % stream_info->stream_reclength ? 1 : 0));
       else {                          /* non-binary stream                 */
                                       /* no up-to-date line position?      */
         if (stream_info->line_write_position == 0)
                                       /* update this                       */
           stream_info->line_write_position = stream_query_line_position(self, stream_info, stream_info->char_write_position);
                                       /* set the character position        */
         stream_info->line_write_char_position = stream_info->char_write_position;
       }
                                       /* return this position              */
       return RexxInteger(stream_info->line_write_position);
     }
     else                              /* just return the write position    */
       return RexxInteger(stream_info->char_write_position);
   }
   else {                              /* return the read position          */
                                       /* check if line or char             */
     if (query_position_flags & query_line_position) {
       if (stream_info->flags.binary)  /* binary stream?                    */
                                       /* return calculated stream position */
         return RexxInteger((stream_info->char_read_position/stream_info->stream_reclength) +
             (stream_info->char_read_position % stream_info->stream_reclength ? 1 : 0));
       else {                          /* non-binary stream                 */
                                       /* no up-to-date line position?      */
         if (stream_info->line_read_position == 0)
                                       /* update this                       */
           stream_info->line_read_position = stream_query_line_position(self, stream_info, stream_info->char_read_position);
                                       /* set the character position        */
         stream_info->line_read_char_position = stream_info->char_read_position;
       }
                                       /* return this position              */
       return RexxInteger(stream_info->line_read_position);
     }
     else                              /* just return the read position     */
       return RexxInteger(stream_info->char_read_position);
   }
}

/********************************************************************************************/
/* read_backward_by_line                                                                    */
/*             read from the current position a requested number of lines                   */
/*             return error, EOF or success indication                                      */
/********************************************************************************************/

long read_backward_by_line(REXXOBJECT self, STREAM_INFO *stream_info, long *line_count, long *current_line, long *current_position)
{

   char *buffer;                       /* buffer pointer                    */
   size_t buffer_count;
   long buffer_size;                   /* size of the buffer                */
   long buffer_index;                  /* position within the buffer        */

   setup_read_stream(IntegerZero);     /* do additional setup               */
   buffer_size = *current_position;    /* get the buffer size               */
   buffer = temp_buffer(buffer_size);  /* get a buffer                      */
   set_stream_position(0);             /* set position to beginning         */
                                       /* read from stream up to the current*/
   buffer_count = read_stream_buffer(stream_info, FALSE, buffer, buffer_size);
   if (stream_info->error != 0)        /* have a read problem?              */
                                       /* raise a notready condition        */
     stream_error(self, stream_info, stream_info->error, IntegerZero);
                                       /* if char read count more than end  */
   if (buffer_count >= line_end_size) {
                                       /* count back thru lines in buffer   */
     for (buffer_index = buffer_count - 1;
           buffer_index >= 0 &&  *line_count >= 0;
          buffer_index--) {
                                       /* got a line-end?                   */
        if (!memcmp(&buffer[buffer_index-line_end_size],(const char *)line_end,line_end_size)
            || buffer[buffer_index-1] == nl){ /* or a single NL ?           */

          (*line_count)--;             /* count this                        */
          (*current_line)--;           /* and move the current line too     */
        }
     }
   }
                                       /* unable to do this?                */
   if (( buffer_index < 0) ||  ( *line_count >= 0) || ( *current_line <= 0)) {
                                       /*  set the stream to the beginning  */
     *current_line = *current_position = 1;
     set_stream_position(0);           /* move the stream position          */
     if ( *line_count > 0)             /* under-run the stream?             */
       return (0);                     /* return the eof indicator          */
     return (1);                       /* return position number 1          */
   }
                                       /* set the new current position      */
   *current_position = buffer_index + line_end_size;
   return ++(*current_line);           /* and the current line              */
}

/********************************************************************************************/
/* read_forward_by_line                                                                     */
/********************************************************************************************/

long read_forward_by_line(
  REXXOBJECT self,                     /* target stream object              */
  STREAM_INFO *stream_info,            /* current stream block              */
  long *line_count,                    /* count to move                     */
  long *current_line,                  /* current line position             */
  long *current_position)              /* current character position        */
{
   char *buffer;                       /* temp read buffer                  */
   long buffer_count;                  /* count of buffer data              */
   long buffer_size;                   /* size of the buffer                */

   setup_read_stream(IntegerZero);     /* do additional setup               */
                                       /* get the buffer size               */
   buffer_size = (stream_size(stream_info) - (*current_position - 1));
   buffer = temp_buffer(buffer_size);  /* get a buffer                      */
                                       /* set the stream position           */
   set_stream_position(*current_position - 1);
                                       /* read from current position to end */
   buffer_count = read_stream_buffer(stream_info, FALSE, buffer, buffer_size);
   if (stream_info->error != 0)        /* have a read problem?              */
                                       /* raise a notready condition        */
     stream_error(self, stream_info, stream_info->error, IntegerZero);
   *current_line += *line_count;       /* assume success                    */
                                       /* move it forward                   */
   *current_position += scan_forward_lines(buffer, buffer_count, line_count, line_end, line_end_size) - 1;
   *current_line -= *line_count;       /* adjust by incompleted lines       */
   if (*line_count)                    /* not able read all?                */
   {
     stream_info->pseudo_lines = 0;    /*       no lines left               */
     return 0;                         /* this is an end of file            */
   }
   return *current_line;               /* return current line count         */
}


/********************************************************************************************/
/* read_from_end_by_line                                                                    */
/*             read from the end of the stream a requested number of lines                  */
/*             return error, EOF or success indication                                      */
/********************************************************************************************/

long read_from_end_by_line(
  REXXOBJECT self,                     /* target stream object              */
  STREAM_INFO *stream_info,            /* current stream block              */
  long *line_count,                    /* count to move                     */
  long *current_line,                  /* current line position             */
  long *current_position)              /* current character position        */
{

   char *buffer;                       /* read buffer                       */
   long buffer_count;                  /* count of characters read          */
   long buffer_size;                   /* size of the buffer                */
   long total_lines;                   /* total number of lines             */

   setup_read_stream(IntegerZero);     /* do additional setup               */
                                       /* get the stream size               */
   buffer_size = stream_size(stream_info);
   buffer = temp_buffer(buffer_size);  /* get a buffer                      */
   set_stream_position(0);             /* set the stream position           */
                                       /* read the stream into the buffer   */
   buffer_count = read_stream_buffer(stream_info, FALSE, buffer,buffer_size);
   if (stream_info->error != 0)        /* have a read problem?              */
                                       /* raise a notready condition        */
     stream_error(self, stream_info, stream_info->error, IntegerZero);
                                       /* count the lines                   */
   total_lines = count_stream_lines(buffer, buffer_count, line_end, line_end_size);
   if (*line_count >= total_lines) {   /* too many to move?                 */
     set_stream_position(0);           /* set to the beginning              */
                                       /* everything is at line 1           */
     *current_line = *current_position = 1;
     if (*line_count > total_lines)    /* too many lines?                   */
       return (0);                     /* return the failure indicator      */
     return (1);                       /* this worked ok                    */
   }
                                       /* compute new current line          */
   *current_line = total_lines - (*line_count-1);
   *line_count = *current_line - 1;    /* get count to move forward         */
                                       /* move it forward                   */
   *current_position = scan_forward_lines(buffer, buffer_count, line_count, line_end, line_end_size);
   return *current_line;               /* return the line position          */
 }

/********************************************************************************************/
/* set_line_position                                                                        */
/*             read from the beginning of the stream counting the lines                     */
/********************************************************************************************/

int set_line_position(
  REXXOBJECT self,                     /* target stream object              */
  STREAM_INFO *stream_info)            /* current stream position           */
{

   char *buffer;                       /* read buffer                       */
   long buffer_count;                  /* count of characters read          */
                                       /* already set up?                   */
   if (stream_info->line_read_position && stream_info->line_write_position)
                                       /* return the read position          */
      return stream_info->line_read_position;
                                       /* at the beginning?                 */
   if (1 == stream_info->char_read_position) {
                                       /* move the line read counts too     */
     stream_info->line_read_position = 1;
     stream_info->line_read_char_position = 1;
                                       /* write count the same?             */
     if (1 == stream_info->char_write_position) {
                                       /* just set line write counts to same*/
       stream_info->line_write_position = 1;
       stream_info->line_write_char_position = 1;
       return 1;                       /* all setup, just return            */
     }
   }
                                       /* if we can set up the write counts */
   if (1 == stream_info->char_write_position) {
                                       /* just reset the counter            */
     stream_info->line_write_position = 1;
     stream_info->line_write_char_position = 1;
   }
   setup_read_stream(IntegerZero);     /* do additional setup               */
                                       /* calculate the buffer size         */
   buffer = temp_buffer((stream_info->char_read_position > stream_info->char_write_position) ?
                      stream_info->char_read_position : stream_info->char_write_position  + 1);
   set_stream_position(0);             /* move to the beginning             */
                                       /* read from stream up to char       */
                                       /* position                          */
                                       /* read position higher?             */
   if (stream_info->char_read_position > stream_info->char_write_position)
                                       /* use it                            */
     buffer_count = stream_info->char_read_position;
   else                                /* use the write position            */
     buffer_count = stream_info->char_write_position;
                                       /* read whats in the stream          */
   buffer_count = read_stream_buffer(stream_info, FALSE, buffer, buffer_count);
   if (stream_info->error != 0)        /* have a read problem?              */
                                       /* raise a notready condition        */
     stream_error(self, stream_info, stream_info->error, IntegerZero);
                                       /* count lines in the buffer         */
   stream_info->line_read_position = count_stream_lines(buffer, stream_info->char_read_position, line_end, line_end_size);
                                       /* update line char count to char    */
   stream_info->line_read_char_position = stream_info->char_read_position;
                                                 /* if the read and write char counts are the same */
                                                 /*  make the line counts the same                */
                                       /* positions the same?               */
   if (stream_info->char_read_position == stream_info->char_write_position) {
                                       /* make the same                     */
       stream_info->line_write_position = stream_info->line_read_position;
       stream_info->line_write_char_position = stream_info->line_read_char_position;
       return 1;                       /* and return                        */
   }
                                       /* count lines in the buffer         */
   stream_info->line_write_position = count_stream_lines(buffer, stream_info->char_write_position, line_end, line_end_size);
                                       /* update the character position     */
   stream_info->line_write_char_position = stream_info->char_write_position;
   return 1;                           /* return success                    */
}

/********************************************************************************************/
/* qualify                                                                                  */
/********************************************************************************************/
RexxMethod1(CSTRING, qualify,
     BUFFER, StreamBuffer )            /* stream information block          */
{

   STREAM_INFO *stream_info;           /* stream information                */

   stream_info = get_stream_info();    /* get the stream block              */
   if (!stream_info->flags.open)       /* not open yet?                     */
     full_name_parameter(stream_info); /* expand the full name              */
                                       /* return the name parameter         */
   return (PCHAR)&stream_info->full_name_parameter;
}

/********************************************************************************************/
/* query_exists                                                                             */
/********************************************************************************************/
RexxMethod1(CSTRING, query_exists,
     BUFFER, StreamBuffer )            /* stream information block          */
{
   STREAM_INFO *stream_info;           /* stream information                */
   struct stat stat_info;

   stream_info = get_stream_info();    /* get the stream block              */
                                       /* check if persistant and return the*/
                                       /* name. if not return null string   */
   if (get_file_statistics(stream_info, &stat_info) == 0 && !(stat_info.st_mode&S_IFDIR)) {
                                       /* opened as a stream?               */
     if (stream_info->flags.handle_stream)
                                       /* just return the name              */
       return (PCHAR)&stream_info->name_parameter;
     else                              /* give the fully expanded name      */
       return (PCHAR)&stream_info->full_name_parameter;
   }
   else
   if (!stream_info->flags.handle_stream &&
       ((strchr(stream_info->full_name_parameter,'*') != NULL) || (strchr(stream_info->full_name_parameter,'?') != NULL)))
   {
      if (SearchFirstFile(stream_info->full_name_parameter))
             return (PCHAR)&stream_info->full_name_parameter;
          else
             return "";
   }
   else
     return "";                        /* don't return anything             */
}

/********************************************************************************************/
/* query_handle                                                                             */
/********************************************************************************************/
RexxMethod1(REXXOBJECT, query_handle,
     BUFFER, StreamBuffer )            /* stream information block          */
{
   STREAM_INFO *stream_info;           /* stream information                */

   stream_info = get_stream_info();    /* get the stream block              */

   if (!stream_info->flags.open)       /* unopened stream?                  */
     return OREF_NULLSTRING;           /* this is a null string             */
   return RexxInteger(stream_info->fh);/* return the handle                 */
}

/********************************************************************************************/
/* query_streamtype                                                                         */
/********************************************************************************************/
RexxMethod1(CSTRING, query_streamtype,
     BUFFER, StreamBuffer )            /* stream information block          */
{
   STREAM_INFO *stream_info;           /* stream information                */

   stream_info = get_stream_info();    /* get the stream block              */
   if (!stream_info->flags.open)       /* not open?                         */
     return "UNKNOWN";                 /* don't know the type               */
                                       /* transient stream?                 */
   else if (stream_info->flags.transient)
     return "TRANSIENT";               /* return the transient type         */
   else
     return "PERSISTENT";              /* this is a persistent stream       */
}

/********************************************************************************************/
/* query_size                                                                               */
/********************************************************************************************/
RexxMethod1(REXXOBJECT, query_size,
     BUFFER, StreamBuffer )            /* stream information block          */
{

   STREAM_INFO *stream_info;           /* stream information                */
   struct stat stat_info;

   stream_info = get_stream_info();    /* get the stream block              */
                                       /* return the stream size            */
   return get_file_statistics(stream_info, &stat_info) ? RexxString("") : RexxInteger(stat_info.st_size);
}

/********************************************************************************************/
/* query_time                                                                               */
/********************************************************************************************/
RexxMethod1(CSTRING, query_time,
     BUFFER, StreamBuffer )            /* stream information block          */
{

   STREAM_INFO *stream_info;           /* stream information                */
   struct stat stat_info;

   stream_info = get_stream_info();    /* get the stream block              */
                                       /* return the stream time            */
   return get_file_statistics(stream_info, &stat_info) ? (char *)""  : stream_time;
}

/********************************************************************************************/
/* stream_state -- return state of the stream                                               */
/********************************************************************************************/

RexxMethod1(CSTRING, stream_state,
     BUFFER, StreamBuffer )            /* stream information block          */
{
  STREAM_INFO *stream_info;            /* stream information                */
  PCHAR        result;                 /* returned result                   */

  stream_info = get_stream_info();     /* get the stream block              */
  switch (stream_info->state) {        /* process the different states      */
    case stream_unknown_state:         /* unknown stream status             */
      result = "UNKNOWN";
      break;

    case stream_notready_state:        /* both notready and an eof condition*/
    case stream_eof_state:             /* return the same thing             */
      result = "NOTREADY";
      break;

    case stream_error_state:           /* had a stream error                */
      result = "ERROR";
      break;

    case stream_ready_state:           /* stream is ready to roll           */
      result = "READY";
      break;
  }
  return result;                       /* return the descriptor             */
}

/********************************************************************************************/
/* stream_description -- return description of the stream                                   */
/********************************************************************************************/

RexxMethod1(REXXOBJECT, stream_description,
     BUFFER, StreamBuffer )            /* stream information block          */
{
  STREAM_INFO *stream_info;            /* stream information                */
  CHAR         work[200];              /* temp buffer                       */
  PCHAR        result;                 /* result string                     */

  stream_info = get_stream_info();     /* get the stream block              */
  switch (stream_info->state) {        /* process the different states      */

    case stream_unknown_state:         /* unknown stream status             */
      result = "UNKNOWN:";
      break;

    case stream_eof_state:             /* had an end-of-file condition      */
      result = "NOTREADY:EOF";
      break;

    case stream_notready_state:        /* had some sort of notready         */
    {
        result = (PCHAR)work;            /* use the work buffer               */
        char *error = NULL;

        if (stream_info->error != 0)
        {
            error = strerror(stream_info->error);
        }

        if (error != NULL)
        {
                                             /* format the result string          */
            sprintf(work, "NOTREADY:%d %s", stream_info->error, error);
        }
        else
        {
                                             /* format the result string          */
            sprintf(work, "NOTREADY:%d", stream_info->error);

        }
        result = work;
        break;
    }

    case stream_error_state:           /* had a stream error                */
    {
        char *error = NULL;

        if (stream_info->error != 0)
        {
            error = strerror(stream_info->error);
        }

        if (error != NULL)
        {
                                             /* format the result string          */
            sprintf(work, "ERROR:%d %s", stream_info->error, error);
        }
        else
        {
                                             /* format the result string          */
            sprintf(work, "ERROR:%d", stream_info->error);

        }
        result = work;
        break;

    }

    case stream_ready_state:           /* stream is ready to roll           */
      result = "READY:";
      break;
  }
  return RexxString(result);           /* return as a string value          */
}

/********************************************************************************************/
/* stream_init                                                                              */
/********************************************************************************************/

RexxMethod1(REXXOBJECT, stream_init,
  CSTRING, name)                       /* name of the stream                */
{
   STREAM_INFO *stream_info;           /* stream information                */
   REXXOBJECT   stream_block;          /* allocated stream block            */

                                       /* get a stream block                */
   stream_block = RexxBuffer(sizeof(STREAM_INFO));
                                       /* associate with this instance      */
   RexxVarSet(c_stream_info, stream_block);
                                       /* address the block                 */
   stream_info = (STREAM_INFO *)buffer_address(stream_block);
                                       /* clear out the block               */
   memset(stream_info, 0, sizeof(STREAM_INFO));
                                       /* initialize stream info structure  */
   strncpy(stream_info->name_parameter,name,path_length+10);
   strcpy(&stream_info->name_parameter[path_length+11],"\0");
   stream_info->stream_file = NULL;
   stream_info->stream_reclength = 0;
   stream_info->pseudo_stream_size = 0;
   stream_info->pseudo_lines = 0;
   stream_info->pseudo_max_lines = 0;
   stream_info->flags.read_only = 0;
   stream_info->flags.write_only = 0;
   stream_info->flags.read_write = 0;
   stream_info->flags.append = 0;
   stream_info->flags.bstd = 0;
   stream_info->char_read_position = 1;
   stream_info->char_write_position = 1;
   stream_info->line_read_position = 1;
   stream_info->line_write_position = 1;
   stream_info->line_read_char_position = 1;
   stream_info->line_write_char_position = 1;
   stream_info->flags.nobuffer = 0;
   stream_info->flags.last_op_was_read = 1;
                                       /* set this as unknown               */
   stream_info->state = stream_unknown_state;
   return NULLOBJ;
}

/***********************************************************************************************/
/* reclength_token will check the next token for numeric and if it is will put it in reclength */
/***********************************************************************************************/

long reclength_token(TTS *ttsp, const char *TokenString, TOKENSTRUCT *tsp, void *userparms)
{
   PCHAR  end_character;               /* conversion end character          */
   LONG   result = 0;                  /* conversion result                 */
   LONG   number;                      /* converted number result           */

                                       /* get the next token in TokenString */
   if (!(result = gettoken(TokenString,tsp))) {
                                       /* convert to a long                 */
     number = strtol(tsp->string, &end_character, 10);
     if (*end_character != '\0') {     /* find a non-numeric character?     */
                                       /* next token was not numeric - but  */
                                       /* since we can default the reclength*/
                                       /* if everything else is ok back up  */
                                       /* the string and let the parser see */
                                       /* if it knows what the next token is*/
       ungettoken(TokenString,tsp);
       return 0;                       /* this worked ok                    */
     }
     else
                                       /* pass back converted number        */
       *((unsigned long *)userparms) = number;
   }
   return 0;                           /* no next token...just default      */
}

/***********************************************************************************************/
/* table_fixup                                                                                 */
/*                 will change the structure offsets in the action tables to be field address  */
/***********************************************************************************************/

void table_fixup(TTS *ttsp, unsigned long parse_fields[])
{
                                                  /* get the tokentablestruct work pointer */
   TTS *work_ttsp;

                                                 /* get the actiontablestruct work pointer */
   ATS *work_atsp;

   long i;

   unsigned long *work_parse_fields;
                                                /* loop thru the action tables in the token table */
   for (work_ttsp = ttsp;
       (*(work_ttsp->token));
       work_ttsp++ ) {

                                                 /* change output pointers from offsets to addresses */
     for (work_atsp = (ATS *)work_ttsp->ATSP, work_parse_fields = parse_fields;
          work_atsp->actions; work_atsp++, work_parse_fields = parse_fields) {
       for (i = (long)work_atsp->output; i > 0; i--, work_parse_fields++);
       work_atsp->output = work_parse_fields;
     }

   }                                         /* endfor */

}
/***********************************************************************************************/
/* unknown_offset                                                                              */
/*                 will check the next token for numeric and if it is will put it in offset    */
/***********************************************************************************************/

long unknown_offset(TTS *ttsp, const char *TokenString, TOKENSTRUCT *tsp, void *userparms)
{
                                                /* loop increment variable */
   unsigned int i = 0;
   long result = 0;

                                       /* get the next token in TokenString */
   if (!(result = gettoken(TokenString,tsp))) {
                                       /* convert string into long for later*/
     while (i < tsp->length && tsp->string[i] >= '0' && tsp->string[i] <= '9' ) {

      *(((POSITION_PARMS *)userparms)->position_offset_pointer) = (tsp->string[i] - '0') +
                                               (*(((POSITION_PARMS *)userparms)->position_offset_pointer) * 10);

       i++;
     }
     if (i == tsp->length) {
                                       /* set the position was specified    */
                                       /*flag and return                    */
        *(((POSITION_PARMS *)userparms)->position_flags_pointer) |= position_offset_specified;
        return result;
     }
     else
        return errcode;                /* have an error here - non-numeric  */
   }
                                       /* no next token - position will     */
                                       /* raise syntax                      */
   return errcode;
}

/********************************************************************************************/
/* unknown_tr will be called if a token is passed to open that is not in the token table    */
/********************************************************************************************/
long unknown_tr(TTS *ttsp, const char *TokenString, TOKENSTRUCT *tsp, void *userparms)
{
   return errcode;
}

