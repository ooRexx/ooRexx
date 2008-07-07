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
/*  Module Name:   WINMACRO.C                                        */
/*                                                                   */
/*  Description:   This module handles all macro space API's         */
/*                                                                   */
/*  Entry Points:  RxMacroChange()            add/change a func      */
/*                 RxMacroDrop()              delete a func          */
/*                 RxMacroQuery()             check for a func       */
/*                 RxMacroReOrder()           change search position */
/*                 RxMacroErase()             clear macro space      */
/*                 RxMacroSave()              save funcs to a file   */
/*                 RxMacroLoad()              load funcs from a file */
/*                                                                   */
/*  IUO Entry:     RxMacroFunctionExecute()  retrieve function image */
/*                                                                   */
/*  Notes:         Functions existing in the macro space cannot make */
/*                 use of the REXX sourceline() function.  Any calls */
/*                 to this function result in a 'Program is          */
/*                 unreadable' error from the interpreter            */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/* Please note the following:                                        */
/*                                                                   */
/* Functions in this module manipulate data that must be available   */
/* to all processes that call REXX API functions.  These processes   */
/* may invoke the REXX interpreter, or make direct calls to the      */
/* API routines.                                                     */
/*                                                                   */
/* In addition, functions in this module may establish data that     */
/* must persist after the calling process terminates.                */
/*                                                                   */
/* To satisfy these requirements, the system must maintain a process */
/* that serves as a data repository.  Functions in this module then  */
/* give critical data to the repository, and the data persists as    */
/* long as the repository process continues running.                 */
/*                                                                   */
/*********************************************************************/
#include "rexx.h"                      /* RXSTRING & REXXSAA() & APIs*/
#include "io.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "malloc.h"
#include "APIServiceTables.h"          /* common api definitions     */
#include "RexxAPIManager.h"            /* RxMacroFunctionExecute()   */
#include "ASCIISymbols.h"              /* code-page for characters   */
#include "APIServiceSystem.h"
#include "APIServiceMessages.h"        /* Window Messages   */
#include "RexxInternalApis.h"

#include "APIUtil.h"
#include "Characters.h"

typedef  LONG REXXENTRY REXX (
         LONG ,                        /* Num of args passed to rexx */
         PRXSTRING,                    /* Array of args passed to rex*/
         PSZ,                          /* [d:][path] filename[.ext]  */
         PRXSTRING,                    /* Loc of rexx proc in memory */
         PSZ,                          /* ASCIIZ initial environment.*/
         LONG ,                        /* type (command,subrtn,funct)*/
         PRXSYSEXIT,                   /* SysExit env. names &  codes*/
         PSHORT,                       /* Ret code from if numeric   */
         PRXSTRING );                  /* Retvalue from the rexx proc*/

#define YES  1                         /* used for flags             */
#define NO   0                         /* used for flags             */

#define SLN  strlength
#define SPT  strptr
static RXSTRING RXSTRING_EMPTY = { 0L, (PCH)0 };

#define RXVERSION  "REXX-ooRexx 6.03"  /* interpreter version str    */
#define RXVERSIZE  16                  /* size of RXVERSION str      */
#define SIGNATURE  0xddd5              /* macro space file marker    */
#define FNAMSIZE   256                 /* file name size             */
#define EXT_SIZE   3                   /* file extension size        */


extern REXXAPIDATA * RexxinitExports;   /* Global state data  */

/* Now needed for local init because RX is process global */
extern LOCALREXXAPIDATA RexxinitLocal;

#define  ONRW           (ULONG)( O_RDWR | O_CREAT )

#define HFILE HANDLE
typedef HFILE *PHFILE;

#define PPVOID VOID**

/*********************************************************************/
/*****        Macro Space Function List Access Functions         *****/
/*********************************************************************/

extern _declspec(dllimport) CRITICAL_SECTION nest;

static int
  callrexx(const char *,PMACRO);       /* call the REXXSAA interprtr */

static int
  request(size_t, const char **, const char *);            /* check a list for a string  */

static BOOL
  eraselst(PMACRO);                    /* free a list of macros      */

static PMACRO
  does_exist(const char *,PMACRO *);   /* see if a func exists       */

static ULONG
  file_read(HFILE,PVOID,ULONG);        /* read from an open file     */

static ULONG
  file_write(HFILE,PVOID,ULONG);       /* write to an open file      */

static INT
  saved_macro(const char *, PMACRO);   /* see if a func exists       */

static int
  ldmacro(size_t, const char **,HFILE); /* load macro space           */

static int
  macrofile_open(const char *, HFILE *);  /* open a macro space file    */

/*********************************************************************/
/*****              RXSTRING Manipulation Functions              *****/
/*********************************************************************/

static ULONG
  rxstrfrmfile(HFILE,PRXSTRING,ULONG); /* read a RXSTRING from file  */

static size_t
  rxstrlen(RXSTRING);                  /* strlen of a RXSTRING       */


extern BOOL MapComBlock(int chain);
extern void UnmapComBlock(int chain);

#define LIST_INIT 0
#define LIST_HEADER 1
#define LIST_IMAGE 2
#define LIST_CLEANUP 3

#define RECEIVE_ADD 0
#define RECEIVE_EXECUTE 1
#define RECEIVE_HEADERLIST 2
#define RECEIVE_IMAGELIST 3

static BOOL ReceiveMacro(PMACRO element, ULONG kind);
static void ReturnMacro(PMACRO element, BOOL withImage);
static RXQUEUE_TALK * FillMacroComBlock(BOOL add, const char *name, const char *data, size_t datalen, size_t spos);
static RXQUEUE_TALK * FillMacroComBlock_List(size_t argc, const char ** argv);
static BOOL CheckMacroComBlock();


/* functions that are called from rxapi.exe */
extern _declspec(dllexport) RexxReturnCode APIAddMacro(BOOL updateIfExists);
extern _declspec(dllexport) RexxReturnCode APIDropMacro(void);
extern _declspec(dllexport) RexxReturnCode APIClearMacroSpace(void);
extern _declspec(dllexport) RexxReturnCode APIQueryMacro(void);
extern _declspec(dllexport) RexxReturnCode APIReorderMacro(void);
extern _declspec(dllexport) RexxReturnCode APIExecuteMacroFunction(void);
extern _declspec(dllexport) RexxReturnCode APIList(int kind);

/*********************************************************************/

/*********************************************************************/
/*                                                                   */
/*  Function Name:      RexxAddMacro                                 */
/*                                                                   */
/*  Description:        change or insert a macro's pcode and literal */
/*                      images in the workspace                      */
/*                                                                   */
/*  Entry Point:        RexxAddMacro                                 */
/*                                                                   */
/*  Inputs:             n   - macro name asciiz string               */
/*                      s   - filename asciiz string                 */
/*                      pos - search order position                  */
/*                                                                   */
/*  Output:             return code                                  */
/*                                                                   */
/*********************************************************************/

RexxReturnCode
REXXENTRY
RexxAddMacro(
  const char *n,                       /* name of macro function     */
  const char *s,                       /* name of file               */
  size_t pos )                         /* search order pos request   */
{
  MACRO  p;
  RexxReturnCode  rc;

  if (pos != RXMACRO_SEARCH_BEFORE &&  /* if pos flag not before and */
       pos != RXMACRO_SEARCH_AFTER )   /*   not after, then          */
    return RXMACRO_INVALID_POSITION;   /*   return proper error code */

  if (!API_RUNNING()) return (RXMACRO_NO_STORAGE);

  if (!CheckMacroComBlock()) return(RXMACRO_NO_STORAGE);

  if (!(rc=callrexx(s, &p))) {        /* call REXXSAA to get image  */
      APISTARTUP_MACRO();                        /* do common entry code       */
      if (FillMacroComBlock(TRUE, n, p.image.strptr, p.i_size, pos))
          rc = (RexxReturnCode)MySendMessage(RXAPI_MACRO,MACRO_ADD,1 /* update if exists */);
      else rc = RXMACRO_NO_STORAGE;
      GlobalFree(p.image.strptr);
      APICLEANUP_MACRO();                        /* release shared resources   */
  }                                  /*                            */

  return (rc);                         /* and exit with return code  */
}


RexxReturnCode APIAddMacro(BOOL updateIfExists)
{
  PCHAR   str;
  PMACRO  p;                           /* existing macro             */
  ULONG   rc = 0;
  RXMACRO_TALK * intercom;

  intercom = (RXMACRO_TALK *)RX.comblock[API_MACRO];
  if (!MACROIMAGE_ABS(intercom)) return RXMACRO_NO_STORAGE;

  if (p=does_exist(intercom->name,PMNULL)) {
      if (!updateIfExists) return(RXMACRO_ALREADY_EXISTS);  /* no duplicates allowed */
      /* macro with this name already exists so update image */
      str = GlobalAlloc(GMEM_FIXED, intercom->i_size+1);  /* allocate new image */
      if (!str) return RXMACRO_NO_STORAGE;
      GlobalFree(p->image.strptr);                      /* free old image */
      p->image.strptr = str;
      ReceiveMacro(p, RECEIVE_ADD);
  }                                    /* name not found, so add it  */
                                       /* allocate header storage    */
  else if (p=GlobalAlloc(GMEM_FIXED, MACROSIZE))
  {
      p->image.strptr = GlobalAlloc(GMEM_FIXED, intercom->i_size+1);  /* allocate new image */
       if (p->image.strptr)
      {
           ReceiveMacro(p, RECEIVE_ADD);
          p->next=(PMACRO) RX.macrobase;                /* chain it in                */
          RX.macrobase=(PCHAR) p;                       /* at head of chain           */
          RX.macrocount++;                     /* count it                   */
      }
      else
      {
           GlobalFree(p);
          rc = RXMACRO_NO_STORAGE;
      }
  } else rc=RXMACRO_NO_STORAGE;

  return rc;
}



/*********************************************************************/
/*                                                                   */
/*  Function Name:      RexxDropMacro                                */
/*                                                                   */
/*  Description:        remove a macro's pcode and literal images    */
/*                                                                   */
/*  Entry Point:        RexxDropMacro                                */
/*                                                                   */
/*  Input:              n - macro name asciiz string                 */
/*                                                                   */
/*  Output:             return code                                  */
/*                                                                   */
/*********************************************************************/

RexxReturnCode
REXXENTRY
RexxDropMacro(const char *n)                       /* name of macro to delete    */
{
  RexxReturnCode rc;                           /* return code from function  */

  if (!API_RUNNING()) return (RXMACRO_NO_STORAGE);

  if (!RX.macrobase) return RXMACRO_NOT_FOUND;   /* no macro registered at all */

  if (!CheckMacroComBlock()) return(RXMACRO_NO_STORAGE);

  APISTARTUP_MACRO();                        /* do common entry code       */

  if (FillMacroComBlock(FALSE, n, NULL, 0, 0))    /* just put the name into com port */
      rc = (RexxReturnCode)MySendMessage(RXAPI_MACRO,MACRO_DROP,0);
  else rc = RXMACRO_NO_STORAGE;

  APICLEANUP_MACRO();                        /* release shared resources   */
  return (rc);                         /* and exit with return code  */
}



RexxReturnCode APIDropMacro(void)
{
  PMACRO p = NULL;                     /* pointer to function struct */
  PMACRO prev = NULL;
  ULONG  rc;                           /* return code from function  */
  RXMACRO_TALK * intercom;
  intercom = (RXMACRO_TALK *)RX.comblock[API_MACRO];

  if (p=does_exist(intercom->name,&prev)) {            /* found name, so delete it   */
      if (prev) prev->next = p->next;      /* if previous, go around it  */
      else
         RX.macrobase = (PCHAR) p->next;   /* else first, so fix base    */
      RX.macrocount--;                   /* decrement macro counter    */

      GlobalFree(p->image.strptr);
      GlobalFree(p);
      rc = RXMACRO_OK;                   /* set successful return code */
  }                                      /* end of "if p..."           */
  else rc = RXMACRO_NOT_FOUND;           /* name not found, so error   */
  return (rc);                           /* and exit with return code  */
}

/*********************************************************************/
/*                                                                   */
/*  Function Name:      RexxClearMacroSpace                          */
/*                                                                   */
/*  Description:        erase all entries in the macro chain         */
/*                                                                   */
/*  Entry Point:        RexxCleanMacroSpace                          */
/*                                                                   */
/*  Output:             return code                                  */
/*                                                                   */
/*********************************************************************/

RexxReturnCode
REXXENTRY
RexxClearMacroSpace(void)
{
  RexxReturnCode rc;                           /* return code from function  */

  if (!API_RUNNING()) return (RXMACRO_NO_STORAGE);

  if (!RX.macrobase) return RXMACRO_NOT_FOUND;   /* no macro registered at all */
  if (!CheckMacroComBlock()) return(RXMACRO_NO_STORAGE);

  APISTARTUP_MACRO();                        /* do common entry code       */
  rc = (RexxReturnCode)MySendMessage(RXAPI_MACRO,MACRO_CLEAR,0);
  APICLEANUP_MACRO();                        /* release shared resources   */
  return (rc);                         /* and exit with return code  */
}


RexxReturnCode APIClearMacroSpace(void)
{
  if (eraselst((PMACRO)RX.macrobase))           /* free all entries in list   */
  {
      RX.macrobase = (PCHAR)0;             /* reset list pointer         */
      RX.macrocount = 0;            /* reset function counter     */
      return RXMACRO_OK;                   /* set return code to success */
  } else return RXMACRO_NOT_FOUND;
}


/*********************************************************************/
/*                                                                   */
/*  Function Name:      RexxSaveMacroSpace                           */
/*                                                                   */
/*  Description:        saves all entries in the macro chain         */
/*                                                                   */
/*  Notes:              File Format:                                 */
/*                         - REXXSAA interpreter version string      */
/*                         - Macro Space file signature flag         */
/*                         - MACRO structures for functions          */
/*                         - RXSTRING structures for images          */
/*                                                                   */
/*  Entry Point:        RexxSaveMacroSpace                           */
/*                                                                   */
/*  Input:              ac   - number of requested functions (0=all) */
/*                      av   - list of function names to save        */
/*                      fnam - name of file to save macros in        */
/*                                                                   */
/*  Output:             return code                                  */
/*                                                                   */
/*********************************************************************/
RexxReturnCode
REXXENTRY
RexxSaveMacroSpace(
  size_t   ac,                         /* count of arguments         */
  const char **av,                     /* argument list              */
  const char * fnam )                  /* file name                  */
{
  ULONG  i;
  MACRO tmp;
  HFILE f;

  LRESULT found;
  RexxReturnCode rc = 0;

  if (!API_RUNNING()) return (RXMACRO_NO_STORAGE);

  if (!RX.macrobase) return RXMACRO_NOT_FOUND;   /* no macro registered at all */
  if (!CheckMacroComBlock()) return(RXMACRO_NO_STORAGE);

  APISTARTUP_MACRO();                        /* do common entry code       */
  if (!av) ac = 0;

  if (FillMacroComBlock_List(ac, av))    /* write arglist to com port */
  {
      found = MySendMessage(RXAPI_MACRO,MACRO_LIST,LIST_INIT);
      if (!found) rc = RXMACRO_NOT_FOUND;
      else if ((f = (HFILE)CreateFile(fnam, GENERIC_READ | GENERIC_WRITE,
                                      0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0L)
               )== (HFILE)INVALID_HANDLE_VALUE) /* if file open has a problem */
                   rc = RXMACRO_FILE_ERROR;           /* was some sort of file error*/
      if (!rc)
         rc = file_write(f,(PVOID)RXVERSION,/* write version to the file  */
                    RXVERSIZE);        /*                            */
      if (!rc) {                       /* if file write was ok...    */
          i = SIGNATURE;               /* get internal file flag     */
          rc = file_write(f, (PVOID)&i,sizeof(i)); /* write flag to the file     */
      }                                /*                            */
      if (!rc)
          rc = file_write(f,(PVOID)&RX.mcount,sizeof(RX.mcount)); /* write func count to file   */

      /* now write all the macro headers */
      while (found && !rc)
      {
          ReceiveMacro(&tmp, RECEIVE_HEADERLIST);
          tmp.image.strptr = NULL;
          rc = file_write(f,(PVOID)&tmp,MACROSIZE); /* write macro to file   */
          if (!rc) found = MySendMessage(RXAPI_MACRO,MACRO_LIST,LIST_HEADER);
      }

      /* now write all the images */
      if (!rc)
      {
          found = MySendMessage(RXAPI_MACRO,MACRO_LIST,LIST_IMAGE);
          if (!found) rc = RXMACRO_NOT_FOUND;
      }

      while (found && !rc)
      {
          if (ReceiveMacro(&tmp, RECEIVE_IMAGELIST))
          {
              rc = file_write(f,tmp.image.SPT,(ULONG)tmp.image.SLN); /* write image to file   */
              GlobalFree(tmp.image.SPT);    /* allocated within ReceiveMacro */
              if (!rc) found = MySendMessage(RXAPI_MACRO,MACRO_LIST,LIST_IMAGE);
          } else rc = RXMACRO_NO_STORAGE;
      }
      MySendMessage(RXAPI_MACRO,MACRO_LIST,LIST_CLEANUP);

      if (!CloseHandle(f) || rc) {           /* there was some write error */
          DeleteFile(fnam);                 /* delete the corrupted file  */
          rc = RXMACRO_FILE_ERROR;         /* set rc to file error       */
      }
      else rc = RXMACRO_OK;              /* set rc to success          */
  } else rc = RXMACRO_NO_STORAGE;

  APICLEANUP_MACRO();                        /* release shared resources   */
  return (rc);                         /* and exit with return code  */
}



static BOOL getNextMacro(ULONG kind)
{
  PMACRO m;                            /* pointer to loop thru list  */

  if (kind == LIST_INIT)
  {      /* if we are in here for the first time do a macro count */
      PMACRO tmp;

      tmp = m = (PMACRO) RX.macrobase;

      /* arguments are passed along so we must seek through the list */
      if (RX.ListArgCount)
      {
          while (tmp)
          {
               if (request(RX.ListArgCount,(PSZ *)RX.ListArgPtr,tmp->name))
              RX.mcount++;
              tmp = tmp->next;
          }
      } else RX.mcount = RX.macrocount;   /* use all */
  }

  if (!RX.InternalMacroPtr) m = (PMACRO) RX.macrobase;
  else m = ((PMACRO)RX.InternalMacroPtr)->next;

  while (m)
  {
      if ( !RX.ListArgCount || request(RX.ListArgCount,(PSZ *)RX.ListArgPtr,m->name))
      {
          ReturnMacro(m, (kind == LIST_IMAGE));
          RX.InternalMacroPtr = (PCHAR) m;
          return TRUE;
      } else m = m->next; /* didn't match, so try next */
  }
  RX.InternalMacroPtr = NULL;   /* no more macro found that fits */
  return FALSE;
}


RexxReturnCode APIList(int kind)
{
  RXMACRO_TALK * intercom;
  ULONG i, size = 0;
  PSZ * ptr;
  PSZ str;
  size_t sl;

  intercom = (RXMACRO_TALK *)RX.comblock[API_MACRO];
  if (kind == LIST_INIT)
  {
      RX.InternalMacroPtr = NULL;
      RX.mcount = 0;

      RX.ListArgCount = intercom->i_size;
      if (RX.ListArgCount)
      {
          ptr = (PSZ *) RX.ListArgPtr = GlobalAlloc(GMEM_FIXED, intercom->image.strlength+sizeof(PSZ)*RX.ListArgCount);
          if (!RX.ListArgPtr) return 0;
          str = MACROIMAGE_ABS(intercom);
          ptr[0] = (PCHAR) ptr + RX.ListArgCount;
          for (i=0; i<RX.ListArgCount;i++) /* copy vector */
          {
              if (i>0) ptr[i] = ptr[i-1]+sl;
              sl = strlen(str)+1;
              memcpy(ptr[i], str, sl);
              str += sl;
          }
      } else RX.ListArgPtr = NULL;
  }

  if (kind == LIST_CLEANUP)
  {
      if (RX.ListArgPtr) GlobalFree(RX.ListArgPtr);
      RX.ListArgPtr = NULL;
      return RXMACRO_OK;
  }
  else return getNextMacro(kind);
}





/*********************************************************************/
/*                                                                   */
/*  Function Name:      RexxLoadMacroSpace                           */
/*                                                                   */
/*  Description:        loads entries from a file into macro space   */
/*                                                                   */
/*  Entry Point:        RexxLoadMacroSpace                           */
/*                                                                   */
/*  Input:              ac   - number of requested functions (0=all) */
/*                      av   - list of function names to load        */
/*                      fnam - name of file to load macros from      */
/*                                                                   */
/*  Output:             return code                                  */
/*                                                                   */
/*********************************************************************/
RexxReturnCode
REXXENTRY
RexxLoadMacroSpace(
  size_t   ac,                         /* argument count             */
  const char **av,                     /* list of argument strings   */
  const char * fnam )                  /* file name to load functs   */
{

  ULONG  rc = 0;
  HFILE f;

  if (!API_RUNNING()) return (RXMACRO_NO_STORAGE);

  if (!CheckMacroComBlock()) return(RXMACRO_NO_STORAGE);

  APISTARTUP_MACRO();                    /* do common entry code       */

  if (!(rc = macrofile_open(fnam,&f))){  /* if no error opening file   */
      if (!av) ac = (ULONG )0;           /* check for no list          */
      rc = ldmacro(ac,av,f);             /* do remainder of load macro */
      if (!CloseHandle(f) && !rc)        /* close the file             */
          rc = RXMACRO_FILE_ERROR;       /* error trying to close file */
  }
  APICLEANUP_MACRO();                  /* release shared resources   */
  return (rc);                         /* and exit with return code  */
}                                      /* end of RxMacroLoad()       */

/*********************************************************************/
/*                                                                   */
/*  Function Name:      RexxQueryMacro                               */
/*                                                                   */
/*  Description:        search for a function in the workspace       */
/*                                                                   */
/*  Entry Point:        RexxQueryMacro                               */
/*                                                                   */
/*  Input:              name - name of function to look for          */
/*                      pos  - pointer to storage for return of pos  */
/*                                                                   */
/*  Output:             return code                                  */
/*                                                                   */
/*********************************************************************/

RexxReturnCode
REXXENTRY
RexxQueryMacro(
  const char *name,                    /* name to search for         */
  unsigned short *pos )                /* pointer for return of pos  */
{
  RexxReturnCode rc;                           /* return code from call      */

  if (!API_RUNNING()) return (RXMACRO_NO_STORAGE);

  if (!RX.macrobase) return RXMACRO_NOT_FOUND;   /* no macro registered at all */
  if (!CheckMacroComBlock()) return(RXMACRO_NO_STORAGE);

  APISTARTUP_MACRO();                        /* do common entry code       */

  if (FillMacroComBlock(FALSE, name, NULL, 0, 0))    /* just put the name into com port */
  {
      rc = (RexxReturnCode)MySendMessage(RXAPI_MACRO,MACRO_QUERY,0);
      if (rc == RXMACRO_OK) *pos = (USHORT) ((RXMACRO_TALK *)LRX.comblock[API_MACRO])->srch_pos;
  }
  else rc = RXMACRO_NO_STORAGE;

  APICLEANUP_MACRO();                        /* release shared resources   */
  return (rc);                         /* and exit with return code  */
}


RexxReturnCode APIQueryMacro(void)
{
  PMACRO tmp;                          /* temp pointer to record     */
  RXMACRO_TALK * intercom;

  intercom = (RXMACRO_TALK *)RX.comblock[API_MACRO];

  if(tmp=does_exist(intercom->name,PMNULL)){     /* if macro exists, then      */
    intercom->srch_pos = (USHORT)tmp->srch_pos;  /*   return position */
    return RXMACRO_OK;
  }
  else return RXMACRO_NOT_FOUND;
}

/*********************************************************************/
/*                                                                   */
/*  Function Name:      RxRecorderMacro                              */
/*                                                                   */
/*  Description:        change a functions search order position     */
/*                                                                   */
/*  Entry Point:        RexxReorderMacro                             */
/*                                                                   */
/*  Input:              name - name of function to change order for  */
/*                      pos  - new search order position             */
/*                                                                   */
/*  Output:             return code                                  */
/*                                                                   */
/*********************************************************************/

RexxReturnCode
REXXENTRY
RexxReorderMacro(
  const char *name,                    /* name of function to change */
  size_t pos )                         /* new position for function  */
{
  RexxReturnCode rc;                           /* return code from call      */

  if ( pos != RXMACRO_SEARCH_BEFORE && /* if pos flag not before and */
       pos != RXMACRO_SEARCH_AFTER )   /*   not after, then          */
    return RXMACRO_INVALID_POSITION;     /*   return proper error code */

  if (!API_RUNNING()) return (RXMACRO_NO_STORAGE);

  if (!RX.macrobase) return RXMACRO_NOT_FOUND;   /* no macro registered at all */
  if (!CheckMacroComBlock()) return(RXMACRO_NO_STORAGE);

  APISTARTUP_MACRO();                        /* do common entry code       */

  if (FillMacroComBlock(FALSE, name, NULL, 0, pos))    /* just put the name and pos into com port */
      rc = (RexxReturnCode)MySendMessage(RXAPI_MACRO,MACRO_REORDER,0);
  else rc = RXMACRO_NO_STORAGE;

  APICLEANUP_MACRO();                        /* release shared resources   */
  return (rc);                         /* and exit with return code  */
}


RexxReturnCode APIReorderMacro()
{
  PMACRO tmp;                          /* temp pointer to record     */
  RXMACRO_TALK * intercom;

  intercom = (RXMACRO_TALK *)RX.comblock[API_MACRO];

  if (tmp=does_exist(intercom->name,PMNULL)) { /* if macro exists, then      */
      tmp->srch_pos = intercom->srch_pos;      /*   set search order pos */
      return RXMACRO_OK;                       /*   set return to successful */
  }
  else return RXMACRO_NOT_FOUND;         /* set return code to failure */
}


/*********************************************************************/
/*                                                                   */
/*  Function Name:      RexxExecuteMacroFunction                     */
/*                                                                   */
/*  Description:        find a macro's pcode and literal images in   */
/*                      the workspace                                */
/*                                                                   */
/*  Entry Point:        RexxExecuteMacroFunction                     */
/*                                                                   */
/*  Inputs:             name - macro name asciiz string              */
/*                      p    - pointer for return of pcode+lits buf  */
/*                                                                   */
/*  Output:             return code                                  */
/*                                                                   */
/*********************************************************************/

RexxReturnCode REXXENTRY RexxExecuteMacroFunction(
  const char *name,                    /* name of func to find       */
  PRXSTRING p )                        /* storage for image return   */
{
  MACRO tmp;                           /* temp macro pointer         */
  RexxReturnCode rc=RXMACRO_OK;                /* return code from function  */

  if (!API_RUNNING()) return (RXMACRO_NO_STORAGE);

  if (!RX.macrobase) return RXMACRO_NOT_FOUND;   /* no macro registered at all */
  if (!CheckMacroComBlock()) return(RXMACRO_NO_STORAGE);

  APISTARTUP_MACRO();                        /* do common entry code       */

  if (FillMacroComBlock(FALSE, name, NULL, 0, 0))    /* just put the name into com port */
      rc = (RexxReturnCode)MySendMessage(RXAPI_MACRO,MACRO_EXECUTE,0);
  else rc = RXMACRO_NO_STORAGE;

  p->strptr = NULL;

  if (rc == RXMACRO_OK)
  {
      if (ReceiveMacro(&tmp, RECEIVE_EXECUTE))
      {
          p->strptr = tmp.image.strptr;  /* is allocated within ReceiveMacro */
          p->strlength = tmp.image.strlength;
      } else rc = RXMACRO_NO_STORAGE;
  }

  APICLEANUP_MACRO();                        /* release shared resources   */
  return (rc);                         /* and exit with return code  */
}



RexxReturnCode APIExecuteMacroFunction(void)
{
  PMACRO tmp;                          /* temp macro pointer         */
  ULONG  rc=RXMACRO_OK;                /* return code from function  */
  RXMACRO_TALK * intercom;

  intercom = (RXMACRO_TALK *)RX.comblock[API_MACRO];

  if(tmp=does_exist(intercom->name,PMNULL))         /* if name exists in list...  */
      ReturnMacro(tmp, TRUE);
  else rc=RXMACRO_NOT_FOUND;           /* name not found, so error   */
  return (rc);                         /* and exit with return code  */
}


/*********************************************************************/
/*                                                                   */
/*  Function Name:      callrexx                                     */
/*                                                                   */
/*  Description:        calls REXXSAA to tokenize a file             */
/*                                                                   */
/*  Entry Point:        callrexx(fnam, current)                      */
/*                                                                   */
/*  Inputs:             fnam    - name of file to tokenize           */
/*                      current - pointer to current function entry  */
/*                                                                   */
/*  Output:             return code (RXMACRO_... flag)               */
/*                                                                   */
/*********************************************************************/
static int callrexx(
  const char *fnam,
  PMACRO current )
{
  BY_HANDLE_FILE_INFORMATION status;
  HFILE      f;
  RXSTRING   av;
  RXSTRING   m[2];
  ULONG       i;
  ULONG      rc;
  REXX      *addr;
  SHORT      retval;
  HINSTANCE  hDll;

  f=CreateFile(fnam,GENERIC_READ, FILE_SHARE_READ, NULL,
       OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);

  if (f==INVALID_HANDLE_VALUE)         /* open the source file       */
    return(RXMACRO_SOURCE_NOT_FOUND);

  m[0] = RXSTRING_EMPTY;               /* empty string for read check*/
  m[1] = RXSTRING_EMPTY;               /* empty string for rtn image */
  rc = RXMACRO_OK;                     /*                            */

  if (!GetFileInformationByHandle(f,&status)) /* find the file size     */
    rc = RXMACRO_FILE_ERROR;           /* error getting file size    */
  else rc = rxstrfrmfile(f,            /* read the source file into  */
                        &(m[0]),       /*   a RXSTRING, passing in   */
                        status.nFileSizeLow);

  if (!CloseHandle(f) && !rc)          /* close source file          */
    rc = RXMACRO_FILE_ERROR;           /* say error if no other error*/

  if (rc)                              /* if any errors upto now...  */
    return(rc);                        /* return the error code      */

  av.SLN = 3L;                         /*   for call to REXXSAA()    */
  av.SPT = "//T";                      /* set parm for tokenize only */
  i = 0;                               /* initialize error code      */

  if ((hDll=LoadLibrary("REXX"))) {
    if (!(addr = (REXX*)GetProcAddress(hDll,"RexxStart")))
      rc = RXMACRO_FILE_ERROR;
    else if ((*addr)(1,&av,(PSZ)fnam,m,/* call REXXSAA interpreter   */
                NULL,0,(PRXSYSEXIT)0,  /*   to get a tokenized image */
                &retval,NULL) )        /*   of the source file       */
      rc = RXMACRO_FILE_ERROR;         /* file error - REXXSAA prblm */
    else if (i)                        /*                            */
      rc = RXMACRO_NO_STORAGE;         /*                            */

    FreeLibrary(hDll);
  }
  else rc = RXMACRO_FILE_ERROR;
  GlobalFree(m[0].SPT);                /* free the source RXSTRING   */
  current->image = m[1];               /* assign string to macro     */
  current->i_size= rxstrlen(m[1]);     /* get string length          */
  return rc;                           /*                            */
}

/*********************************************************************/
/*                                                                   */
/*  Function Name:      request                                      */
/*                                                                   */
/*  Description:        scans through argument list to find a string */
/*                                                                   */
/*  Entry Point:        request(argc, argv, name)                    */
/*                                                                   */
/*  Inputs:             argc - count of arguments in list            */
/*                      argv - list of ASCIIZ strings to be searched */
/*                      name - string to search for in the list      */
/*                                                                   */
/*  Output:             return code (YES or NO)                      */
/*                                                                   */
/*********************************************************************/
static int   request(
  size_t   argc,                       /* count of argument strings  */
  const char**argv,                    /* list of argument strings   */
  const char *name )                   /* string name to search for  */
{
  size_t i;                            /* counter to loop thru args  */
  int    rc;                           /* flag for return value      */
                                       /*                            */
  rc = NO;                             /* initialize return to NO    */
  for (i = 0; i < argc; i++ ) {        /* move through list of args  */
    if (!_stricmp(name, argv[i]))      /* if the strings match, then */
      rc = YES;                        /*   set return flag to YES   */
    }                                  /* end of "for..." loop       */
  return rc;                           /* return YES or NO flag      */
}




/*********************************************************************/
/*                                                                   */
/*  Function Name:      eraselst                                     */
/*                                                                   */
/*  Description:        free all macro space entries                 */
/*                      and its images                               */
/*                                                                   */
/*  Entry Point:        eraselst(ptr)                                */
/*                                                                   */
/*  Inputs:             ptr - pointer to start of temporary list     */
/*                                                                   */
/*  Output:             (none)                                       */
/*                                                                   */
/*********************************************************************/
static BOOL eraselst(
  PMACRO ptr )                         /* pointer to list start      */
{
  PMACRO t = (PMACRO)0;                /* pointer to loop thru list  */
  BOOL err = FALSE;

  while (ptr && !err) {                /* while valid pointer...     */
    t=ptr->next;
    err = GlobalFree(ptr->image.SPT) != 0;  /* free this block's string   */
    if (!err) err = GlobalFree(ptr) != 0;   /* free macro header          */
    ptr=t;                             /* point to next, if any      */
  }                                    /* end of "while..." loop     */
  return (!err);                       /* return to caller           */
}


/*********************************************************************/
/*                                                                   */
/*  Function Name:      does_exist                                   */
/*                                                                   */
/*  Description:        scans through the macro space for a function */
/*                                                                   */
/*  Entry Point:        does_exist(name, prev)                       */
/*                                                                   */
/*  Inputs:             name - name of function to find              */
/*                      prev - pointer for return of previous entry  */
/*                                                                   */
/*  Output:             pointer to record in macro space, if any     */
/*                                                                   */
/*********************************************************************/
static PMACRO does_exist(
  const char *name,                    /* name to search for         */
  PMACRO     *prev)
{
  PMACRO work = (PMACRO)0;             /* pointer to move thru list  */
  PMACRO temp;

  for (temp=NULL,work=(PMACRO)RX.macrobase; /* start at beginning of list */
       work &&                         /*   and, while still valid   */
         _stricmp(work->name,name);   /*   and not the correct one, */
       work=(temp=work)->next);        /*   move on to next entry    */
  if (prev) *prev=temp;                /* return previous, if wanted */
  return work;                         /* return pointer, if exists  */
}


/*********************************************************************/
/*                                                                   */
/*  Function Name:      saved_macro                                  */
/*                                                                   */
/*  Description:        scans a list of a saved macro space for a    */
/*                      named function                               */
/*                                                                   */
/*  Entry Point:        saved_macro(name,chain)                      */
/*                                                                   */
/*  Inputs:             name - name of function to find              */
/*                      chain - list of functions to search          */
/*                                                                   */
/*  Output:             YES if macro located, NO otherwise.          */
/*                                                                   */
/*********************************************************************/
static int saved_macro(
  const char *name,                    /* name to search for         */
  PMACRO      chain )                  /* list to search             */
{
  PMACRO old;

  for (; chain; old = chain, chain = chain->next)
  {
      if (!_stricmp(chain->name,name))  /* if there,                */
         return (YES);                 /*   return YES               */
  }
  return (NO);                         /* not found                  */
}


/*********************************************************************/
/*                                                                   */
/*  Function Name:      rxstrfrmfile                                 */
/*                                                                   */
/*  Description:        reads a RXSTRING from a given file handle    */
/*                                                                   */
/*  Entry Point:        rxstrfrmfile(file, r, size)                  */
/*                                                                   */
/*  Inputs:             file - file handle to read from              */
/*                      r    - RXSTRING to read into                 */
/*                      size - number of bytes to read               */
/*                                                                   */
/*  Output:             return code from read                        */
/*                      or out of memory code                        */
/*********************************************************************/
static ULONG rxstrfrmfile(
HFILE     file,
PRXSTRING r,
ULONG     size)
{
  ULONG  rc      = 0;
  r->SLN = size;
  if (size)                            /* if bytes to read           */
  {
      if (r->SPT = GlobalAlloc(GMEM_FIXED, size+1))   /* allocate a buffer of the   */
      {
          rc=file_read(file,r->SPT,size);  /* read the information       */
          if (rc) {GlobalFree(r->SPT); r->SPT = NULL;}
      }
      else rc = RXMACRO_NO_STORAGE;      /* error during allocation    */
  }
  return rc;                           /* return the error code      */
}

/*********************************************************************/
/*                                                                   */
/*  Function Name:      rxstrlen                                     */
/*                                                                   */
/*  Description:        finds the length of a RXSTRING               */
/*                                                                   */
/*  Entry Point:        rxstrlen(r)                                  */
/*                                                                   */
/*  Inputs:             r - RXSTRING to determine length from        */
/*                                                                   */
/*  Output:             length of the RXSTRING                       */
/*                                                                   */
/*********************************************************************/
static size_t rxstrlen(
  RXSTRING r )
{
  size_t len = 0;

  if (RXVALIDSTRING(r))
    len = r.SLN;                       /* get length of short string */
  return len;
}


/*********************************************************************/
/*                                                                   */
/*  Function Name:      file_write                                   */
/*                                                                   */
/*  Description:        Writes a buffer to a specified file.         */
/*                                                                   */
/*  Entry Point:        file_write(f,b,l)                            */
/*                                                                   */
/*  Inputs:             f - Open file handle                         */
/*                      b - buffer to write out                      */
/*                      l - length of buffer                         */
/*                                                                   */
/*  Output:             error code - 0 if everything is OK           */
/*                                   +ve is RXMACRO_* code           */
/*********************************************************************/
static ULONG file_write(
  HFILE  f,
  PVOID  b,
  ULONG  l )
{
  ULONG  count;
  WriteFile(f, b, l, &count, NULL);
  return( (count != l) ? RXMACRO_FILE_ERROR : RXMACRO_OK);
}


/*********************************************************************/
/*                                                                   */
/*  Function Name:      file_read                                    */
/*                                                                   */
/*  Description:        Reads  a buffer to a specified file.         */
/*                                                                   */
/*  Entry Point:        file_read(f,b,l)                             */
/*                                                                   */
/*  Inputs:             f - Open file handle                         */
/*                      b - buffer to read into                      */
/*                      l - length of buffer                         */
/*                                                                   */
/*  Output:             error code - 0 if everything is OK           */
/*                                   +ve is error code               */
/*********************************************************************/
static ULONG file_read(
  HFILE  f,
  PVOID  b,
  ULONG  l )
{
  ULONG  count;
  ReadFile(f, b, l, &count, NULL);
  return( (count != l) ? RXMACRO_FILE_ERROR : RXMACRO_OK);
}


/*********************************************************************/
/*                                                                   */
/*  Function Name:      ldmacro                                      */
/*                                                                   */
/*  Description:        Loads macro space from an open file.         */
/*                      It is assumed that the file is open and      */
/*                      the signature is valid. This function is     */
/*                      basically a continuation of RexxLoadMacro.   */
/*                      It's separated to make it readable and       */
/*                      maintainable.                                */
/*                                                                   */
/*  Entry Point:        ldmacro(ac,av,f)                             */
/*                                                                   */
/*  Inputs:             ac - Count of arguments                      */
/*                      av - Array of arguments                      */
/*                      f - Open file handle                         */
/*                                                                   */
/*  Output:             error code - 0 if everything is OK           */
/*                                   any RXMACRO_* error code        */
/*********************************************************************/
static int   ldmacro(
  size_t   ac,
  const char **av,
  HFILE    f )
{
  PMACRO tbase,wbase,w,t;
  size_t tbase_size;                   /* size of allocated temp stor*/
  ULONG  i;
  int    rc;
  RXSTRING p;
  PMACRO pOld = NULL;

  if (file_read(f, (PVOID)&RX.mcount,  /* read macro function count  */
                     sizeof(RX.mcount)))
    return(RXMACRO_FILE_ERROR);        /* there was file error       */

  if (!(tbase_size =                   /* calculate allocation size  */
      RX.mcount * MACROSIZE))
    return(RXMACRO_OK);                /* zero means nothing to read */

  tbase = GlobalAlloc(GMEM_FIXED, tbase_size);
  if (!tbase)
    return(RXMACRO_NO_STORAGE);        /* no memory available        */

  if (file_read(f, (void *)tbase, (ULONG)tbase_size))  /* read macro headers from fil*/
  {
      GlobalFree(tbase);                 /* free macro header list */
      return(RXMACRO_FILE_ERROR);        /* there was file error       */
  }

  for (i=1; i<RX.mcount; i++)            /* connect each element with the next in array */
      tbase[i-1].next = &tbase[i];
  tbase[RX.mcount-1].next = NULL;


  if (ac==0) {                  /* if we need to load all     */
      w = tbase;                         /* check for duplicates first */
      while (w) {                        /* go through the entire list */
          if (FillMacroComBlock(FALSE, w->name, NULL, 0, 0))    /* just put the name into com port */
              rc = (int)MySendMessage(RXAPI_MACRO,MACRO_QUERY,0);
          else return RXMACRO_NO_STORAGE;
          if (rc == RXMACRO_OK) {
              GlobalFree(tbase);                 /* free macro header list */
              return(RXMACRO_ALREADY_EXISTS);    /* no duplicates allowed      */
          }
          w = w->next;                     /* step to the next one       */
      }
  }
  else {                               /* given a real list to load  */
      for (i=0; i < ac; i++) {         /* check out all functions    */
          if (!saved_macro(av[i],tbase)) { /* if not in saved file       */
              GlobalFree(tbase);             /* free macro header list */
              return(RXMACRO_NOT_FOUND);     /* macro not there            */
          }
          if (FillMacroComBlock(FALSE, av[i], NULL, 0, 0))    /* just put the name into com port */
              rc = (int)MySendMessage(RXAPI_MACRO,MACRO_QUERY,0);
          else return RXMACRO_NO_STORAGE;
          if (rc == RXMACRO_OK) {
              GlobalFree(tbase);                 /* free macro header list */
              return(RXMACRO_ALREADY_EXISTS);    /* no duplicates allowed      */
          }
      }
  }

  rc = 0;
  wbase = NULL;
  for(w=tbase; w; w=w->next)
  {       /* read all macro functs...   */
      p = RXSTRING_EMPTY;                /* initialize to empty string */
      if (!ac || request(ac,av,w->name)) /* if all, or as requested..  */
          rc=rxstrfrmfile(f, &p, (ULONG)w->i_size);  /* read function from file    */
      else {
          w->image.strptr = NULL;
          if (SetFilePointer(f,(LONG)w->i_size,0,FILE_CURRENT)==0xFFFFFFFF)
              rc= RXMACRO_FILE_ERROR;          /* error changing file pointer*/
      }
      if (rc) break;                       /* for any error break loop   */
      w->image.strptr = p.strptr;         /* assign image to macro in list */
  }

  if (rc)   /* there was a trap so don't add to macro space but free list and images */
  {
      t = w;  /* keep macro that trapped */
      for(w=tbase; w, w != t; w=w->next)                    /* free images */
          if (w->image.strptr) GlobalFree(w->image.strptr);
      GlobalFree(tbase);        /* free the header list */
  }
  else
  {
      for(w=tbase; w; w=w->next)
      {
          if (w->image.strptr)
          {
              if (FillMacroComBlock(TRUE, w->name, w->image.strptr, w->i_size, w->srch_pos))
                  rc = (int)MySendMessage(RXAPI_MACRO,MACRO_ADD,0 /* don't update if exists */);
              else rc = RXMACRO_NO_STORAGE;
              GlobalFree(w->image.strptr);       /* free image */
          }
          if (rc) break;                       /* for any error break loop   */
      }
      GlobalFree(tbase);        /* free the header list */
  }

  return(rc);
}


/*********************************************************************/
/*                                                                   */
/*  Function Name:      macrofile_open                               */
/*                                                                   */
/*  Description:        Opens a file and checks its signature to     */
/*                      make sure it is a macro file. If a macro     */
/*                      file, the file pointer will be left at       */
/*                      macro function count value.                  */
/*                                                                   */
/*  Entry Point:        macrofile_open(fnam,fp)                      */
/*                                                                   */
/*  Inputs:             fnam - name of file to open                  */
/*                      fp - pointer in which open handle is returned*/
/*                                                                   */
/*  Output:             error code - 0 if everything is OK           */
/*                                   any RXMACRO_* error code        */
/*********************************************************************/
static int macrofile_open(
  const char *fnam,
  HFILE *fp )
{
  ULONG  i,rc = 0;
  CHAR buf[RXVERSIZE];
  if ((*fp = (HFILE)CreateFile(fnam, GENERIC_READ,
                0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0L)
            ) == (HFILE)INVALID_HANDLE_VALUE) /* open macro file */
     return(RXMACRO_FILE_ERROR);       /* there was file error       */

  if (file_read(*fp, (PVOID)buf,       /* read signature from file   */
                           RXVERSIZE)) /*                            */
    rc = RXMACRO_FILE_ERROR;           /* there was file error       */
  else if (memcmp(buf, RXVERSION,      /* if interpreter version     */
                  RXVERSIZE) )         /*   doesnt match current ver */
    rc = RXMACRO_SIGNATURE_ERROR;      /* the signatures didnot match*/
  else if (file_read(*fp, (PVOID)&i,   /* read signature from file   */
                     sizeof(i)))       /*                            */
    rc = RXMACRO_FILE_ERROR;           /* there was file error       */
  else if (i!=SIGNATURE)               /* if signatures dont match or*/
    rc = RXMACRO_SIGNATURE_ERROR;      /* the signatures didnot match*/
  if (rc) CloseHandle(*fp);
  return(rc);
}




/* functions needed to use communication port */

static RXQUEUE_TALK *
    FillMacroComBlock(BOOL add, const char *name, const char *data, size_t datalen, size_t spos)
{
    RXMACRO_TALK * icom;

    icom = LRX.comblock[API_MACRO];
    /* do we need to extend the communication block? */
    if (add && (datalen + 1 + sizeof(RXMACRO_TALK) > LRX.comblockMacro_ExtensionLevel * PAGE_SIZE))
    {
       if (MySendMessage(RXAPI_MACROCOMEXTEND,
                       (WPARAM)datalen + sizeof(RXMACRO_TALK) + 1,
                       (LPARAM)0)) return NULL;
       if (!CheckMacroComBlock()) return NULL;
       icom = LRX.comblock[API_MACRO];
    }

    strncpy(icom->name,name,NAMESIZE-1);     /* copy the macro name        */
    icom->srch_pos = spos;

    if (add)
    {
        if (datalen)
        {
            icom->image.strptr = (PUCHAR)(icom + 1);  /* set absolute pointer */
            memcpy(icom->image.strptr,data,datalen+1);
        }
        icom->image.strlength = datalen;
        icom->image.strptr = (PCHAR)sizeof(RXMACRO_TALK);   /* set relative pointer */
        icom->i_size = datalen;           /* size of image              */
    }
    return LRX.comblock[API_MACRO];
}


static BOOL ReceiveMacro(PMACRO element, ULONG kind)
{
    RXMACRO_TALK * intercom;
    if (kind != RECEIVE_ADD)
        intercom = (RXMACRO_TALK *) LRX.comblock[API_MACRO];
    else
        intercom = (RXMACRO_TALK *) RX.comblock[API_MACRO];

    strncpy(element->name,intercom->name,NAMESIZE-1);     /* copy the macro name        */
    if ((kind == RECEIVE_EXECUTE) || (kind == RECEIVE_IMAGELIST))
        element->image.strptr = GlobalAlloc(GMEM_FIXED, intercom->i_size + 1);
    if (kind != RECEIVE_HEADERLIST)
    {
        if (!element->image.strptr) return FALSE;
        memcpy(element->image.strptr, MACROIMAGE_ABS(intercom), intercom->i_size + 1);  /* copy image from com port */
    }
    element->srch_pos = intercom->srch_pos;
      element->image.strlength = intercom->i_size;
      element->i_size = intercom->i_size;
    return TRUE;
}


static void ReturnMacro(PMACRO element, BOOL withImage)
{
   RXMACRO_TALK * intercom;
   intercom = (RXMACRO_TALK *) RX.comblock[API_MACRO];

   if (withImage)
   {
       intercom->image.strptr = (PUCHAR)(intercom + 1);   /* absolute address */
       memcpy(intercom->image.strptr,element->image.strptr, element->i_size+1);
   } else
       strncpy(intercom->name, element->name,NAMESIZE-1);     /* copy the macro name        */

   intercom->srch_pos = element->srch_pos;
   intercom->image.strlength = element->i_size;
   intercom->i_size = element->i_size;
   intercom->image.strptr = (PCHAR) sizeof(RXMACRO_TALK);   /* return relative address */
}


/* Check whether or not local com block has same size as global com block */
static BOOL CheckMacroComBlock()
{
    if (!LRX.comblock[API_MACRO]) return FALSE;
    if (LRX.comblockMacro_ExtensionLevel != RX.comblockMacro_ExtensionLevel)
    {
        UnmapComBlock(API_MACRO);
        return MapComBlock(API_MACRO);
    }
    return LRX.comblock[API_MACRO] != 0;
}


/* functions needed to fill filter arguments for save into communication port */

static RXQUEUE_TALK *
    FillMacroComBlock_List(size_t argc, const char ** argv)
{
    RXMACRO_TALK * icom;
    icom = LRX.comblock[API_MACRO];

    icom->i_size = argc;   /* store number of args in i_size */

    if (argc)
    {
        size_t i,size = 0;
        char *ptr;
        for (i=0; i<argc;i++)
            size += strlen(argv[i])+1;

        if (size + sizeof(RXMACRO_TALK) > LRX.comblockMacro_ExtensionLevel * PAGE_SIZE)
        {
            if (MySendMessage(RXAPI_MACROCOMEXTEND,
                           (WPARAM)size + sizeof(RXMACRO_TALK),
                           (LPARAM)0)) return NULL;
            if (!CheckMacroComBlock()) return NULL;
            icom = LRX.comblock[API_MACRO];
        }
        ptr = (char *)(icom + 1);  /* set absolute pointer */

        for (i=0; i<argc;i++)
        {
            size_t sl = strlen(argv[i])+1;
            memcpy(ptr, argv[i],sl);
            ptr += sl;
        }
        icom->image.strlength = size;
        icom->image.strptr = (char *)sizeof(RXMACRO_TALK);   /* set relative pointer */
    }
    return LRX.comblock[API_MACRO];
}
