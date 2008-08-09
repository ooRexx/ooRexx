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
/*****************************************************************************/
/* REXX AIX Support                                             aixshmem.c   */
/*                                                                            */
/* Module for greating shared memory segments                                */
/*                                                                           */
/* Implemented via the System V IPC interface.                               */
/* Both, IBM AIX and Linux do support the System V IPC standard form AT&T,   */
/* so it should work the same way on both systems.      (weigold)            */
/*****************************************************************************/


#include <stdio.h>
#include "SharedMemorySupport.h"
#include <sys/shm.h>
#include <errno.h>

/******************************************************************************/
/* Name:       getshmem                                                       */
/*                                                                            */
/* Arguments:  key - the "name" of the shared memory segment                  */
/*             segsize - the size of the memory segment                       */
/*                                                                            */
/* Returned:  >0 - worked well, return the memory ID                          */
/*            -1 - already exitsts                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
int getshmem(key_t key, int segsize){

 int rc;
 rc=(shmget(key, segsize, IPC_CREAT|IPC_EXCL|432));
 //if((errno == ENOSPC) || (segsize>SHMMAX))
 if ( ( rc == -1 ) && ( errno != EEXIST ) )
   return (-2);                          /* system limit reached              */
 else
  return rc;
}


/******************************************************************************/
/* Name:       openshmem                                                      */
/*                                                                            */
/* Arguments:  key - the "name" of the shared memory segment                  */
/*             segsize - the size of the memory segment                       */
/*                                                                            */
/* Returned:  >0 - worked well, return the memory ID                          */
/*            -1 - error                                                      */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
int openshmem(key_t key, int segsize){

  return (shmget(key, segsize, 0));
}

/******************************************************************************/
/* Name:       attachshmem                                                    */
/*                                                                            */
/* Arguments:  shmid - ID of the shared memory segment                        */
/*                                                                            */
/* Returned:  >0 - worked well, return the segment pointer                    */
/*            -1 - error                                                      */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
char * attachshmem(int shmid){

#if defined(OPSYS_SUN)
  return ((char*)shmat(shmid, 0, SHM_SHARE_MMU));   /* explicit conversion    */
#else
  return ((char*)shmat(shmid, 0, 0));   /* explicit conversion                */
#endif
}

/******************************************************************************/
/* Name:       detachshmem                                                    */
/*                                                                            */
/* Arguments:  shmidptr - ptr to the shared memory segment                    */
/*                                                                            */
/* Returned:  none                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
void detachshmem(char *  shmptr){

  shmdt(shmptr);
}

/******************************************************************************/
/* Name:       removeshmem                                                    */
/*                                                                            */
/* Arguments:  shmid - ID of the shared memory segment                        */
/*                                                                            */
/* Returned:  none                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
void removeshmem(int shmid){

  shmctl(shmid, IPC_RMID, 0);
}

