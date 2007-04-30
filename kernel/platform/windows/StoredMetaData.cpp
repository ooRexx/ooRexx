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
/*********************************************************************/
/*                                                                   */
/*  Module Name:        WINMETA.C                                    */
/*                                                                   */
/* Unflatten saved methods from various sources.                     */
/*                                                                   */
/*********************************************************************/

#define  INCL_ERRORS
#define  INCL_DOSQUEUES
#define  INCL_REXXSAA
#include "RexxCore.h"
#include "StringClass.hpp"
#include "RexxBuffer.hpp"
#include "RexxSmartBuffer.hpp"
#include "MethodClass.hpp"
#include "RexxCode.hpp"
#include "RexxActivity.hpp"
#include "SourceFile.hpp"
#include SYSREXXSAA

/*********************************************************************/
/*         Definitions for use by the Meta I/O functionality         */
/*********************************************************************/

#define  METAVERSION       34
                                       /* to ::class instruction errors     */
                                       /* 29 => 31 bump for change in hash  */
                                       /* algorithm                         */
                                       /* 31 => 34 bump for new kernel and  */
                                       /* USERID BIF                        */
#define  MAGIC          11111          /* function                          */


#define  VERPRE         "WINDOWS  "    /* 8 chars + 1 blank          */
#define  LENPRE         9
                                       /* size of control structure         */
#define  CONTROLSZ      sizeof(FILE_CONTROL)

#define  compiledHeader "/**/@REXX"


// OS2 FILESTATUS Stuctures duplicated for win32
typedef struct _FDATE {
   USHORT day:5;
   USHORT month:4;
   USHORT year:7;
} FDATE;

typedef struct _FTIME{
   USHORT twosecs:5;
   USHORT minutes:6;
   USHORT hours:5;
} FTIME;


typedef struct _FILESTATUS{
   FDATE fdataCreation;
   FTIME ftimeCreation;
   FDATE fdateLastAccess;
   FTIME ftimeLastAccess;
   FDATE fdateLastWrite;
   FTIME ftimeLastWrite;
   ULONG cbFile;
   ULONG cbFileAlloc;
   USHORT attrFile;
} FILESTATUS;

typedef struct _control {              /* meta date control info            */
    USHORT   Magic;                    /* identifies as 'meta' prog         */
    USHORT   MetaVersion;              /* version of the meta prog          */
    UCHAR    RexxVersion[40];          /* version of rexx intrpreter        */
    FILESTATUS FileStatus;             /* file information                  */
    LONG     ImageSize;                /* size of the method info           */
} FILE_CONTROL;                        /* saved control info                */


typedef FILE_CONTROL *PFILE_CONTROL;   /* pointer to file info              */

extern BOOL ProcessSaveImage;
RexxMethod *SysRestoreTranslatedProgram(RexxString *, FILE *Handle);

/*********************************************************************/
/*                                                                   */
/*  Function:    SysRestoreProgram                                   */
/*                                                                   */
/*  Description: This function is used to load the meta data from a  */
/*               file into the proper interpreter variables.         */
/*                                                                   */
/*********************************************************************/

RexxMethod *SysRestoreProgram(
  RexxString *FileName )               /* name of file to process           */

{
  FILE         *Handle;                /* output file handle                */
  PCHAR         File;                  /* ASCII-Z file name                 */

  RexxMethod  * Method;                /* unflattened method                */
  FILE_CONTROL  Control;               /* control information               */
  PCHAR         StartPointer;          /* start of buffered method          */
  RexxBuffer  * Buffer;                /* Buffer to unflatten               */
  LONG          BufferSize;            /* size of the buffer                */
  ULONG         BytesRead;             /* actual bytes read                 */
  RexxCode    * Code;                  /* parent rexx method                */
  RexxSource  * Source;                /* REXX source object                */
  RexxActivity *activity;              /* the current activity              */
  PVOID         MethodInfo;
                                       /* temporary read buffer             */
  CHAR          fileTag[sizeof(compiledHeader)];

  if (ProcessSaveImage)                /* doing save image?                 */
    return OREF_NULL;                  /* never restore during image build  */

  File = FileName->stringData;         /* get the file name pointer         */

  Handle = fopen(File, "rb");          /* open the input file               */
  if (Handle == NULL)                  /* get an open error?                */
    return OREF_NULL;                  /* no restored image                 */

                                       /* see if this is a "sourceless" one */
  Method = SysRestoreTranslatedProgram(FileName, Handle);
  if (Method != OREF_NULL)             /* get a method out of this?         */
    return Method;                     /* this process is finished          */
/* this is to load the tokenized form that is eventually stored behind the script */
  Handle = fopen(File, "rb");          /* open the input file               */
  if (Handle == NULL)                  /* get an open error?                */
    return OREF_NULL;                  /* no restored image                 */

  activity = CurrentActivity;          /* save the activity                 */
  ReleaseKernelAccess(activity);       /* release the access                */
                                       /* read the first file part          */
  if (fseek(Handle, 0-sizeof(compiledHeader), SEEK_END) == 0)
     BytesRead = fread(fileTag, 1 ,sizeof(compiledHeader), Handle);
                                       /* not a compiled file?              */
  if ((BytesRead != sizeof(compiledHeader)) || (strcmp(fileTag, compiledHeader) != 0)) {
    fclose(Handle);                    /* close the file                    */
    RequestKernelAccess(activity);     /* get the lock back                 */
    return OREF_NULL;                  /* not a saved program               */
  }
                                       /* now read the control info         */
  if (fseek(Handle, 0-sizeof(compiledHeader)-sizeof(Control), SEEK_END) == 0)
     BytesRead = fread((PCHAR)&Control, 1, sizeof(Control), Handle);

                                       /* check the control info            */
  if ((BytesRead != sizeof(Control)) || (Control.MetaVersion != METAVERSION) || (Control.Magic != MAGIC)) {
    fclose(Handle);                    /* close the file                    */
                                       /* got an error here                 */
    RequestKernelAccess(activity);       /* get the lock back                 */
    report_exception1(Error_Program_unreadable_version, FileName);
  }
                                       /* read the file size                */
  BufferSize = Control.ImageSize;      /* get the method info size          */
  if (fseek(Handle, 0-sizeof(compiledHeader)-sizeof(Control)-BufferSize, SEEK_END) != 0) {
    RequestKernelAccess(activity);     /* get the lock back                 */
    fclose(Handle);                    /* close the file                    */
    return OREF_NULL;                  /* not a saved program               */
  }

  MethodInfo = GlobalAlloc(GMEM_FIXED, BufferSize);  /* allocate a temp buffer */
  if (!MethodInfo) {
    RequestKernelAccess(activity);     /* get the lock back                 */
    fclose(Handle);                    /* close the file                    */
    return OREF_NULL;                  /* not a saved program               */
  }
  /* read the tokenized program */
  BytesRead = fread(MethodInfo, 1, BufferSize, Handle);
  fclose(Handle);                      /* close the file                    */

  RequestKernelAccess(activity);       /* get the lock back                 */
  Buffer = new_buffer(BufferSize);     /* get a new buffer                  */
                                       /* position relative to the end      */
  StartPointer = ((PCHAR)Buffer + ObjectSize(Buffer)) - BufferSize;
  memcpy(StartPointer, MethodInfo, BufferSize);
  GlobalFree(MethodInfo);              /* done with the tokenize buffer     */
  save(Buffer);                        /* protect the buffer                */
                                       /* "puff" this out usable form       */
  Method = TheMethodClass->restore(Buffer, StartPointer);
  save(Method);                        /* protect the method code           */
  discard(Buffer);                     /* release the buffer protection     */
                        /* buffer need not to be holded because it is now an envelope and referenced by Method */
  Code = (RexxCode *)Method->code;     /* get the REXX code object          */
  Source = Code->u_source;             /* and now the source object         */
                                       /* switch the file name (this might  */
                                       /* be different than the name        */
  Source->setProgramName(FileName);    /* originally saved under            */
  Source->setReconnect();              /* allow source file reconnection    */
  discard_hold(Method);                /* now release the lock on this      */
  return Method;                       /* return the unflattened method     */
}

/*********************************************************************/
/*                                                                   */
/*  Function:    SysSaveProgram                                      */
/*                                                                   */
/*  Description: This function saves a flattened method to a file    */
/*                                                                   */
/*********************************************************************/
// Macro replaces this in Winrexx.h
// Not implemented for Windows because,
// unable to save a program image in Windows, no EA's

void SysSaveProgram(
  RexxString * FileName,               /* name of file to process           */
  RexxMethod * Method )                /* method to save                    */
{
  FILE          *Handle;                /* file handle                      */
  FILE_CONTROL  Control;               /* control information               */
  RexxBuffer *  MethodBuffer;          /* flattened method                  */
  RexxSmartBuffer *FlatBuffer;         /* fully flattened buffer            */
  PCHAR         BufferAddress;         /* address of flattened method data  */
  LONG          BufferLength;          /* length of the flattened method    */
  RexxString *  Version;               /* REXX version string               */
  ULONG         BytesRead;             /* actual bytes read                 */
  PCHAR         File;                  /* ASCII-Z file name                 */
  RexxActivity *activity;              /* current activity                  */
  CHAR          savetok[65];

  if (ProcessSaveImage)                /* doing save image                  */
    return;                            /* never save during image build     */

  /* don't save tokens if environment variable isn't set */
  if ((GetEnvironmentVariable("RXSAVETOKENS", savetok, 64) < 1) || (strcmp("YES",savetok) != 0))
    return;

  save(FileName);                      /* protect the file name             */
  save(Method);                        /* and the method too                */
  File = FileName->stringData;         /* get the file name pointer         */
                                       /* open the file                     */
  activity = CurrentActivity;          /* save the activity                 */


  Handle = fopen(File, "a+b");         /* open the output file  */
  if (Handle == NULL)                  /* get an open error?                */
     return;

  FlatBuffer = Method->saveMethod();   /* flatten the method                */
  save(FlatBuffer);                    /* protect the flattened one too     */
                                       /* retrieve the length of the buffer */
  BufferLength = (LONG)FlatBuffer->current;
  MethodBuffer = FlatBuffer->buffer;   /* get to the actual data buffer     */
  BufferAddress = MethodBuffer->data;  /* retrieve buffer starting address  */
                                       /* clear out the cntrol info         */
  memset((void *)&Control, 0, sizeof(Control));
                                       /* fill in version info              */
  memcpy(Control.RexxVersion, VERPRE, LENPRE);
  Version = version_number();          /* get the version string            */
  memcpy((PCHAR)Control.RexxVersion + LENPRE, Version->stringData, Version->length>(40-LENPRE)?(40-LENPRE):Version->length);
  Control.MetaVersion = METAVERSION;   /* current meta version              */
  Control.Magic = MAGIC;               /* magic signature number            */
  Control.ImageSize = BufferLength;    /* add the buffer length             */

  ReleaseKernelAccess(activity);       /* release the access                */
                                       /* write out the REXX signature      */
  BytesRead = putc(0x1a, Handle);   /* Ctrl Z */
  fwrite(BufferAddress, 1, BufferLength, Handle);

  fwrite(&Control, 1, sizeof(Control), Handle);

  fwrite(compiledHeader, 1, sizeof(compiledHeader), Handle);
                                       /* now the control info              */
                                       /* and finally the flattened method  */
  fclose(Handle);                      /* done saving                       */
  RequestKernelAccess(activity);       /* and reaquire the kernel lock      */
  discard_hold(Method);                /* release the method now            */
  discard_hold(FileName);
  discard_hold(FlatBuffer);            /* release the method now            */
}


/*********************************************************************/
/*                                                                   */
/*  Function:    SysRestoreProgramBuffer                             */
/*                                                                   */
/*  Description: This function is used to unflatten a REXX method    */
/*               from a buffer into a method.                        */
/*                                                                   */
/*********************************************************************/

RexxMethod *SysRestoreProgramBuffer(
  PRXSTRING    InBuffer,               /* pointer to stored method          */
  RexxString * Name)                   /* name associated with the program  */
{
  PFILE_CONTROL Control;               /* control information               */
  PCHAR         MethodInfo;            /* buffered flattened method         */
  PCHAR         StartPointer;          /* start of buffered information     */
  RexxBuffer  * Buffer;                /* Buffer to unflatten               */
  LONG          BufferSize;            /* size of the buffer                */
  RexxMethod  * Method;                /* unflattened method                */
  RexxCode    * Code;                  /* parent rexx method                */
  RexxSource  * Source;                /* REXX source object                */

                                       /* address the control information   */
  Control = (PFILE_CONTROL)InBuffer->strptr;
                                       /* check the control info            */
  if ((Control->MetaVersion != METAVERSION) ||
      (Control->Magic != MAGIC)) {
    return OREF_NULL;                  /* can't load it                     */
  }
  MethodInfo = InBuffer->strptr + CONTROLSZ;
                                       /* get the buffer size               */
  BufferSize = InBuffer->strlength - CONTROLSZ;
  Buffer = new_buffer(BufferSize);     /* get a new buffer                  */
                                       /* position relative to the end      */
  StartPointer = ((PCHAR)Buffer + ObjectSize(Buffer)) - BufferSize;
                                       /* fill in the buffer                */
  memcpy(StartPointer, MethodInfo, BufferSize);
  save(Buffer);                        /* protect the buffer                */
                                       /* "puff" this out usable form       */
  Method = TheMethodClass->restore(Buffer, StartPointer);
  Code = (RexxCode *)Method->code;     /* get the REXX code object          */
  Source = Code->u_source;             /* and now the source object         */
                                       /* switch the file name (this might  */
                                       /* be different than the name        */
  Source->setProgramName(Name);        /* originally saved under            */
                                       /* NOTE:  no source file reconnect   */
                                       /* is possible here                  */
  discard(Buffer);                /* release the buffer protection     */
                        /* buffer need not to be holded because it is now an envelope and referenced by Method */
  return Method;                       /* return the unflattened method     */
}
                                       /* point to the flattened method     */
/*********************************************************************/
/*                                                                   */
/*  Function:    SysRestoreInstoreProgram                            */
/*                                                                   */
/*  Description: This function is used to load the meta data from an */
/*               instorage buffer.                                   */
/*                                                                   */
/*********************************************************************/
RexxMethod *SysRestoreInstoreProgram(
  PRXSTRING    InBuffer,               /* pointer to stored method          */
  RexxString * Name)                   /* name associated with the program  */
{
  RXSTRING     ImageData;              /* actual image part of this         */

                                       /* not a compiled file?              */
  if (strcmp(InBuffer->strptr, compiledHeader) != 0)
    return OREF_NULL;                  /* not a saved program               */
                                       /* point to the image start          */
  ImageData.strptr = InBuffer->strptr + sizeof(compiledHeader);
                                       /* and adjust the length too         */
  ImageData.strlength = InBuffer->strlength - sizeof(compiledHeader);
                                       /* now go unflatten this             */
  return SysRestoreProgramBuffer(&ImageData, Name);
}

/*********************************************************************/
/*                                                                   */
/*  Function:    SysSaveProgramBuffer                                */
/*                                                                   */
/*  Description: This function saves a flattened method to an        */
/*               RXSTRING buffer                                     */
/*                                                                   */
/*********************************************************************/
void SysSaveProgramBuffer(
  PRXSTRING    OutBuffer,              /* location to save the program      */
  RexxMethod * Method )                /* method to save                    */
{
  PFILE_CONTROL Control;               /* control information               */
  PCHAR         Buffer;                /* buffer pointer                    */
  RexxBuffer  * MethodBuffer;          /* flattened method                  */
  RexxSmartBuffer *FlatBuffer;         /* flattened smart buffer            */
  PCHAR         BufferAddress;         /* address of flattened method data  */
  LONG          BufferLength;          /* length of the flattened method    */
  RexxString  * Version;               /* REXX version string               */

  save(Method);                        /* lock the method down              */
  FlatBuffer = Method->saveMethod();   /* flatten the method                */
                                       /* retrieve the length of the buffer */
  BufferLength = (LONG)FlatBuffer->current;
  MethodBuffer = FlatBuffer->buffer;   /* get to the actual data buffer     */
  BufferAddress = MethodBuffer->data;  /* retrieve buffer starting address  */
                                       /* get the final buffer              */
  Buffer = (char *)SysAllocateResultMemory(BufferLength + CONTROLSZ);
  discard_hold(Method);                /* release the collection lock       */
  OutBuffer->strptr = Buffer;          /* fill in the result pointer        */
                                       /* and the result length             */
  OutBuffer->strlength = BufferLength + CONTROLSZ;
  Control = (PFILE_CONTROL)Buffer;     /* set pointer to control info       */
                                       /* fill in version info              */
  memcpy(Control->RexxVersion, VERPRE, LENPRE);
  Version = version_number();          /* get the version string            */
                                       /* copy in the version string        */
  memcpy((Control->RexxVersion) + LENPRE, Version->stringData, Version->length + 1);

  Control->MetaVersion = METAVERSION;  /* current meta version              */
  Control->Magic = MAGIC;              /* magic signature number            */
  Control->ImageSize = BufferLength;   /* save the buffer size              */
  Buffer = Buffer + CONTROLSZ;         /* step the buffer pointer           */
                                       /* Copy the method buffer            */
  memcpy(Buffer, BufferAddress, BufferLength);
}

/*********************************************************************/
/*                                                                   */
/*  Function:    SysSaveTranslatedProgram                            */
/*                                                                   */
/*  Description: This function saves a flattened method to a file    */
/*                                                                   */
/*********************************************************************/

// retrofit by IH
void SysSaveTranslatedProgram(
  PCHAR        File,                   /* name of file to process           */
  RexxMethod * Method )                /* method to save                    */
{
  FILE         *Handle;                /* output file handle                */
  FILE_CONTROL  Control;               /* control information               */
  RexxBuffer *  MethodBuffer;          /* flattened method                  */
  RexxSmartBuffer *FlatBuffer;         /* flattened smart buffer            */
  PCHAR         BufferAddress;         /* address of flattened method data  */
  LONG          BufferLength;          /* length of the flattened method    */
  RexxString  * Version;               /* REXX version string               */
  RexxActivity *activity;              /* the current activity              */

  Handle = fopen(File, "wb");          /* open the output file              */
  if (Handle == NULL)                  /* get an open error?                */
                                       /* got an error here                 */
    report_exception1(Error_Program_unreadable_output_error, new_cstring(File));
  save(Method);                        /* and the method too                */
  FlatBuffer = Method->saveMethod();   /* flatten the method                */
  save(FlatBuffer);                    /* protect the flattened one too     */
                                       /* retrieve the length of the buffer */
  BufferLength = (LONG)FlatBuffer->current;
  MethodBuffer = FlatBuffer->buffer;   /* get to the actual data buffer     */
  BufferAddress = MethodBuffer->data;  /* retrieve buffer starting address  */
                                       /* clear out the cntrol info         */
  memset((void *)&Control, 0, sizeof(Control));
                                       /* fill in version info              */
  memcpy(Control.RexxVersion, VERPRE, LENPRE);
  Version = version_number();          /* get the version string            */
  strcpy((PCHAR)Control.RexxVersion + LENPRE, Version->stringData);
  Control.MetaVersion = METAVERSION;   /* current meta version              */
  Control.Magic = MAGIC;               /* magic signature number            */
  Control.ImageSize = BufferLength;    /* add the buffer length             */

  activity = CurrentActivity;          /* save the activity                 */
  ReleaseKernelAccess(activity);       /* release the access                */
                                       /* write out the REXX signature      */
  fwrite(compiledHeader, 1, sizeof(compiledHeader), Handle);
                                       /* now the control info              */
  fwrite(&Control, 1, sizeof(Control), Handle);
                                       /* and finally the flattened method  */
  fwrite(BufferAddress, 1, BufferLength, Handle);
  fclose(Handle);                      /* done saving                       */
  RequestKernelAccess(activity);       /* and reaquire the kernel lock      */
  discard_hold(Method);                /* release the method now            */
  discard_hold(FlatBuffer);            /* release the method now            */
}
/*********************************************************************/
/*                                                                   */
/*  Function:    SysRestoreTranslatedProgram                         */
/*                                                                   */
/*  Description: This function is used to load a flattened method    */
/*               from a file into the proper interpreter variables.  */
/*                                                                   */
/*********************************************************************/

RexxMethod *SysRestoreTranslatedProgram(
  RexxString *FileName,                /* name of file to process           */
  FILE       *Handle )                 /* handle of the file to process     */
{
  FILE_CONTROL  Control;               /* control information               */
  PCHAR         StartPointer;          /* start of buffered method          */
  RexxBuffer  * Buffer;                /* Buffer to unflatten               */
  LONG          BufferSize;            /* size of the buffer                */
  ULONG         BytesRead;             /* actual bytes read                 */
  RexxMethod  * Method;                /* unflattened method                */
  RexxCode    * Code;                  /* parent rexx method                */
  RexxSource  * Source;                /* REXX source object                */
  RexxActivity *activity;              /* the current activity              */
                                       /* temporary read buffer             */
  CHAR          fileTag[sizeof(compiledHeader)];

  activity = CurrentActivity;          /* save the activity                 */
  ReleaseKernelAccess(activity);       /* release the access                */

                                       /* read the first file part          */
  BytesRead = fread(fileTag, 1, sizeof(compiledHeader), Handle);
                                       /* not a compiled file?              */
  if (strcmp(fileTag, compiledHeader) != 0) {
    RequestKernelAccess(activity);     /* get the lock back                 */
    fclose(Handle);                    /* close the file                    */
    return OREF_NULL;                  /* not a saved program               */
  }
                                       /* now read the control info         */
  BytesRead = fread((PCHAR)&Control, 1, sizeof(Control), Handle);
  RequestKernelAccess(activity);       /* get the lock back                 */
                                       /* check the control info            */
  if ((Control.MetaVersion != METAVERSION) || (Control.Magic != MAGIC)) {
    fclose(Handle);                    /* close the file                    */
                                       /* got an error here                 */
    report_exception1(Error_Program_unreadable_version, FileName);
  }
                                       /* read the file size                */
  BufferSize = Control.ImageSize;      /* get the method info size          */
  Buffer = new_buffer(BufferSize);     /* get a new buffer                  */
  save(Buffer);                        /* protect the buffer                */
                                       /* position relative to the end      */
  StartPointer = ((PCHAR)Buffer + ObjectSize(Buffer)) - BufferSize;
  ReleaseKernelAccess(activity);       /* release the access                */
                                       /* read the flattened method         */
  BytesRead = fread(StartPointer, 1, BufferSize, Handle);
  fclose(Handle);                      /* close the file                    */
  RequestKernelAccess(activity);       /* get the lock back                 */
                                       /* "puff" this out usable form       */
  Method = TheMethodClass->restore(Buffer, StartPointer);
  save(Method);                        /* protect the method code           */
  discard(Buffer);                     /* release the buffer protection     */
                        /* buffer need not to be holded because it is now an envelope and referenced by Method */
  Code = (RexxCode *)Method->code;     /* get the REXX code object          */
  Source = Code->u_source;             /* and now the source object         */
                                       /* switch the file name (this might  */
                                       /* be different than the name        */
  Source->setProgramName(FileName);    /* originally saved under            */
  discard_hold(Method);                /* now release the lock on this      */
  return Method;                       /* return the unflattened method     */
}


