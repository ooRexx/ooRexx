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
/*                                                                            */
/*  Module Name:   AIXMACRO.C                                                 */
/*                                                                            */
/*  Description:   This module handles all macro space API's                  */
/*                                                                            */
/*  Entry Points:  RxMacroChange()            add/change a func               */
/*                 RxMacroDrop()              delete a func                   */
/*                 RxMacroQuery()             check for a func                */
/*                 RxMacroReOrder()           change search position          */
/*                 RxMacroErase()             clear macro space               */
/*                 RxMacroSave()              save funcs to a file            */
/*                 RxMacroLoad()              load funcs from a file          */
/*                                                                            */
/*  IUO Entry:     RxMacroFunctionExecute()  retrieve function image          */
/*                                                                            */
/*  Notes:         Functions existing in the macro space cannot make          */
/*                 use of the REXX sourceline() function.  Any calls          */
/*                 to this function result in a 'Program is                   */
/*                 unreadable' error from the interpreter                     */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* Please note the following:                                                 */
/*                                                                            */
/* Functions in this module manipulate data that must be available            */
/* to all processes that call REXX API functions.  These processes            */
/* may invoke the REXX interpreter, or make direct calls to the               */
/* API routines.                                                              */
/*                                                                            */
/* In addition, functions in this module may establish data that              */
/* must persist after the calling process terminates.                         */
/******************************************************************************/

#define INCL_RXMACRO

#include <stdio.h>
#include "PlatformDefinitions.h"
#include SYSREXXSAA
#include <stdlib.h>
#include <string.h>
#include "APIDefinitions.h"
#include "RexxAPIManager.h"                  /* RexxxExecuteMacroFunction() */
#include "APIUtilities.h"
#include "SharedMemorySupport.h"
#include <sys/stat.h>
#include <unistd.h>

#if defined(HAVE_SYS_LDR_H)
# include <sys/ldr.h>
#endif

#include <dlfcn.h>

//#ifdef AIX
LONG   APIENTRY ApiRexxStart(          /* Prototype for internal start*/
         LONG ,                        /* Num of args passed to rexx  */
         PRXSTRING,                    /* Array of args passed to rex */
         PSZ,                          /* [d:][path] filename[.ext]   */
         PRXSTRING,                    /* Loc of rexx proc in memory  */
         PSZ,                          /* ASCIIZ initial environment. */
         LONG ,                        /* type (command,subrtn,funct) */
         PRXSYSEXIT,                   /* SysExit env. names &  codes */
         PSHORT,                       /* Ret code from if numeric    */
         PRXSTRING );                  /* Retvalue from the rexx proc */
//#endif

extern REXXAPIDATA  *apidata;          /* Global state data          */

#define SLN  strlength
#define SPT  strptr

typedef  LONG APIENTRY REXX (
         LONG ,                        /* Num of args passed to rexx */
         PRXSTRING,                    /* Array of args passed to rex*/
         PSZ,                          /* [d:][path] filename[.ext]  */
         PRXSTRING,                    /* Loc of rexx proc in memory */
         PSZ,                          /* ASCIIZ initial environment.*/
         LONG ,                        /* type (command,subrtn,funct)*/
         PRXSYSEXIT,                   /* SysExit env. names &  codes*/
         PSHORT,                       /* Ret code from if numeric   */
         PRXSTRING );                  /* Retvalue from the rexx proc*/

static RXSTRING RXSTRING_EMPTY = { 0L, (PCH)0 };

#define RXVERSION  "REXXSAA 4.00"      /* interpreter version str    */
#define RXVERSIZE  12                  /* size of RXVERSION str      */
#define SIGNATURE  0xddd5              /* macro space file marker    */


/*********************************************************************/
/*****        Macro Space Function List Access Functions         *****/
/*********************************************************************/

static ULONG                           /*                            */
  does_exist(PSZ,ULONG *);             /* see if a func exists       */
static ULONG                           /*                            */
  callrexx(PSZ,PMACRO);                /* call the REXXSAA interprtr */
static ULONG                           /*                            */
  file_read(FILE*,PVOID,ULONG);        /* read from an open file     */
static ULONG                           /*                            */
    makelst(ULONG,PSZ *,PULONG *);     /* build a macro list         */
static ULONG                           /*                            */
  request(ULONG,PSZ *,PSZ);            /* check a list for a string  */
static ULONG                           /*                            */
  file_write(FILE*,PVOID,ULONG);       /* write to an open file      */
static VOID                            /*                            */
  freelst(PULONG , ULONG);             /* free a list of macros      */
static ULONG                           /*                            */
  macrofile_open(PSZ, FILE **);        /* open a macro space file    */
static ULONG                           /*                            */
  ldmacro(ULONG,PSZ *,FILE*);          /* load macro space           */
static INT                             /*                            */
  saved_macro(PSZ,PMACRO);             /* see if a func exists       */
ULONG                                  /*                            */
  dup_list(PMACRO);                    /* copy a macro list          */
/*********************************************************************/
/*****              RXSTRING Manipulation Functions              *****/
/*********************************************************************/
static ULONG                           /*                            */
  rxstrfrmfile(FILE*,PRXSTRING,ULONG,  /* read a RXSTRING from file  */
             PMEMORYBASE);             /*                            */
static VOID                            /*                            */
  rxstrfree(ULONG,RXSTRING);           /* free a RXSTRING            */
                                       /*                            */
LONG                                   /*                            */
  rxstrdup(RXSTRING);                  /* strdup a RXSTRING          */
                                       /*                            */
static ULONG                           /*                            */
  rxstrlen(RXSTRING);                  /* strlen of a RXSTRING       */
                                       /*                            */
static VOID                            /*                            */
  rximagefree(ULONG, ULONG);           /* free a macro image         */
                                       /*                            */
static ULONG                           /*                            */
  rxstrtofile(FILE*,PCHAR,ULONG);      /* write a image to a file    */
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
ULONG  APIENTRY RexxAddMacro(
  PSZ    n,                            /* name of macro function     */
  PSZ    s,                            /* name of file               */
  ULONG  pos )                         /* search order pos request   */
{
  MACRO   w;                           /* temp struct to build entry */
  ULONG   p;                           /* offset to function struct  */
  ULONG   rc;                          /* return code from function  */
  ULONG   newheader, oldimage, oldimagesize;

  APISTARTUP(MACROCHAIN);              /* do common entry code       */


  if (pos != RXMACRO_SEARCH_BEFORE &&  /* if pos flag not before and */
       pos != RXMACRO_SEARCH_AFTER )   /*   not after, then          */
    rc = RXMACRO_INVALID_POSITION;     /*   return proper error code */
  else if (p=does_exist(n,NULL)) {     /* found name, so change it   */
    if ( !(rc = callrexx(s, &w))) {    /* call REXXSAA to get image  */
#ifdef ORXAP_DEBUG
  fprintf(stderr," %s:1 RexxAddMacro address of MDATA = %x.\n", __FILE__ ,
                                                               (MDATA(p)));
#endif
      oldimage = MDATA(p)->image;      /* remember the old image     */
      oldimagesize = MDATA(p)->i_size; /* and it's size              */
      MDATA(p)->image = w.image;       /* copy image string to list  */
      MDATA(p)->srch_pos = pos;        /* make sure proper flag      */
      MDATA(p)->i_size = w.i_size;     /* set the new size           */
                                       /* free old pcode+lit image   */
      rximagefree(oldimage, oldimagesize);
    }
  }                                    /* name not found, so add it  */
                                       /* allocate header storage    */
  else if (!RxAllocMem(&newheader, MACROSIZE, MACROMEM)) {
    p = newheader;
#ifdef ORXAP_DEBUG
  fprintf(stderr," %s:2 RexxAddMacro address of MDATA = %x.\n", __FILE__ ,
                                                               (MDATA(p)));
#endif
    strncpy(MDATA(p)->name,n,NAMESIZE-1);/* copy the macro name      */
    if (!(rc=callrexx(s, &w))) {       /* call REXXSAA to get image  */
      MDATA(p)->image = w.image;       /* copy the offset to  image  */
      MDATA(p)->i_size = w.i_size;     /* set the new size           */
      MDATA(p)->srch_pos = pos;        /* make sure proper flag      */
      MDATA(p)->next=apidata->mbase;   /* chain it in                */
      apidata->mbase=newheader;        /* at head of chain           */
      apidata->macrocount++;           /* count it                   */
    }                                  /*                            */
                                       /* free mem if REXXSAA failed */
    else
    {
       RxFreeMem(newheader,MACROSIZE,MACROMEM);
       /*  If the last macro was removed, remove shm as well @MIC002A */
       if(apidata->macrocount <= 1)       /* if the chain is empty   */
       {
         removeshmem(apidata->mbasememId);/* remove the macro memory */
         detachshmem(apidata->macrobase); /* force the deletion      */
         apidata->macrobase = NULL;       /* reset memory pointer    */
         apidata->macrocount = NULL;      /* reset macro counter     */
       }
     }
  }                                    /*                            */
  else rc=RXMACRO_NO_STORAGE;          /* no storage avail error     */
#ifdef ORXAP_DEBUG
  fprintf(stderr," %s:3 RexxAddMacro address of MDATA = %x.\n", __FILE__ ,
                                                               (MDATA(p)));
#endif
  APICLEANUP(MACROCHAIN);              /* release shared resources   */
  return (rc);                         /* and exit with return code  */
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

APIRET APIENTRY RexxDropMacro(
  PSZ n )                              /* name of macro to delete    */
{
  ULONG p;                             /* pointer to function struct */
  ULONG t;                             /* temp pointer to macro      */
  ULONG  rc;                           /* return code from function  */
                                       /* Exception handler record   */

  p  = 0;                              /* pointer to function struct */
  t  = 0;                              /* temp pointer to macro      */

  APISTARTUP(MACROCHAIN);              /* do common entry code       */

    if (p=does_exist(n,&t)) {          /* found name, so delete it   */
      if (t)
        MDATA(t)->next = MDATA(p)->next;/* if previous, go around it */

      else apidata->mbase = MDATA(p)->next;/* else first, so fix base*/
                                       /* free this block's string   */
      rximagefree(MDATA(p)->image, MDATA(p)->i_size);
                                       /* free this block's storage  */
    RxFreeMem(((ULONG)(((char*)MDATA(p))-(apidata->macrobase))),
                                           MACROSIZE, MACROMEM);
    (apidata->macrocount)--;           /* decrement macro counter    */
    if(apidata->macrocount == 0){      /* if now the chain is empty  */
      removeshmem(apidata->mbasememId);/* remove the macro memory    */
      detachshmem(apidata->macrobase); /* force the deletion         */
      apidata->macrobase = NULL;       /* reset memory pointer       */
    }
    rc = RXMACRO_OK;                   /* set successful return code */
    }                                  /* end of "if p..."           */
  else rc = RXMACRO_NOT_FOUND;         /* name not found, so error   */
  APICLEANUP(MACROCHAIN);              /* release shared resources   */
  return (rc);                         /* and exit with return code  */
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

APIRET APIENTRY RexxClearMacroSpace(void)
{
  ULONG  rc;                           /* return code from function  */

  APISTARTUP(MACROCHAIN);              /* do common entry code       */

                                       /* now try to get macro base  */

  if (apidata->macrobase) {            /* if macro space exists...   */
      removeshmem(apidata->mbasememId);/* remove the macro memory    */
      detachshmem(apidata->macrobase); /* force the deletion         */
      apidata->macrobase = NULL;       /* reset memory pointer       */
      apidata->mbase = NULL;           /* reset the anchor           */
      apidata->macrocount = NULL;      /* reset function counter     */
       rc = RXMACRO_OK;                /* set return code to success */
  }                                    /* end of "for..." loop       */
  else rc = RXMACRO_NOT_FOUND;         /* otherwise macro not found  */
  APICLEANUP(MACROCHAIN);              /* release shared resources   */
  return (rc);                         /* and exit with return code  */
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

APIRET APIENTRY RexxSaveMacroSpace(
  ULONG    ac,                         /* count of arguments         */
  PSZ     *av,                         /* argument list              */
  PSZ      fnam )                      /* file name                  */
{
  FILE     *f;                         /* file handle                */
  PULONG    list;                      /* list of functions to save  */
  PMACRO    p;                         /* temp pointer to struct     */
  ULONG     i;                         /* miscellaneous ULONG s      */
  ULONG     rc;                        /* return code from function  */
                                       /* Exception handler record   */

  list = NULL;                         /* list of functions to save  */
  p    = NULL;                         /* temp pointer to struct     */
  i    = (ULONG )0;                    /* miscellaneous ULONG s      */
  rc   = (ULONG )0;                    /* return code from function  */
  if (!av) ac = (ULONG )0;             /* check for no list          */


  APISTARTUP(MACROCHAIN);              /* do common entry code       */

  if (!makelst(ac,av,&list))           /* if makelst fails...        */
    rc = RXMACRO_NO_STORAGE;           /* no memory for list         */
  else if (!apidata->mcount)           /* if makelst created nul list*/
    rc = RXMACRO_NOT_FOUND;            /* macros requested not found */
  else if((f = fopen(fnam,"w+")) == NULL)
    rc = RXMACRO_FILE_ERROR;           /* was some sort of file error*/
  else {                               /* otherwise ready to write.. */
    rc = file_write(f,(PVOID)RXVERSION,/* write version to the file  */
                    RXVERSIZE);        /*                            */
    if (!rc) {                         /* if file write was ok...    */
      i = SIGNATURE;                   /* get internal file flag     */
      rc = file_write(f, (PVOID)&i,    /* write flag to the file     */
                      sizeof(i));      /*                            */
    }                                  /*                            */
    if (!rc)                           /* if file write was ok...    */
      rc = file_write(f,               /* write func count to file   */
           (PVOID)&(apidata->mcount),sizeof(apidata->mcount));
                                       /* for each entry in tmp list */
    for(i=0; !rc && i < apidata->mcount; i++)
                                       /* write the macro function   */
                                       /*   headers to the file      */
      rc = file_write(f,(PVOID)(apidata->macrobase+list[i]),
                      MACROSIZE);
                                       /* for each entry in tmp list */
    for(i=0; !rc && i < apidata->mcount; i++)
                                       /* write macros funcs to file */
      rc=rxstrtofile(f,(apidata->macrobase+(MDATA(list[i])->image))
                 , MDATA(list[i])->i_size);
                                       /* close the file fails or    */
    if (fclose(f) || rc) {             /* there was some write error */
      remove(fnam);                    /* delete the corrupted file  */
      rc = RXMACRO_FILE_ERROR;         /* set rc to file error       */
    }                                  /*                            */
    else rc = RXMACRO_OK;              /* set rc to success          */
  }                                    /* end of "ready to write.."  */
  freelst(list,ac);                    /* free temp list             */
  APICLEANUP(MACROCHAIN);              /* release shared resources   */
  return (rc);                         /* and exit with return code  */
}


/*$PE*/
/*$FD.RexxLoadMacroSpace */
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

APIRET APIENTRY RexxLoadMacroSpace(
  ULONG    ac,                         /* argument count             */
  PSZ     *av,                         /* list of argument strings   */
  PSZ      fnam )                      /* file name to load functs   */
{
  FILE     *f;                         /* file handle                */
  ULONG     rc ;                       /* return code from function  */
                                       /* Exception handler record   */

  APISTARTUP(MACROCHAIN);              /* do common entry code       */

  if (!(rc = macrofile_open(fnam,&f))){/* if no error opening file   */
    if (!av) ac = (ULONG )0;           /* check for no list          */
    rc = ldmacro(ac,av,f);             /* do remainder of load macro */
    if (fclose(f) && !rc)            /* close the file             */
      rc = RXMACRO_FILE_ERROR;         /* error trying to close file */
  }
  APICLEANUP(MACROCHAIN);              /* release shared resources   */
  return (rc);                         /* and exit with return code  */
  }                                    /* end of RxMacroLoad()       */


/*$PE*/
/*$FD.RexxQueryMacro*/
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

APIRET APIENTRY RexxQueryMacro(
  PSZ     name,                        /* name to search for         */
  PUSHORT pos )                        /* pointer for return of pos  */
{
  ULONG  rc;                           /* return code from call      */
  ULONG tmp;                           /* temp pointer to record     */

  APISTARTUP(MACROCHAIN);              /* do common entry code       */

  if(tmp=does_exist(name,NULL)){       /* if macro exists, then      */
    *pos = (USHORT)MDATA(tmp)->srch_pos;/*   return position and     */
    rc = RXMACRO_OK;                   /*   set return to successful */
  }                                    /*                            */
  else rc = RXMACRO_NOT_FOUND;         /* set return code to failure */
  APICLEANUP(MACROCHAIN);              /* release shared resources   */
  return (rc);                         /* and exit with return code  */
}


/*$PE*/
/*$FD.RexxReorderMacro*/
/*********************************************************************/
/*                                                                   */
/*  Function Name:      RxReorderMacro                              */
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

APIRET APIENTRY RexxReorderMacro(
  PSZ    name,                         /* name of function to change */
  ULONG  pos )                         /* new position for function  */
{
  ULONG  rc;                           /* return code from call      */
  ULONG tmp;                           /* temp pointer to record     */

  APISTARTUP(MACROCHAIN);              /* do common entry code       */

  if ( pos != RXMACRO_SEARCH_BEFORE && /* if pos flag not before and */
       pos != RXMACRO_SEARCH_AFTER )   /*   not after, then          */
    rc = RXMACRO_INVALID_POSITION;     /*   return proper error code */
  else
    if (tmp=does_exist(name,NULL)) { /* if macro exists, then      */
      MDATA(tmp)->srch_pos = pos;      /*   set search order pos and */
      rc = RXMACRO_OK;                 /*   set return to successful */
    }                                  /* end of "if exists..."      */
  else rc = RXMACRO_NOT_FOUND;         /* set return code to failure */
  APICLEANUP(MACROCHAIN);              /* release shared resources   */
  return (rc);                         /* and exit with return code  */
}

/*$PE*/
/*$FD.RexxExecuteMacroFunction*/
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

APIRET APIENTRY RexxExecuteMacroFunction(
  PSZ       name,                      /* name of func to find       */
  PRXSTRING p )                        /* storage for image return   */
{
  ULONG tmp;                           /* temp macro pointer         */
  ULONG  rc;                           /* return code from function  */

  if(tmp=does_exist(name,NULL)){       /* if name exists in list...  */
                                       /* copy string and            */
    (*p).strptr = (apidata->macrobase+(MDATA(tmp)->image));
    (*p).strlength = MDATA(tmp)->i_size;/* it's size                */
    rc=RXMACRO_OK;                     /* set successful return code */
    }                                  /* end of "if exists..."      */
  else rc=RXMACRO_NOT_FOUND;           /* name not found, so error   */
  return (rc);                         /* and exit with return code  */
}


/*********************************************************************/
/*********************************************************************/


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
/*  Output:             offset to record in macro space, if any      */
/*                                                                   */
/*********************************************************************/
ULONG does_exist(
  PSZ         name,                    /* name to search for         */
  ULONG     *prev )                    /*                            */
{
  ULONG work = 0;                      /* pointer to move thru list  */
  ULONG temp = 0;                      /*                            */

  for (temp=NULL, work=apidata->mbase; /* start at beginning of list */
       work &&                         /*   and, while still valid   */
                                       /*   and not the correct one, */
         rxstricmp(MDATA(work)->name,name);
       work=MDATA((temp=work))->next);   /*   move on to next entry    */
  if (prev) *prev=temp;                /* return previous, if wanted */
  return work;                         /* return pointer, if exists  */
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
static ULONG callrexx(
  PSZ    fnam,
  PMACRO current )
{
  ULONG fsize = NULL;
  FILE      *f;
  RXSTRING   av;
  RXSTRING   m[2];
  ULONG       i;
  ULONG      rc;
  REXX       *addr;
  PVOID      plib;
  SHORT      retval;
  struct stat       finfo;

  if (!(f = fopen(fnam,"rb")))         /* open the source file       */
    return(RXMACRO_SOURCE_NOT_FOUND);

  m[0] = RXSTRING_EMPTY;               /* empty string for read check*/
  m[1] = RXSTRING_EMPTY;               /* empty string for rtn image */
  rc = RXMACRO_OK;                     /* find the file size         */
#if defined( HAVE_FSTAT )
  if(fstat((fileno(f)), &finfo) == -1)
#else  /* AIX */
  if(stat(fnam,&finfo) == -1)
#endif
    rc = RXMACRO_FILE_ERROR;           /* error getting file info    */
  else {
    fsize = finfo.st_size;             /* read the file size         */
    rc = rxstrfrmfile(f,               /* read the source file into  */
                     &(m[0]),          /*   a RXSTRING, passing in   */
                     fsize,            /*   the byte count to read   */
                     &apidata->memory_base);
  }
  if (fclose(f) && !rc)                /* close source file          */
    rc = RXMACRO_FILE_ERROR;           /* say error if no other error*/

  if (rc) {                            /* if any errors upto now...  */
    rxstrfree(YES,m[0]);               /* free source code           */
    return(rc);                        /* and return the error code  */
  }

  av.SLN = 3;                          /*   for call to REXXSAA()    */
  av.SPT = "//T";                      /* set parm for tokenize only */
  i = 0;                               /* initialize error code      */
//#ifdef AIX
  if (ApiRexxStart(1,&av,(PSZ)fnam,m,  /* call REXXSAA interpreter   */
                NULL,0,(PRXSYSEXIT)0,  /* to get a tokenized image   */
                &retval,NULL) )        /* of the source file         */
        rc = RXMACRO_SOURCE_NOT_FOUND; /* file error - REXX problm   */
//#else
//if ((plib = dlopen("librexx.so.1",RTLD_LAZY)) != NULL)
//{
//   addr =(REXX*)dlsym(plib, "RexxStart"); /* get func address      */
//   if ((*addr)(1,&av,(PSZ)fnam,m,    /* call REXXSAA interpreter   */
//              NULL,0,(PRXSYSEXIT)0,  /*   to get a tokenized image */
//              &retval,NULL) )        /*   of the source file       */
//      rc = RXMACRO_SOURCE_NOT_FOUND; /* file error - REXXSAA prblm */
//   dlclose(plib);                    /* free REXX DLL              */
//}
//else
//   rc = RXMACRO_FILE_ERROR;
//#endif
  rxstrfree(NO, m[0]);                /* free the source RXSTRING   */
                                       /* copy string to macro space */
  current->image = rxstrdup(m[1]);
  if (rc == -1)                        /* if no error yet but error  */
                                       /* trying to strdup...        */
    rc = RXMACRO_NO_STORAGE;           /* then report the error      */
  current->i_size= rxstrlen(m[1]);     /* get string length          */
  rxstrfree(NO, m[1]);                 /* free image from REXXSAA    */
  return rc;                           /*                            */
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
/*                      memId- ID of the shared memory area          */
/*                                                                   */
/*  Output:             return code from read                        */
/*                      or out of memory code                        */
/*********************************************************************/
static ULONG rxstrfrmfile(
FILE     *file,
PRXSTRING r,
ULONG     size,
PMEMORYBASE  memory_pool )             /* Memory pool for the read   */
{
  ULONG  rc      = 0;
  key_t key;                           /* key for the memory segment */

  r->SLN = size;
  if (size)                            /* if bytes to read           */
    if ((r->SPT = (char*)malloc((size_t)size)) != NULL )
      rc=file_read(file,(void*)(r->SPT),size); /*read the information*/
    else rc = RXMACRO_NO_STORAGE;      /* error during allocation    */
  return rc;                           /* return the error code      */
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
  FILE  *f,
  PVOID  b,
  ULONG  l )
{
   ULONG  rc;
                                       /* if read from disk is ok ...*/
  if ((rc = fread(b, 1, l, f)))
    if (rc != l)                       /* but not all read ...       */
      return (1);                      /* then force a read error.   */
  return (0);
}

/*********************************************************************/
/*                                                                   */
/*  Function Name:      rxstrfree                                    */
/*                                                                   */
/*  Description:        frees the storage of a RXSTRING              */
/*                                                                   */
/*  Entry Point:        rxstrfree(s,r)                               */
/*                                                                   */
/*  Inputs:             s - ULONG  tell API manager to free          */
/*                      r - RXSTRING to free                         */
/*                                                                   */
/*  Output:             (none)                                       */
/*                                                                   */
/*********************************************************************/
static VOID rxstrfree(
  ULONG    s,
  RXSTRING r)
{
  if (s) {                             /* if our memory              */
    if (RXVALIDSTRING(r)) {
                                       /* free the memory            */
      RxFreeMem(((ULONG)((r.SPT) - (apidata->macrobase))),r.SLN,
                MACROMEM);
    }

  }
  else                                 /* rxstring from RexxStart    */
    free(r.SPT);                       /* free directly              */
}

/*********************************************************************/
/*                                                                   */
/*  Function Name:      rxstrdup                                     */
/*                                                                   */
/*  Description:        copies a RXSTRING structure and data         */
/*                                                                   */
/*  Entry Point:        rxstrdup(r)                                  */
/*                                                                   */
/*  Inputs:             r - RXSTRING to copy                         */
/*                                                                   */
/*  Output:             copy of the input RXSTRING                   */
/*                                                                   */
/*********************************************************************/
LONG rxstrdup(
  RXSTRING r)
{
  ULONG offset;

  if (RXVALIDSTRING(r)) {
    if (!RxAllocMem(&offset,          /* allocate memory            */
        r.SLN, MACROMEM))
                                      /* copy the data over         */
      memcpy(apidata->macrobase+offset,r.SPT,r.SLN);
    else return (-1);                 /* memory failure             */
  }
  return offset;
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
static ULONG rxstrlen(
  RXSTRING r )
{
  ULONG len;

  len = 0;                             /* initialize zero length     */
  if (RXVALIDSTRING(r))
    len = r.SLN;                       /* get length of short string */
  return len;
}

/*********************************************************************/
/*                                                                   */
/*  Function Name:      rximagefree                                  */
/*                                                                   */
/*  Description:        frees the storage of a RXSTRING image        */
/*                                                                   */
/*  Entry Point:        rximagefree(r)                               */
/*                                                                   */
/*  Inputs:             memory ID - RXSTRING to free                 */
/*                                                                   */
/*  Output:             (none)                                       */
/*                                                                   */
/*********************************************************************/
static VOID rximagefree(
  ULONG offset,                    /* point in memory                */
  ULONG size)                      /* size of the image              */
{
      RxFreeMem(offset, size, MACROMEM);/* free the memory           */
}


/*********************************************************************/
/*                                                                   */
/*  Function Name:      makelst                                      */
/*                                                                   */
/*  Description:        Select macros which have a match in set of   */
/*                      arguments. Add the pointer to that macro to  */
/*                      a temporary list which is then returned to   */
/*                      the caller. The temporary list is allocated  */
/*                      to allow for maximum possible matches. This  */
/*                      number is argcount unless argcount is zero   */
/*                      in which case the number is macrocount.      */
/*                                                                   */
/*  Entry Point:        makelst(argc, argv, list)                    */
/*                                                                   */
/*  Inputs:             argc - count of arguments in list            */
/*                      argv - list of ASCIIZ strings to be searched */
/*                      list - Address of a pointer to macros.       */
/*                                                                   */
/*  Output:             YES if list was created NO otherwise. Note   */
/*                      that a list of 0 elements is valid.          */
/*********************************************************************/
static ULONG makelst(
  ULONG    argc,                       /* count of argument strings  */
  PSZ     *argv,                       /* list of argument strings   */
  PULONG *list )                       /* list of macro ptrs returnd */
{
  ULONG t;                             /* pointer to loop thru list  */
  ULONG  i;                            /*                            */
                                       /*                            */
  apidata->mcount = 0;                 /* init count to zero         */
                                       /* if no args or too many args*/
  if (!(i = argc) || i > apidata->macrocount)
    i = apidata->macrocount;           /* use maximum macro count    */
                                       /*                            */
  if (!i)                              /* if no macro to write say   */
    return(YES);                       /* everything is ok. 0 mcount */

//#ifndef LINUX
  *list=(PULONG)malloc(i*sizeof(ULONG));
//#else
//  (PVOID)*list=malloc(i*sizeof(ULONG));
//#endif

  if(*list == NULL)
    return(NO);                        /* no memory so return NO     */
                                       /*                            */
                                       /*                            */
                                       /* move through list of macros*/
  for (t=apidata->mbase; t; t=MDATA(t)->next) {
    if ( !argc ||                      /* if no arguments, or if     */
                                       /*   the strings match, and   */
         request(argc,argv,MDATA(t)->name))
      (*list)[apidata->mcount++] = t;  /* save match in our list     */
    }                                  /* end of "for..." loop       */
  return YES;                          /* return pointer to list     */
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
static ULONG request(
  ULONG    argc,                       /* count of argument strings  */
  PSZ     *argv,                       /* list of argument strings   */
  PSZ      name )                      /* string name to search for  */
{
  ULONG  i;                            /* counter to loop thru args  */
  ULONG  rc;                           /* flag for return value      */
                                       /*                            */
  rc = NO;                             /* initialize return to NO    */
  for (i = 0; i < argc; i++ ) {        /* move through list of args  */
    if (!rxstricmp(name, argv[i]))     /* if the strings match, then */
      rc = YES;                        /*   set return flag to YES   */
    }                                  /* end of "for..." loop       */
  return rc;                           /* return YES or NO flag      */
}


/*$PE*/
/*$FD.file_write*/
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
  FILE  *f,
  PVOID  b,
  ULONG  l )
{

   ULONG  rc;
                                       /* if write to disk is ok ... */
  if ((rc = fwrite(b, 1, l, f)))
    if (rc != l)                       /* but not all read ...       */
      return (1);                      /* then force a read error.   */
  return (0);
}


/*$PE*/
/*$FD.rxstrtofile*/
/*********************************************************************/
/*                                                                   */
/*  Function Name:      rxstrtofile                                  */
/*                                                                   */
/*  Description:        writes a RXSTRING to a given file handle     */
/*                                                                   */
/*  Entry Point:        rxstrtofile(file, r)                         */
/*                                                                   */
/*  Inputs:             file - file handle to write to               */
/*                      r    - RXSTRING to write                     */
/*                                                                   */
/*  Output:             Error code from file_write() (see below)     */
/*                                                                   */
/*********************************************************************/
static ULONG rxstrtofile(
  FILE    *file,                       /* handle to open file        */
  PCHAR    r ,                         /* image to be written        */
  ULONG    size)                       /* size of the image          */
{
  ULONG  rc    =        0;

  rc = file_write(file, r, size); /* write out the information  */
  return(rc);
}

/*$PE*/
/*$FD.freelst*/
/*********************************************************************/
/*                                                                   */
/*  Function Name:      freelst                                      */
/*                                                                   */
/*  Description:        free a temporary list of macro space entries */
/*                                                                   */
/*  Entry Point:        freelst(ptr)                                 */
/*                                                                   */
/*  Inputs:             ptr - pointer to start of temporary list     */
/*                                                                   */
/*  Output:             (none)                                       */
/*                                                                   */
/*********************************************************************/
static VOID freelst(
  PULONG  ptr,                         /* pointer to list start      */
  ULONG   count )                      /* size of list to free       */
{
  free((void*)ptr);
  return;                              /* return to caller           */
}


/*$PE*/
/*$FD.macrofile_open*/
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
static ULONG macrofile_open(
  PSZ    fnam,
  FILE **fp )
{
  ULONG  i,rc = 0;
  CHAR buf[RXVERSIZE];


  if((*fp = fopen(fnam,"r")) == NULL)  /* open the file for read     */
    return(RXMACRO_FILE_ERROR);        /* there was file error       */

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

  if (rc) fclose(*fp);                 /* close file on any error    */
  return(rc);
}



/*$PE*/
/*$FD.ldmacro*/
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
static ULONG ldmacro(
  ULONG    ac,
  PSZ     *av,
  FILE    *f )
{
  PMACRO tbase,wbase,w;
  ULONG  tbase_size;                   /* size of allocated temp stor*/
  ULONG  rc,i;
  RXSTRING p;
  ULONG tmp;

                                       /* read macro function count  */
  if (file_read(f, (PVOID)&(apidata->mcount),
                     sizeof(apidata->mcount)))
    return(RXMACRO_FILE_ERROR);        /* there was file error       */

  if (!(tbase_size =                   /* calculate allocation size  */
      (apidata->mcount) * MACROSIZE))
    return(RXMACRO_OK);                /* zero means nothing to read */
                                       /* try to allocate temp memory*/
  tbase = (PMACRO) malloc(tbase_size);
  if (tbase == NULL)
    return(RXMACRO_NO_STORAGE);        /* no memory available        */

  if (file_read(f, (PVOID)tbase,       /*read macro headers from file*/
                tbase_size)) {
    free((PVOID)tbase);                /* free temp list buffer      */
    return(RXMACRO_FILE_ERROR);        /* there was file error       */
  }

  for (i=0; i < (apidata->mcount); i++) {
    tbase[i].image = NULL;             /* make sure there is no garbg*/
    tbase[i].temp_buf = RXSTRING_EMPTY;
  }

  if (ac==0) {                         /* if we need to load all     */
    w = tbase;                         /* check for duplicates first */
    for (i=0;i<(apidata->mcount);i++) {/* go through the entire list */
      if (does_exist(w->name,NULL)) {  /* if already loaded          */
        free((PVOID)tbase);            /* free temp list buffer      */
        return(RXMACRO_ALREADY_EXISTS);/* no duplicates allowed      */
      }
      w++;                             /* step to the next one       */
    }
  }
  else {                               /* given a real list to load  */
    for (i=0; i < ac; i++) {           /* check out all functions    */
      if (!saved_macro(av[i],tbase)) { /* if not in saved file       */
        free((PVOID)tbase);            /* free temp list buffer      */
        return(RXMACRO_NOT_FOUND);     /* macro not there            */
      }
      if (does_exist(av[i],NULL)) {    /* if already in list         */
        free((PVOID)tbase);            /* free temp list buffer      */
        return(RXMACRO_ALREADY_EXISTS);/* no duplicates allowed      */
      }
    }
  }


  rc = 0;
  w = tbase;                           /* copy ptr to temp list      */
  for(i=0;i<(apidata->mcount);i++){    /* read all macro functs...   */
    p = RXSTRING_EMPTY;                /* initialize to empty string */
    if (!ac || request(ac,av,w->name)) /* if all, or as requested..  */
      rc=rxstrfrmfile(f,&p,w->i_size,  /* read function from file    */
                      &(apidata->macro_base));
    else if (fseek(f,                  /* else skip over string      */
                     w->i_size,        /*                            */
                           SEEK_CUR))  /*                            */
      rc= RXMACRO_FILE_ERROR;          /* error changing file pointer*/
    if (rc) break;                     /* for any error break loop   */
    w->temp_buf = p;                   /* add to temp buffer         */
    w++;                    /* step to the next one       */
  }                                    /* end of "read all funcs..." */

  if (!rc) {                           /* if the read was clean ...  */
    if (!(dup_list(tbase)))            /* add list to macrobase      */
      rc = RXMACRO_NO_STORAGE;         /* no more storage in macrobas*/
    else {                             /* if something was added...  */
      apidata->macrocount += apidata->mcount;/* increment counter    */
      rc = RXMACRO_OK;                 /* set successful return code */
    }                                  /* end "if something was.."   */
  }

  free(tbase);                         /* free temp list buffer      */
  return(rc);
}


/*$PE*/
/*$FD.saved_macro*/
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
static INT saved_macro(
  PSZ         name,                    /* name to search for         */
  PMACRO      chain )                  /* list to search             */
{
                                       /* start at beginning of list */
  for (int i=0;i<(apidata->mcount);i++) {
                                       /*   and, while still valid   */
    if (!rxstricmp(chain->name,name))  /* if there,                  */
      return (YES);                    /*   return YES               */
    else
      chain++;                         /* let's point to the next one*/
  }
  return (NO);                         /* not found                  */
}


/*$PE*/
/*$FD.dup_list*/
/*********************************************************************/
/*                                                                   */
/*  Function Name:      dup_list                                     */
/*                                                                   */
/*  Description:        copies a list of macro space entries         */
/*                                                                   */
/*  Entry Point:        dup_list(old)                                */
/*                                                                   */
/*  Inputs:             old - pointer to start of old list           */
/*                                                                   */
/*  Output:             Return code;1 on success!                    */
/*                                                                   */
/*********************************************************************/
ULONG dup_list(
  PMACRO old )
{
  ULONG t      = 0;                    /* pointer to loop thru list  */
  ULONG image;                         /* offset to the image space  */
  ULONG retval = 0;                    /* pointer for return value   */
  INT new_mcount = 0, add = 0;
  PMACRO loop;
  ULONG del;

  /* calculate how much macros to add                                 */
  loop = old;
  for (int i=0;i<(apidata->mcount);i++){ /*scan old list               */
    if((loop->temp_buf).strlength != 0)  /* if there is a image        */
      add++;                             /*  count this                 */
    loop++;                              /* step to the next one        */
  }

  for (int j=0;j<(apidata->mcount);j++){     /* scan old list...   */
    if((old->temp_buf).strlength != 0){ /* if we should copy this  */
      if (!RxAllocMem(&t,               /* if storage can be created  */
              MACROSIZE,
              MACROMEM)
          && !RxAllocMem(&image,
                    old->i_size,
                        MACROMEM)) {
                                        /*   in the right mem, then...*/
                                        /* copy old information       */
        memcpy((apidata->macrobase+t),old,MACROSIZE);
        MDATA(t)->image = image;        /* remember the image location*/
        MDATA(t)->i_size = (old->temp_buf).strlength;/* and the size  */
                                        /* copy the image             */
        memcpy((apidata->macrobase+image), (old->temp_buf).strptr,
                                       (old->temp_buf).strlength);
        MDATA(t)->next = apidata->mbase;/* chain it in                */
        apidata->mbase = t;             /* at front of chain          */

        new_mcount++;                   /* count this                 */
        old++;                          /* step to the next one       */
      }                                 /*                            */
      else                              /* if no more memory...       */
        break;                          /* then break out of loop     */
                                        /*                            */
    }
    else old++;                         /* take the next one          */
  }



                                        /*                            */
  if (new_mcount != add) {              /* if we stopped half way..   */
                                  /* free up all that we have added   */
    for(int i=0;i<add;i++){
      del = apidata->mbase;             /* remember this to delete    */
      apidata->mbase = MDATA(apidata->mbase)->next;/* chain it out    */
                                        /* free the image             */
      RxFreeMem(MDATA(del)->image,MDATA(del)->i_size, MACROMEM);
      RxFreeMem(del,MACROSIZE,MACROMEM);/* free the header */
    }
    apidata->mcount = NULL;            /* nothing was added          */
    return (0);                        /* error condition            */
  }                                    /* end "if we stopped..."     */
  apidata->mcount = new_mcount;        /* remember how many added    */
  return (1);                          /* return ok    */
}

