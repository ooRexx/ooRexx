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
/* REXX Kernel                                                  stream.h      */
/*                                                                            */
/* Stream declarations and includes                                           */
/*                                                                            */
/******************************************************************************/
#ifndef STREAM_H_INCLUDED
#define STREAM_H_INCLUDED

#if defined(AIX) || defined(LINUX)

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <fcntl.h>                     /* needed for open()                 */
#include <strings.h>
#include <unistd.h>
#include <dirent.h>

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#ifdef AIX
#include <sys/io.h>
#endif
                                       /* map _sopen to open                */
#define _sopen(n, o, s, m) open(n, o, 0644)
                                       /* map stricmp to strcasecmp         */
#define stricmp(s1, s2) strcasecmp(s1, s2)

#else
#include <direct.h>
#include <io.h>
#include <share.h>
#endif   //AIX or LINUX

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>

#define stdin_handle 0
#define stdout_handle 1
#define stderr_handle 2

#if defined(AIX) || defined(LINUX)
#define SH_DENYRW     0x10
#define SH_DENYWR     0x20
#define SH_DENYRD     0x30
#define SH_DENYNO     0x40
#endif

#if defined(PATH_MAX)
# define path_length PATH_MAX + 1
#elif defined(_POSIX_PATH_MAX)
# define path_length _POSIX_PATH_MAX + 1
#elif defined(_POSIX_PATH_MAX)
# define path_length _MAX_PATH + 1
#else
# define path_length 256
#endif

#if defined(FILENAME_MAX)
# define name_length FILENAME_MAX + 1
#elif defined(_MAX_FNAME)
# define name_length _MAX_FNAME + 1
#else
# define name_length 256
#endif

#define name_parameter_length path_length+name_length

#define carriage_return "\r\r\n"
#define carriage_return_size 3

#define nbt_line_end "\n"
#define nbt_line_end_size 1

#define std_line_end "\n"
#define std_line_end_size 1

#define ctrl_z 0x1a                    /* end-of-file marker                */
#define nl     '\n'                    /* new line character                */
#define cr     '\r'                    /* carriage return character         */
#define default_buffer_size 128        /* default size of saved buffer      */

#define set_nobuffer setvbuf(stream_info->stream_file,NULL,_IONBF,0)

#define get_stat(p,s) SysStat(p,s)                       /* uses full path, and stat buffer */

#define get_fstat(h,s) fstat(h,s)                       /* uses file handle and stat buffer */

#define _transient (SysFileIsDevice(stream_info->fh) || (ftell(stream_info->stream_file) < 0) || SysFileIsPipe(stream_info) ? 1 : 0)

#define current_drive ('A'+(_getdrive()-1))

#define current_path(p,l) getcwd(p,l)                   /* uses path buffer and length */

                                            /* open needs oflag pmode and for fdopen the type */
#define open_the_stream(o,p,t) openStream(stream_info,o,p,t,SH_DENYRW)
                                            /* open needs oflag pmode and for fdopen the type */
#define open_the_stream_shared(o,p,t,s) openStream(stream_info,o,p,t,s)

#define open_stream_by_handle(mode) (stream_info->stream_file = fdopen(stream_info->fh,mode))

#define close_the_stream fclose(stream_info->stream_file)

#define buffer_flush  fflush(stream_info->stream_file)

#define stream_time ctime(&stat_info.st_mtime)

#define set_stream_position(o) fseek(stream_info->stream_file,o,0)

#define update_stream_size(s) stream_info->pseudo_stream_size = \
                            (s > stream_info->pseudo_stream_size) ? s - 1 : stream_info->pseudo_stream_size

//#define tell_stream_position ftell(stream_info->stream_file)
#define tell_stream_position SysTellPosition(stream_info)

//#define line_write(b,c) fwrite(b,1,c,stream_info->stream_file)

#define checked_line_write(b,c) line_write_check(b, c, stream_info->stream_file)

#define non_binary_read(b,n) fgets(b, n,stream_info->stream_file)

#define binary_read(b,n) fread(b, 1, n,stream_info->stream_file)

/*****************************************************************************/
/* macros for stream operations                                              */
/*****************************************************************************/
                                        /* reset line position is used by the nbp position routines */
#define reset_line_position { \
                             stream_info->line_read_position = 0; \
                             stream_info->line_write_position = 0; \
                             stream_info->line_read_char_position = 0; \
                             stream_info->line_write_char_position = 0; \
                            }

/*****************************************************************************/
/* structures for parser exits                                               */
/*****************************************************************************/

typedef struct {
  long *position_flags_pointer;
  long *position_offset_pointer;
 } POSITION_PARMS;

/* dummy parameter structure for call to parser */
typedef struct {
  long *dummy_pointer;
 } DUMMY_PARMS;

#define stream_unknown_state  0        /* stream is in unknown state        */
#define stream_ready_state    1        /* stream is in ready state          */
#define stream_notready_state 2        /* stream is in a notready condition */
#define stream_eof_state      3        /* stream is in an eof condition     */
#define stream_error_state    4        /* stream has had an error condition */

/*****************************************************************************/
/* Stream structure to hold information pertinent to all streams             */
/*****************************************************************************/

typedef struct Stream_Info {
                                       /* specified stream name             */
   char name_parameter[name_parameter_length];
                                       /* fully resolved stream name        */
   char full_name_parameter[name_parameter_length];
   long char_read_position;            /* current character position        */
   long char_write_position;           /* current write position            */
   long line_read_position;            /* current read line number          */
   long line_write_position;           /* current write line number         */
   long line_read_char_position;       /* current line read position        */
   long line_write_char_position;      /* current line write position       */
   long pseudo_stream_size;            /* emulated stream size (lines)      */
   long pseudo_lines;                  /* emulated lines() value            */
   long pseudo_max_lines;              /* emulated max lines() value        */
   FILE *stream_file;                  /* file information                  */
   int fh;                             /* stream file handle                */
   int state;                          /* current stream state              */
   int error;                          /* error information                 */
   long stream_reclength;              /* binary file record length         */
   char *bufferAddress;                /* current read buffer               */
   long  bufferLength;                 /* current read buffer size          */
   struct {
      unsigned read_only :1;           /* if read only specified            */
      unsigned write_only :1;          /* if write only specified           */
      unsigned read_write :1;          /* if read and write stream          */
      unsigned append   :1;            /* if append was specified           */
      unsigned nobuffer :1;            /* if nobuffer on open specified     */
      unsigned bstd :1;                /* if standard stream specified      */
      unsigned last_op_was_read :1;    /* keep track of the last operation  */
      unsigned handle_stream :1;       /* opened as a handle stream         */
      unsigned transient :1;           /* have a transient stream           */
      unsigned binary :1;              /* stream is opened as a binary      */
      unsigned open :1;                /* stream is open                    */
   } flags;
} STREAM_INFO;


#ifndef SysQualifyStreamName
                                       /* Qualify a stream name             */
void SysQualifyStreamName(STREAM_INFO *);
#endif

#ifndef SearchFirstFile
BOOL SearchFirstFile(PCHAR name);
#endif

#ifndef SysBinaryFilemode
FILE * SysBinaryFilemode(FILE *, BOOL);
#endif

#ifndef SysFileIsDevice
BOOL SysFileIsDevice(int fhandle);
#endif

#ifndef SysPeekKeyboard
int SysPeekKeyboard(void);
#endif

#if defined(AIX) || defined(LINUX)
#ifndef SysPeekSTDIN
int SysPeekSTDIN(STREAM_INFO *);
#endif
#endif

#if defined(AIX) || defined(LINUX)
#ifndef SysPeekSTD
int SysPeekSTD(STREAM_INFO *);
#endif
#endif

#ifndef SysStat
INT SysStat(char *, struct stat *);
#endif

#ifndef SysFileIsPipe
BOOL SysFileIsPipe(STREAM_INFO *);
#endif

#ifndef SysTellPosition
LONG SysTellPosition(STREAM_INFO *);
#endif

#endif
