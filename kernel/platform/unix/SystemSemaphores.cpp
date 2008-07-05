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
/* REXX AIX Support                                             aixsem.c      */
/*                                                                            */
/* Module for greating kernel semaphor sets                                   */
/*                                                                            */
/* Implemented via the System V IPC interface.                                */
/******************************************************************************/
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <pthread.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#if defined( HAVE_SYS_SEM_H )
# include <sys/sem.h>
#endif

#include <sys/time.h>
#include "RexxCore.h"
#include "SystemSemaphores.h"
#include <errno.h>

#if defined( HAVE_SCHED_H )
# include <sched.h>
#endif

extern int iSemShmMode;                        /* Mode from startup          */

#if !defined( HAVE_UNION_SEMUN )
union semun {
  int val;
  struct semid_ds *buf;
  unsigned short *array;
};
#endif
/******************************************************************************/
/* Name:       createsem                                                      */
/*                                                                            */
/* Arguments:  sid - place for the semaphore identifier                       */
/*             key - the "name" of the semaphore                              */
/*             numbers - how many semaphores to create in this set            */
/*                                                                            */
/* Returned:   0 - worked well                                                */
/*            -1 - already exists                                             */
/*             1 - error                                                      */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
int createsem(int *sid, key_t key, int members){

  int cntr;
  union semun semopts;

  if(members > SEMMSL)      /* SEMMSL is max number of semaphores per set     */
    return (1);             /* return error                                   */
                            /* try to get a semaphore with rw-rights for the  */
                            /* owner only(0600).                              */
  if ((*sid = semget(key, members, IPC_CREAT|IPC_EXCL|iSemShmMode)) == -1) {
    if(errno == EEXIST)
      return (-1);          /* already exists                                 */
    return (1);             /* some other error                               */
  }
  semopts.val = 1;          /* initial value for all semaphores               */
                            /* do the initialisation                          */
  for(cntr = 0;cntr < members;cntr++)
    semctl(*sid, cntr, SETVAL, semopts);
  return (0);               /* worked well                                    */
}



/******************************************************************************/
/* Name:       opensem                                                        */
/*                                                                            */
/* Arguments:  sid - place for the semaphore identifier                       */
/*             key - the "name" of the semaphore                              */
/*                                                                            */
/* Returned:   0 - worked well                                                */
/*             1 - error                                                      */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
int opensem(int *sid, key_t key){

  /* Open the semaphore set - do not create ! */
  if((*sid = semget(key, 0, iSemShmMode)) == -1)
    return (1);        /* error - not exists  */
  return (0);          /* worked well         */
}

/******************************************************************************/
/* Name:       locksem, sleep if locked                                       */
/*                                                                            */
/* Arguments:  sid - semaphore identifier                                     */
/*             member - which semaphore                                       */
/*                                                                            */
/* Returned:   -1 - error                                                     */
/*              0 - woked well                                                */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
int locksem(int sid, int member){

  struct sembuf sem_lock = {member, -1,IPC_NOWAIT};
#if defined( HAVE_NANOSLEEP )
  struct timespec    Rqtp, Rmtp;
#elif defined( HAVE_NSLEEP )
  struct timestruc_t Rqtp, Rmtp;
#endif

  if(member < 0 || member > (get_member_count(sid)-1)){
    fprintf(stderr,"*E* The member of the semaphore set does not exist (lock)!\n");
    return (-1);
  }
  sem_lock.sem_num = member;
                                   /* try to lock the semaphore               */
  while(semop(sid,&sem_lock,1) != 0 ){ /* while no success                    */
    if((errno) && (errno != EAGAIN))/* if there was a real error              */
      return (-1);                 /* return so                               */
#if defined( HAVE_NANOSLEEP )
    Rqtp.tv_sec = 1;
    Rqtp.tv_nsec = 0;
    nanosleep(&Rqtp, &Rmtp);
#elif defined( HAVE_NSLEEP )
    Rqtp.tv_sec = 1;
    Rqtp.tv_nsec = 0;
    nsleep(&Rqtp, &Rmtp);
#else
    sleep( 1 );
#endif
    SysThreadYield();              /* free the processor (go to               */
  }
  return (0);                      /* worked well                             */
}

/******************************************************************************/
/* Name:       unlocksem                                                      */
/*                                                                            */
/* Arguments:  sid - semaphore identifier                                     */
/*             member - which semaphore                                       */
/*                                                                            */
/* Returned:   none                                                           */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
void unlocksem(int sid, int member){

  struct sembuf sem_unlock = {member, 1, IPC_NOWAIT};

  if(member < 0 || member > (get_member_count(sid)-1))
    printf("*E* The member of the semaphore set does not exist (unlock)!\n");
  sem_unlock.sem_num = member;
                              /* unlock the semaphore                        */
  if((semop(sid, &sem_unlock, 1)) == -1)
   ;
}

/******************************************************************************/
/* Name:       removesem                                                      */
/*                                                                            */
/* Arguments:  sid - semaphore identifier                                     */
/*                                                                            */
/* Returned:   none                                                           */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
void removesem(int sid)
{
  union semun semopts;

  semopts.val = 0;
  semctl(sid, 0, IPC_RMID, semopts);
}


/******************************************************************************/
/* Name:       get_member_count                                               */
/*                                                                            */
/* Arguments:  sid - semaphore identifier                                     */
/*                                                                            */
/* Returned:   amount of members in the set                                   */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
unsigned short get_member_count(int sid)
{
  struct semid_ds mysemds;
  union semun semopts;

  semopts.buf = &mysemds;
  semctl(sid, 0, IPC_STAT, semopts);
  return(semopts.buf->sem_nsems);
}


/******************************************************************************/
/* Name:       getval                                                         */
/*                                                                            */
/* Arguments:  sid - semaphore identifier                                     */
/*             member - which semaphore                                       */
/*                                                                            */
/* Returned:   semaphore value                                                */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
int  getval(int sid, int member)
{
  int semval;
  union semun semopts;

  semopts.val = 0;
  semval = semctl(sid, member, GETVAL, semopts);
  return semval;
}

/******************************************************************************/
/* Name:       semgetpid                                                      */
/*                                                                            */
/* Arguments:  sid - semaphore identifier                                     */
/*             member - which semaphore                                       */
/*                                                                            */
/* Returned:   pid of the last process which called semop()                   */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
int  semgetpid(int sid, int member)
{
  int pid;
  union semun semopts;

  semopts.val = 0;
  pid = semctl(sid, member, GETPID, semopts);
  return pid;
}
/******************************************************************************/
/* Name:       semgetnumberwaiting                                            */
/*                                                                            */
/* Arguments:  sid - semaphore identifier                                     */
/*             member - which semaphore                                       */
/*                                                                            */
/* Returned:   number of processes waiting on the sem                         */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
int  semgetnumberwaiting(int sid, int member)
{
  int waiting;
  union semun semopts;

  waiting = semctl(sid, member, GETNCNT, semopts);
  return waiting;
}

/******************************************************************************/
/* Name:       init_sema                                                      */
/*                                                                            */
/* Arguments:  sid - semaphore identifier                                     */
/*             member - which semaphore                                       */
/*                                                                            */
/* Returned:   none                                                           */
/*                                                                            */
/******************************************************************************/
void init_sema(int sid, int member)
{
  // union semun semopts;

  // semopts.val = 1;                   /* initial value                */
                                        /* do the initialisation        */
  union semun semopts;

  semopts.val = 1;                   /* initial value                */
  semctl(sid, member, SETVAL, semopts);
}


