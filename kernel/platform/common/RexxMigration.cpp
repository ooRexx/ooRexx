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
/* REXX Kernel                                                       REXXRT.C */
/*                                                                            */
/* convert a tokenized file from pre-v2.0.0 to tokenized >= v2.0.0            */
/*                                                                            */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "RexxCore.h"
#include "ExpressionFunction.hpp"

#define  NEWMETAVERSION       31       /* 09/18/00 bump for the change to hashvalues    */
                                       /* note: this assumes that rexx will have a new  */
                                       /* metaversion of 31 on all platforms affected   */
                                       /* by the change in the hash algorithm           */
#define  MAGIC          11111

// OS2 FILESTATUS Stuctures duplicated for win32
typedef struct _FDATE {
   unsigned short day:5;
   unsigned short month:4;
   unsigned short year:7;
} FDATE;

typedef struct _FTIME{
   unsigned short twosecs:5;
   unsigned short minutes:6;
   unsigned short hours:5;
} FTIME;


#ifdef WIN32

typedef struct _FILESTATUS{
   FDATE fdataCreation;
   FTIME ftimeCreation;
   FDATE fdateLastAccess;
   FTIME ftimeLastAccess;
   FDATE fdateLastWrite;
   FTIME ftimeLastWrite;
   unsigned long cbFile;
   unsigned long cbFileAlloc;
   unsigned short attrFile;
} FILESTATUS;

typedef struct _control {              /* meta date control info            */
    unsigned short Magic;              /* identifies as 'meta' prog         */
    unsigned short MetaVersion;        /* version of the meta prog          */
             char  RexxVersion[40];    /* version of rexx interpreter       */
    FILESTATUS FileStatus;             /* file information                  */
    long     ImageSize;                /* size of the method info           */
} FILE_CONTROL;                        /* saved control info                */

typedef struct _CONVERSION {
  char from[8];
  char to[8];
  unsigned short from_meta;
  unsigned short to_meta;
  // check and change meta number
  int (__stdcall *checkAndChangeMetaNumber)(FILE_CONTROL*);
  // fix hash
  void (__stdcall *fixHashValue)(RexxString*);
  // check BIF
  int (__stdcall *checkBIF)(RexxExpressionFunction*);
  // fix object numbers
  unsigned int (__stdcall *fixObjectNumber)(RexxObject*);
  char to_date[40];
} CONVERSION;


int __stdcall to21Meta(FILE_CONTROL*);
int __stdcall to13Meta(FILE_CONTROL*);
int __stdcall toNewMeta(FILE_CONTROL*);

void __stdcall createNewHash(RexxString*);
void __stdcall createOldHash(RexxString*);

int __stdcall checkUSERbif(RexxExpressionFunction*);

unsigned int __stdcall to212ObjNums(RexxObject*);
unsigned int __stdcall to211ObjNums(RexxObject*);

CONVERSION possibilities[] = {
  { "1.0.3", "2.1", (unsigned short) 29, (unsigned short) 31, to21Meta, createNewHash, NULL, NULL, "WINDOWS  OBJREXX 6.00 20 Feb 2001" },
  { "2.1", "1.0.3", (unsigned short) 31, (unsigned short) 29, to13Meta, createOldHash, NULL, NULL, "WINDOWS  OBJREXX 6.00 18 May 1999"},
  { "2.1", "2.1.1", (unsigned short) 31, (unsigned short) 31, toNewMeta, NULL, NULL, NULL, "WINDOWS  OBJREXX 6.00 24 Apr 2002" },
  { "2.1.1", "2.1.2", (unsigned short) 31, (unsigned short) 34, toNewMeta, NULL, NULL, to212ObjNums,"WINDOWS  OBJREXX 6.00 2 Dec 2002"  },
  { "2.1.2", "2.1.1", (unsigned short) 34, (unsigned short) 31, toNewMeta, NULL, checkUSERbif, to211ObjNums, "WINDOWS  OBJREXX 6.00 24 Apr 2002" },
  { "2.1.2", "2.1.3", (unsigned short) 34, (unsigned short) 34, toNewMeta, NULL, NULL, NULL, "WINDOWS  OBJREXX 6.00 31 Mar 2004" },
  { "2.1.3", "2.1.2", (unsigned short) 34, (unsigned short) 34, toNewMeta, NULL, NULL, NULL, "WINDOWS  OBJREXX 6.00 2 Dec 2002" }
};

#else
  // other platforms:   AIX
typedef struct _control {              /* meta date control info            */
    unsigned short Magic;              /* identifies as 'meta' prog         */
    unsigned short MetaVersion;        /* version of the meta prog          */
             char  RexxVersion[40];    /* version of rexx interpreter       */
    long     ImageSize;                /* size of the method info           */
} FILE_CONTROL;                        /* saved control info                */


typedef struct _CONVERSION {
  char from[8];
  char to[8];
  unsigned short from_meta;
  unsigned short to_meta;
  // check and change meta number
  int (*checkAndChangeMetaNumber)(FILE_CONTROL*);
  // fix hash
  void (*fixHashValue)(RexxString*);
  // check BIF
  int (*checkBIF)(RexxExpressionFunction*);
  // fix object numbers
  unsigned int (*fixObjectNumber)(RexxObject*);
} CONVERSION;

int  to112Meta(FILE_CONTROL*);
int  to119Meta(FILE_CONTROL*);
int  to118Meta(FILE_CONTROL*);
int  to1110Meta(FILE_CONTROL*);

int checkUSERbif(RexxExpressionFunction*);

unsigned int  to118_ObjNums(RexxObject*);
unsigned int  to1110_ObjNums(RexxObject*);

CONVERSION possibilities[] = {
  { "1.1.2", "1.1.10", (unsigned short) 22, (unsigned short) 34, to1110Meta, NULL, NULL, to1110_ObjNums },
  { "1.1.7", "1.1.10", (unsigned short) 33, (unsigned short) 34, to1110Meta, NULL, NULL, to1110_ObjNums },
  { "1.1.8", "1.1.10", (unsigned short) 33, (unsigned short) 34, to1110Meta, NULL, NULL, to1110_ObjNums },
//{ "1.1.9", "1.1.8",  (unsigned short) 34, (unsigned short) 33, to118Meta,  NULL, checkUSERbif, to118_ObjNums },
  { "1.1.9", "1.1.10", (unsigned short) 34, (unsigned short) 34, to1110Meta, NULL, NULL, NULL }
 };
#endif

#define possible_conversions sizeof(possibilities)/sizeof(CONVERSION)

                                       /* size of control structure         */
#define  CONTROLSZ      sizeof(FILE_CONTROL)

#define  compiledHeader "/**/@REXX"

enum { ERROR_OK = 0, ERROR_NOT_TOKENIZED, ERROR_CORRUPTED, ERROR_WARNING_EXTRABYTES,
       ERROR_VERSION_CONFLICT, ERROR_NOT_COMPATIBLE };

int target = 0;  // which entry in possibilities[] to use
char *version_info; // only valid as long as the buffer is alive

#ifdef WIN32
/****************************************************************************/
/* tokenized format fixing routines for windows                             */
/****************************************************************************/

int __stdcall to21Meta(FILE_CONTROL *fileControl)
{
  /* does the magic number fit, is the tokenizer version "old enough"? */
  if ( (fileControl->Magic != MAGIC) || (fileControl->MetaVersion > possibilities[target].from_meta) ) {
    return ERROR_VERSION_CONFLICT;
  }
  /* set new meta version for this file */
  fileControl->MetaVersion = possibilities[target].to_meta;

  /* set new version date for this file */
  memset(fileControl->RexxVersion,0,40);
  memcpy(fileControl->RexxVersion, possibilities[target].to_date,40);

  return ERROR_OK;
}

int __stdcall to13Meta(FILE_CONTROL *fileControl)
{
  /* does the magic number fit, is the tokenizer version "new enough"? */
  if ( (fileControl->Magic != MAGIC) || (fileControl->MetaVersion != possibilities[target].from_meta) ) {
    return ERROR_VERSION_CONFLICT;
  }
  /* see that this is not a V2.1.1 tokenized format (the meta number was
     unfortunately not bumped from V2.1 to V2.1.1, so this is necessary */
  if (memcmp("WINDOWS  OBJREXX 6.00 20 Feb 2001", fileControl->RexxVersion, 33) != 0) {
    return ERROR_VERSION_CONFLICT;
  }
  /* set new meta version for this file */
  fileControl->MetaVersion = possibilities[target].to_meta;

  /* set new version date for this file */
  memset(fileControl->RexxVersion,0,40);
  memcpy(fileControl->RexxVersion, possibilities[target].to_date,40);

  return ERROR_OK;
}

int __stdcall toNewMeta(FILE_CONTROL *fileControl)
{
  /* does the magic number fit, is the tokenizer version correct? */
  if ( (fileControl->Magic != MAGIC) || (fileControl->MetaVersion != possibilities[target].from_meta) ) {
    return ERROR_VERSION_CONFLICT;
  }
  /* set new meta version for this file */
  fileControl->MetaVersion = possibilities[target].to_meta;

  /* set new version date for this file */
  memset(fileControl->RexxVersion,0,40);
  memcpy(fileControl->RexxVersion, possibilities[target].to_date,40);

  return ERROR_OK;
}

void __stdcall createNewHash(RexxString *candidate)
{
  const char *stringData = candidate->getStringData();
  int stringLength = candidate->getLength();
  unsigned int oldHash;
  unsigned int newHash;

  /* first caluclate the old hash value to see if this is set */
  if (stringLength == 0) oldHash = newHash = 1;
  else {
    if (stringLength >= sizeof(long))
      oldHash = *(unsigned long*) stringData + stringLength;
    else
      oldHash = *(unsigned short*) stringData + stringLength;
    newHash = oldHash + stringData[stringLength-1]; /* the new hash just adds the last character to the old hash */
  }

  candidate->hashvalue = newHash;
}

void __stdcall createOldHash(RexxString *candidate)
{
  const char *stringData = candidate->getStringData();
  int stringLength = candidate->getLength();
  unsigned int oldHash;
  unsigned int newHash;

  /* caluclate the old hash value */
  if (stringLength == 0) oldHash = 1;
  else {
    if (stringLength >= sizeof(long))
      oldHash = *(unsigned long*) stringData + stringLength;
    else
      oldHash = *(unsigned short*) stringData + stringLength;
    newHash = oldHash;
  }
  candidate->hashvalue = newHash;
}

int __stdcall checkUSERbif(RexxExpressionFunction *function)
{
  // is target pre-v2.1.1?
  if (possibilities[target].to_meta < 34) {
    // is this a BIF not implemented in the previous version?
    if (function->builtin_index > 82) {
      fprintf(stderr,"V%s does not support the USERID built-in function used in the original format.\n", possibilities[target].to);
      return ERROR_NOT_COMPATIBLE;
    }
  }
  return ERROR_OK;
}


/*
   the introduction of the MutableBuffer moves the object indices for the
   table: With V2.1.2, the mutable buffer and mutable buffer class were
   inserted after T_memory, causing a shift of all objects higher than
   highest_exposed_T.
   This must be fixed between the V2.1.1 and V2.1.2 tokenized versions.

   V2.1.2                                      V2.1.1
   T_memory                                    T_memory (highest_exposed_T)
   T_mutable_buffer                            T_intstack
   T_mutable_buffer_class (highest_exposed_T)  ...
   T_intstack
   ...

   These two functions must always return the valid object number for
   v2.1.2 since the defines for T_<objname> are from the current RexxCore.h!!!
*/
unsigned int __stdcall to212ObjNums(RexxObject *obj)
{
  unsigned int objNum = (unsigned int) obj->behaviour;
  if (objNum > T_memory && objNum <= highest_T) {
    objNum += 2;  // skip the mutable buffer entries
    obj->behaviour = (RexxBehaviour*) objNum;
  }
  return objNum;
}

unsigned int __stdcall to211ObjNums(RexxObject *obj)
{
  unsigned int objNum = (unsigned int) obj->behaviour;
  if (objNum > T_memory && objNum <= highest_T) {
    if (objNum == T_mutablebuffer ||
      objNum == T_mutablebuffer_class) {
      fprintf(stderr, "The MutableBuffer class is not supported for V2.1.1!\n");
      exit(-1);
    }
    obj->behaviour = (RexxBehaviour*) (objNum-2); // skip the mutable buffer entries
  }
  return objNum;
}
#else
/****************************************************************************/
/* tokenized format fixing routines for AIX                                 */
/****************************************************************************/

int to1110Meta(FILE_CONTROL *fileControl)
{
  /* does the magic number fit, is the tokenizer version correct? */
  if ( (fileControl->Magic != MAGIC) || (fileControl->MetaVersion != possibilities[target].from_meta) ) {
    return ERROR_VERSION_CONFLICT;
  }
  /* set new meta version for this file */
  fileControl->MetaVersion = possibilities[target].to_meta;
  memcpy(fileControl->RexxVersion, "AIX 1.1.10", 10);

  return ERROR_OK;
}

unsigned int to1110_ObjNums(RexxObject *obj)
{
  unsigned int objNum = (unsigned int) obj->behaviour;
  if (objNum > T_memory && objNum <= highest_T) {
    objNum += 2;  // skip the mutable buffer entries
    obj->behaviour = (RexxBehaviour*) objNum;
  }
  return objNum;
}

int to119Meta(FILE_CONTROL *fileControl)
{
  /* does the magic number fit, is the tokenizer version correct? */
  if ( (fileControl->Magic != MAGIC) || (fileControl->MetaVersion != possibilities[target].from_meta) ) {
    return ERROR_VERSION_CONFLICT;
  }
  /* set new meta version for this file */
  fileControl->MetaVersion = possibilities[target].to_meta;
  memcpy(fileControl->RexxVersion, "AIX 1.1.9 ", 10);

  return ERROR_OK;
}

int  to112Meta(FILE_CONTROL *fileControl)
{
  /* does the magic number fit, is the tokenizer version "new enough"? */
  if ( (fileControl->Magic != MAGIC) || (fileControl->MetaVersion != possibilities[target].from_meta) ) {
    return ERROR_VERSION_CONFLICT;
  }
  if (memcmp("AIX 1.1.9 ", fileControl->RexxVersion, 10) != 0) {
    return ERROR_VERSION_CONFLICT;
  }
  /* set new meta version for this file */
  fileControl->MetaVersion = possibilities[target].to_meta;
  return ERROR_OK;
}

int  to118Meta(FILE_CONTROL *fileControl)
{
  /* does the magic number fit, is the tokenizer version "new enough"? */
  if ( (fileControl->Magic != MAGIC) || (fileControl->MetaVersion != possibilities[target].from_meta) ) {
    return ERROR_VERSION_CONFLICT;
  }
  /* set new meta version for this file */
  fileControl->MetaVersion = possibilities[target].to_meta;
  memcpy(fileControl->RexxVersion, "AIX 1.1.8 ", 10);
  return ERROR_OK;
}

unsigned int to118_ObjNums(RexxObject *obj)
{
  unsigned int objNum = (unsigned int) obj->behaviour;
  if (objNum > T_memory && objNum <= highest_T) {
    if (objNum == T_mutablebuffer ||
      objNum == T_mutablebuffer_class) {
      fprintf(stderr, "The MutableBuffer class is not supported for V1.1.8!\n");
      exit(-1);
    }
    obj->behaviour = (RexxBehaviour*) (objNum-2); // skip the mutable buffer entries
  }
  return objNum;
}

int checkUSERbif(RexxExpressionFunction *function)
{
  // is target pre-v1.1.9?
  if (possibilities[target].to_meta < 34) {
    // is this a BIF not implemented in the previous version?
    if (function->builtin_index > 82) {
      fprintf(stderr,"V%s does not support the USERID built-in function used in the original format.\n", possibilities[target].to);
      return ERROR_NOT_COMPATIBLE;
    }
  }
  return ERROR_OK;
}

#endif  /* ============================== End of functions for AIX */

/****************************************************************************/
/* this is the main "retokenize" routine                                    */
/* first, check if the file is a in tokenized format                        */
/* then, read file control block and check magic number and version number  */
/* if successful, set new meta version to field in file control block       */
/* rest of buffer are the rexx objects, lined up one after the other,       */
/* so now walk through each object, test if it is affected by the change of */
/* hashvalues (currently only only strings, afaik), if yes, recalculate that*/
/* value                                                                    */
/****************************************************************************/
int checkAndChange(char *buffer, long length)
{
  FILE_CONTROL *fileControl = NULL;
  int rc = ERROR_OK;
  char *end;
  RexxObject *candidate;
  unsigned int objNum;

  int i=0;

  /* is this tokenized? */
  if (memcmp(buffer, compiledHeader, sizeof(compiledHeader)) != 0)
    return ERROR_NOT_TOKENIZED;
  buffer+=sizeof(compiledHeader);
  length-=sizeof(compiledHeader);
  /* are there enough remaining bytes to contain a file control structure? */
  if (length < CONTROLSZ)
    return ERROR_CORRUPTED;
  fileControl = (FILE_CONTROL*) buffer;
  version_info = (char*) fileControl->RexxVersion;

  /* fix the meta number */
  rc = possibilities[target].checkAndChangeMetaNumber(fileControl);
  if (rc != ERROR_OK) {
    return rc;
  }

  buffer+=CONTROLSZ;
  length-=CONTROLSZ;

  /* the remaining bytes should match the info in the file control block */
  if (length != fileControl->ImageSize) rc = ERROR_WARNING_EXTRABYTES;

  /* set end pointer */
  end = buffer + fileControl->ImageSize;

  /* while we're not there, process the objects */
  while (buffer < end) {
    /* cast the current position into a rexx object */
    candidate = (RexxObject*) buffer;
    objNum = (unsigned int) candidate->behaviour;
    /* for debugging:
       fprintf(stderr,"Object %d, size %d bytes, behaviour %08x, hashvalue %08x\n",++i,ObjectSize(candidate),candidate->behaviour,candidate->hashvalue);
    */
    if (possibilities[target].fixObjectNumber != NULL) {
      objNum = possibilities[target].fixObjectNumber(candidate);
    }
    /* switch according to object type */
    switch (objNum) {
      /* the hashvalue for a string needs to be recalculated */
      case T_string:
        /* need to fix the hashvalue? */
        if (possibilities[target].fixHashValue != NULL) {
          possibilities[target].fixHashValue((RexxString*) candidate);
        }
        break;
      /* all other objects don't need a recalculation */
      case T_string_class:
      case T_object:
      case T_object_class:
      case T_class:
      case T_class_class:
      case T_array:
      case T_array_class:
      case T_directory:
      case T_directory_class:
      case T_envelope:
      case T_envelope_class:
      case T_integer:
      case T_integer_class:
      case T_list:
      case T_list_class:
      case T_message:
      case T_message_class:
      case T_method:
      case T_method_class:
      case T_numberstring:
      case T_numberstring_class:
      case T_queue:
      case T_queue_class:
      case T_stem:
      case T_stem_class:
      case T_somproxy:
      case T_somproxy_class:
      case T_supplier:
      case T_supplier_class:
      case T_table:
      case T_table_class:
      case T_relation:
      case T_relation_class:
      case T_memory:
      case T_intstack:
      case T_activation:
      case T_activity:
      case T_activity_class:
      case T_behaviour:
      case T_buffer:
      case T_corral:
      case T_hashtab:
      case T_listtable:
      case T_rexxmethod:
      case T_nmethod:
      case T_nmethod_class:
      case T_nativeact:
      case T_smartbuffer:
      case T_sommethod:
      case T_stack:
      case T_variable:
      case T_vdict:
      case T_clause:
      case T_source:
      case T_token:
      case T_parse_instruction:
      case T_parse_address:
      case T_parse_assignment:
      case T_parse_block:
      case T_parse_call:
      case T_parse_command:
      case T_parse_compound:
      case T_parse_do:
      case T_parse_dot_variable:
      case T_parse_drop:
      case T_parse_else:
      case T_parse_end:
      case T_parse_endif:
      case T_parse_exit:
      case T_parse_expose:
      case T_parse_forward:
        break;
      case T_parse_function:
        /* need to check the used BIF? */
        if (possibilities[target].checkBIF != NULL) {
          rc = possibilities[target].checkBIF((RexxExpressionFunction*) candidate);
          if (rc != ERROR_OK) {
            return rc;
          }
        }
        break;
      case T_parse_guard:
      case T_parse_if:
      case T_parse_interpret:
      case T_parse_label:
      case T_parse_leave:
      case T_parse_message:
      case T_parse_message_send:
      case T_parse_nop:
      case T_parse_numeric:
      case T_parse_operator:
      case T_parse_options:
      case T_parse_otherwise:
      case T_parse_parse:
      case T_parse_procedure:
      case T_parse_queue:
      case T_parse_raise:
      case T_parse_reply:
      case T_parse_return:
      case T_parse_say:
      case T_parse_select:
      case T_parse_signal:
      case T_parse_stem:
      case T_parse_then:
      case T_parse_trace:
      case T_parse_trigger:
      case T_parse_use:
      case T_parse_variable:
      case T_parse_varref:
      case T_compound_element:
      case T_activation_frame_buffer:
      case T_parse_unary_operator:
      case T_parse_binary_operator:
        break;
      /* in the unlikely event that we hit an unknown object, report an error */
      default:
        /* warning, if this a non-primitive object we shouldn't generate an error?! */
        return (int) candidate->behaviour;
    }
    /* advance to next object */
    buffer+=ObjectSize(candidate);
    length-=ObjectSize(candidate);
  }

  /* if there are bytes remaining (unlikely, except someone fiddled with the */
  /* original tokenized file), report a warning                              */
  if (length) rc = ERROR_WARNING_EXTRABYTES;
  return rc;
}

/**********************************************************/
/* show the retokenizer usage                             */
/**********************************************************/
void show_usage(char **argv)
{
  char chOpt = OPT_CHAR;
  int i;
  fprintf(stderr, "Usage: %s", argv[0]);
  for (i = 1; i <= possible_conversions; i++) {
    fprintf(stderr, "%c%c%d", i==1?' ':'|', chOpt, i );
  }
  fprintf(stderr, " oldfile newfile\n\n");
  for (i = 0; i < possible_conversions; i++) {
    if (i == 0) {
      fprintf(stderr, "options: %c1 convert from V%s to V%s format\n",
                      chOpt, possibilities[i].from, possibilities[i].to);
    } else {
      fprintf(stderr, "         %c%d convert from V%s to V%s format\n",
                      chOpt, (i+1), possibilities[i].from, possibilities[i].to);
    }
  }
  fprintf(stderr,"\nNote: Converting to a previous version is not recommended.\n"
                 "      Instead, use an appropriate version to run the script.\n");
}


/**********************************************************/
/* main function:                                         */
/*                                                        */
/* read filenames, read input file, output converted file */
/**********************************************************/
int SysCall main(int argc, char **argv)
{
  FILE *stream;
  char *oldfilename;
  char *newfilename;
  /* char tempfilename[512]; */
  char *buffer;
  long length;
  int rc;

//  if (argc < 3 || argc > 4) {
  if (argc != 4) {
    show_usage(argv);
    exit(1);
  }

  if (*argv[1] ==  OPT_CHAR) {
#ifdef WIN32
    sscanf(argv[1], "/%d", &target);
#else
    sscanf(argv[1], "-%d", &target);
#endif
    if (target < 1 || target > possible_conversions) {
      show_usage(argv);
      exit(2);
    }
    target--;
    /* set old filename */
    oldfilename = argv[2];
    /* set new filename */
    newfilename = argv[3];
  } else {
    show_usage(argv);
    exit(3);
  }

  /* filenames identical? */
  if (!strcmp(oldfilename,newfilename)) {
//  sprintf(tempfilename,"%s.new",newfilename);
//  newfilename = tempfilename;
    fprintf(stderr,"\nOutput file name must be different from input file name.\n");
    exit(4);
  }

  /* open the old tokenized file... */
  stream = fopen(oldfilename,"rb");
  if (stream) {
    /* get size of file */
    fseek(stream,0,SEEK_END);
    length = ftell(stream);
    rewind(stream);
    /* allocate memory... */
    buffer = (char *) malloc(sizeof(char)*length);
    /* ...and read it into memory at once */
    if (buffer)
      fread(buffer,sizeof(char),length,stream);
    else {
      fprintf(stderr,"Error: not enough memory!\n");
      fclose(stream);
      exit(5);
    }
    fclose(stream);

    /* do the conversion:                                                  */
    /* checkAndChange(...) will modify the file in memory and upon success */
    /* write it back to disk.                                              */
    switch (rc = checkAndChange(buffer,length)) {
      case ERROR_WARNING_EXTRABYTES:
        fprintf(stderr,"Warning, ignoring extra bytes in file!\n");
      case ERROR_OK:
        stream = fopen(newfilename,"wb");
        if (stream) {
          fwrite(buffer,sizeof(char),length,stream);
          fclose(stream);
        } else fprintf(stderr,"Error: cannot write output file %s\n",newfilename);
        break;
      case ERROR_NOT_TOKENIZED:
        fprintf(stderr,"Input file is not in non-source format!\n");
        break;
      case ERROR_CORRUPTED:
        fprintf(stderr,"Input file is damaged!\n");
        break;
      case ERROR_VERSION_CONFLICT:
        fprintf(stderr,"Version conflict. The script cannot be converted from V%s to V%s.\n"
                       "Is the conversion possible and the source script in required format?\n"
                       "Source scripts' tokenizer was: %s\n",
                       possibilities[target].from, possibilities[target].to, version_info);
#if 0
        if (target == TO_NEW)
          fprintf(stderr,"Version conflict. The script was not processed with a previous version.\n");
        else
          fprintf(stderr,"Version conflict. The script is not in the new runtime format.\n");
#endif
        break;
      case ERROR_NOT_COMPATIBLE:
        // output is in checkUserBIF()
        break;
      default:
        fprintf(stderr,"Error: unknown object type %02x\n",rc);
        break;
    }
    /* free memory */
    free(buffer);
  } else fprintf(stderr,"Error: cannot open %s\n",oldfilename);

  return 0;
}
