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
/* REXX AIX  Support                                            aixmeta.c     */
/*                                                                            */
/* Unflatten saved methods from various sources.                              */
/* Not supported at this time                                                 */
/******************************************************************************/
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#define INCL_REXX_STREAM
#include "RexxCore.h"
#include "StringClass.hpp"
#include "BufferClass.hpp"
#include "RexxSmartBuffer.hpp"
#include "MethodClass.hpp"
#include "RexxCode.hpp"
#include "RexxActivity.hpp"
#include "SourceFile.hpp"
#include "ProtectedObject.hpp"

/*********************************************************************/
/*         Definitions for use by the Meta I/O functionality         */
/*********************************************************************/

#define  METAVERSION       34          /* 07/10/02 bump because of           */
                                       /* class list changed to arrays;      */
                                       /* bump 33 => 34 for new kernel and   */
                                       /* USERID BIF                         */
#define  MAGIC          11111          /* function                           */

#define VERPRE ORX_SYS_STR " " PACKAGE_VERSION " "
#define LENPRE strlen( VERPRE )

#define  compiledHeader "/**/@REXX"

#define  magicNumber "#!"              /* AIX magic number flag             */

typedef struct _control {              /* meta date control info            */
    unsigned short Magic;              /* identifies as 'meta' prog         */
    unsigned short MetaVersion;        /* version of the meta prog          */
    char           RexxVersion[40];    /* version of rexx intrpreter        */
    size_t         ImageSize;          /* size of the method info           */
} FILE_CONTROL;                        /* saved control info                */
                                       /* size of control structure         */
#define  CONTROLSZ      sizeof(FILE_CONTROL)

typedef FILE_CONTROL *PFILE_CONTROL;   /* pointer to file info              */

RexxMethod *SysRestoreTranslatedProgram(RexxString *,FILE *);

/*********************************************************************/
/*                                                                   */
/*  Function:    SysRestoreProgram                                   */
/*                                                                   */
/*  Description: This function is used to load the meta data from a  */
/*               file into the proper interpreter variables.  It     */
/*               also checks to see if the target file has an AIX    */
/*               magic number in the first line.  If it does, it     */
/*               replaces the magic number with a blank line and     */
/*               creates a new method.                               */
/*********************************************************************/

RexxMethod *SysRestoreProgram(
  RexxString *FileName )               /* name of file to process           */
{
  const char   *File;                  /* ASCII-Z file name                 */
  FILE         *Handle;                /* file handle                       */
  RexxMethod   *Method;                /* unflattened method                */
  char          fileTag[sizeof(magicNumber)];
  size_t        buffersize,            /* size of the buffer                */
                position;              /* Temp file location                */
  RexxBuffer  * buffer;                /* Buffer to unflatten               */

  // if we're in save image mode, we never used a saved image
  if (memoryObject.savingImage())
  {
      return OREF_NULL;
  }

  File = FileName->getStringData();    /* get the file name pointer         */
  Handle = fopen(File, "rb");          /* open the file                     */
  if (Handle == NULL)                  /* get anything?                     */
    return OREF_NULL;                  /* no restored image                 */
                                       /* see if this is a "sourceless" one */
  Method = SysRestoreTranslatedProgram(FileName, Handle);
  if (Method != OREF_NULL)             /* get a method out of this?         */
    return Method;                     /* this process is finished          */

                                       /* Check for magic number            */
  fseek(Handle,0,SEEK_SET);            /* Reset file position               */
                                       /* Read in start of file             */
  fread(fileTag, 1, sizeof(magicNumber), Handle);
                                       /* See if we have a magic number     */
  if (memcmp(fileTag, magicNumber, sizeof(magicNumber) - 1) == 0) {
    fseek(Handle, 0, SEEK_END);        /* seek to the file end              */
    buffersize = ftell(Handle);        /* get the file size                 */
    fseek(Handle, 0, SEEK_SET);        /* seek back to the file beginning   */
                                       /* Move past magic number by finding */
                                       /* first lineend character           */
    while (memcmp(fileTag,line_end,line_end_size)) {
      fread(fileTag,1,1,Handle);
    }
    position = ftell(Handle);          /* Find out where we are             */
                                       /* Adjust size of buffer due to      */
                                       /* removal of magic number line      */
                                       /* Add one for lineend character     */
    buffersize = buffersize - position + 1;
    buffer = new_buffer(buffersize);   /* get a buffer object               */
                                       /* Copy in a blank line              */
    memcpy(buffer->address(),line_end,line_end_size);
                                       /* read the entire file in one shot  */
    fread(buffer->address()+line_end_size, 1, buffersize, Handle);
    fclose(Handle);                    /* close the file                    */
    ProtectedObject p(buffer);
                                       /* Create a method object            */
    Method = TheMethodClass->newRexxBuffer(FileName,buffer,(RexxClass *)TheNilObject);
    return Method;
  }
    fclose(Handle);                    /* close the file                    */

  return OREF_NULL;                    /* return the unflattened method     */
}

/*********************************************************************/
/*                                                                   */
/*  Function:    SysSaveProgram                                      */
/*                                                                   */
/*  Description: This function saves a flattened method to a file    */
/*                                                                   */
/*********************************************************************/

void SysSaveProgram(
  RexxString * FileName,               /* name of file to process           */
  RexxMethod * Method )                /* method to save                    */
{
  return;
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
  char         *MethodInfo;            /* buffered flattened method         */
  char         *StartPointer;          /* start of buffered information     */
  RexxBuffer  * Buffer;                /* Buffer to unflatten               */
  size_t        BufferSize;            /* size of the buffer                */
  RexxMethod  * Method;                /* unflattened method                */
  RexxSource  * Source;                /* REXX source object                */

                                       /* address the control information   */
  Control = (PFILE_CONTROL)InBuffer->strptr;
                                       /* check the control info            */
  if ((Control->MetaVersion != METAVERSION) ||
      (Control->Magic != MAGIC)) {
    return OREF_NULL;                  /* can't load it                     */
  }
                                       /* point to the flattened method     */
  MethodInfo = InBuffer->strptr + CONTROLSZ;
                                       /* get the buffer size               */
  BufferSize = InBuffer->strlength - CONTROLSZ;
  Buffer = new_buffer(BufferSize);     /* get a new buffer                  */
                                       /* position relative to the end      */
  StartPointer = ((char *)Buffer + Buffer->getObjectSize()) - BufferSize;
                                       /* fill in the buffer                */
  memcpy(StartPointer, MethodInfo, BufferSize);
  ProtectedObject p(Buffer);
                                       /* "puff" this out usable form       */
  Method = TheMethodClass->restore(Buffer, StartPointer);
                                       /* and now the source object         */
  Source = ((RexxCode*)Method->getCode())->getSourceObject();   /* and now the source object         */
                                       /* switch the file name (this might  */
                                       /* be different than the name        */
  Source->setProgramName(Name);        /* originally saved under            */
                                       /* NOTE:  no source file reconnect   */
                                       /* is possible here                  */
  return Method;                       /* return the unflattened method     */
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
  char         *Buffer;                /* buffer pointer                    */
  RexxBuffer  * MethodBuffer;          /* flattened method                  */
  RexxSmartBuffer *FlatBuffer;         /* flattened smart buffer            */
  char         *BufferAddress;         /* address of flattened method data  */
  size_t        BufferLength;          /* length of the flattened method    */
  RexxString  * Version;               /* REXX version string               */

  ProtectedObject p(Method);
  FlatBuffer = Method->saveMethod();   /* flatten the method                */
                                       /* retrieve the length of the buffer */
  BufferLength = FlatBuffer->current;
  MethodBuffer = FlatBuffer->buffer;   /* get to the actual data buffer     */
  BufferAddress = MethodBuffer->address(); /* retrieve buffer starting address  */
                                       /* get the final buffer              */
  Buffer = (char *)SysAllocateResultMemory(BufferLength + CONTROLSZ);
  OutBuffer->strptr = Buffer;          /* fill in the result pointer        */
                                       /* and the result length             */
  OutBuffer->strlength = BufferLength + CONTROLSZ;
  Control = (PFILE_CONTROL)Buffer;     /* set pointer to control info       */
                                       /* fill in version info              */
  memcpy(Control->RexxVersion, VERPRE, LENPRE);
  Version = version_number();          /* get the version string            */
                                       /* copy in the version string        */
  memcpy((Control->RexxVersion) + LENPRE, Version->getStringData(), Version->getLength() + 1);

  Control->MetaVersion = METAVERSION;  /* current meta version              */
  Control->Magic = MAGIC;              /* magic signature number            */
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

void SysSaveTranslatedProgram(
  const char  *File,                   /* name of file to process           */
  RexxMethod * Method )                /* method to save                    */
{
  FILE         *Handle;                /* output file handle                */
  FILE_CONTROL  Control;               /* control information               */
  RexxBuffer *  MethodBuffer;          /* flattened method                  */
  RexxSmartBuffer *FlatBuffer;         /* flattened smart buffer            */
  char         *BufferAddress;         /* address of flattened method data  */
  size_t        BufferLength;          /* length of the flattened method    */
  RexxString  * Version;               /* REXX version string               */
  RexxActivity *activity;              /* the current activity              */

  Handle = fopen(File, "wb");          /* open the output file              */
  if (Handle == NULL)                  /* get an open error?                */
                                       /* got an error here                 */
    reportException(Error_Program_unreadable_output_error, File);
  ProtectedObject p1(Method);
  FlatBuffer = Method->saveMethod();   /* flatten the method                */
  ProtectedObject p2(FlatBuffer);
                                       /* retrieve the length of the buffer */
  BufferLength = FlatBuffer->current;
  MethodBuffer = FlatBuffer->buffer;   /* get to the actual data buffer     */
  BufferAddress = MethodBuffer->address();  /* retrieve buffer starting address  */
                                       /* clear out the cntrol info         */
  memset((void *)&Control, 0, sizeof(Control));
                                       /* fill in version info              */
  memcpy(Control.RexxVersion, VERPRE, LENPRE);
  Version = version_number();          /* get the version string            */
  memcpy((char *)Control.RexxVersion + LENPRE, Version->getStringData(),
               Version->getLength()>40-LENPRE?40-LENPRE:Version->getLength());
  Control.MetaVersion = METAVERSION;   /* current meta version              */
  Control.Magic = MAGIC;               /* magic signature number            */
  Control.ImageSize = BufferLength;    /* add the buffer length             */

  activity = ActivityManager::currentActivity;          /* save the activity                 */
  activity->requestAccess();           /* release the access                */
                                       /* write out the REXX signature      */
  fwrite(compiledHeader, 1, sizeof(compiledHeader), Handle);
                                       /* now the control info              */
  fwrite(&Control, 1, sizeof(Control), Handle);
                                       /* and finally the flattened method  */
  fwrite(BufferAddress, 1, BufferLength, Handle);
  fclose(Handle);                      /* done saving                       */
  activity->requestAccess();           /* and reaquire the kernel lock      */
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
  RexxString *FileName,                /* name of the translate file        */
  FILE       *Handle )                 /* handle of the file to process     */
{
  FILE_CONTROL  Control;               /* control information               */
  char         *StartPointer;          /* start of buffered method          */
  RexxBuffer   *Buffer;                /* Buffer to unflatten               */
  size_t        BufferSize;            /* size of the buffer                */
  RexxMethod   *Method;                /* unflattened method                */
  RexxSource   *Source;                /* REXX source object                */
                                       /* temporary read buffer             */
  char          fileTag[sizeof(compiledHeader)];

  {
      UnsafeBlock releaser;
                                          /* read the first file part          */
     fread(fileTag, 1, sizeof(compiledHeader), Handle);
                                          /* not a compiled file?              */
     if (strcmp(fileTag, compiledHeader) != 0) {
       return OREF_NULL;                  /* not a saved program               */
     }
                                          /* now read the control info         */
     fread((char *)&Control, 1, sizeof(Control), Handle);

  }
                                       /* check the control info            */
  if ((Control.MetaVersion != METAVERSION) || (Control.Magic != MAGIC)) {
                                       /* got an error here                 */
    reportException(Error_Program_unreadable_version, FileName);
  }
                                       /* read the file size                */
  BufferSize = Control.ImageSize;      /* get the method info size          */
  Buffer = new_buffer(BufferSize);     /* get a new buffer                  */
  ProtectedObject p1(Buffer);
                                       /* position relative to the end      */
  StartPointer = ((char *)Buffer + Buffer->getObjectSize()) - BufferSize;
  {
      UnsafeBlock releaser;

                                           /* read the flattened method         */
      fread(StartPointer, 1, BufferSize, Handle);
      fclose(Handle);                      /* close the file                    */
  }
                                       /* "puff" this out usable form       */
  Method = TheMethodClass->restore(Buffer, StartPointer);
  ProtectedObject p2(Method);
  Source = ((RexxCode*)Method->getCode())->getSourceObject();   /* and now the source object         */
                                       /* switch the file name (this might  */
                                       /* be different than the name        */
  Source->setProgramName(FileName);    /* originally saved under            */
  return Method;                       /* return the unflattened method     */
}
