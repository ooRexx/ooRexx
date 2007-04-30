dnl
dnl Check to see if FILE struct contains _cnt member
dnl
AC_DEFUN([ORX_FILE__CNT],
[
AC_MSG_CHECKING(if FILE struct contains _cnt member)
AC_TRY_COMPILE([#include <stdio.h>],
[FILE *fp; fp->_cnt = 0;],
  orx_result=yes; AC_DEFINE(HAVE_FILE__CNT, 1, Define to 1 if FILE struct contains _cnt member), orx_result=no )
AC_MSG_RESULT($orx_result)
])

dnl
dnl Check to see if FILE struct contains _IO_read_ptr member
dnl
AC_DEFUN([ORX_FILE__IO_READ_PTR],
[
AC_MSG_CHECKING(if FILE struct contains _IO_read_ptr member)
AC_TRY_COMPILE([#include <stdio.h>],
[FILE *fp; fp->_IO_read_ptr = 0;],
  orx_result=yes; AC_DEFINE(HAVE_FILE__IO_READ_PTR, 1, Define to 1 if FILE struct contains _IO_read_ptr member), orx_result=no )
AC_MSG_RESULT($orx_result)
])

dnl
dnl Check to see if union semun is (incorrectly) defined in sys/sem.h
dnl
AC_DEFUN([ORX_SEMUN_DEFINED],
[
AC_MSG_CHECKING(if union semun is incorrectly defined)
AC_TRY_COMPILE([
#include <sys/types.h>
#ifdef HAVE_SYS_SEM_H
#include <sys/sem.h>
#endif
],
[union semun semopts],
  orx_result=yes; AC_DEFINE(HAVE_UNION_SEMUN, 1, Define to 1 if union semun is defined in sys/sem.h), orx_result=no )
AC_MSG_RESULT($orx_result)
])

dnl
dnl Check which values are valid for pthread_mutexattr_settype() arg 4 are valid
dnl
AC_DEFUN([ORX_PTHREAD_MUTEXATTR_SETTYPE_ARG2],
[
AC_MSG_CHECKING(which values are valid for pthread_mutexattr_settype arg 2 are valid)
AC_TRY_COMPILE([
#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif
#ifdef HAVE_SYS_SEM_H
#include <sys/sem.h>
#endif
],
[
pthread_mutexattr_t mutexattr;
pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE_NP);
],
  orx_result=yes; AC_DEFINE(HAVE_PTHREAD_MUTEX_RECURSIVE_NP, 1, Define to 1 if PTHREAD_MUTEX_RECURSIVE_NP is a valid value), orx_result=no )
AC_TRY_COMPILE([
#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif
#ifdef HAVE_SYS_SEM_H
#include <sys/sem.h>
#endif
],
[
pthread_mutexattr_t mutexattr;
pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
],
  orx_result=yes; AC_DEFINE(HAVE_PTHREAD_MUTEX_RECURSIVE, 1, Define to 1 if PTHREAD_MUTEX_RECURSIVE is a valid value), orx_result=no )
AC_TRY_COMPILE([
#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif
#ifdef HAVE_SYS_SEM_H
#include <sys/sem.h>
#endif
],
[
pthread_mutexattr_t mutexattr;
pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_ERRORCHECK);
],
  orx_result=yes; AC_DEFINE(HAVE_PTHREAD_MUTEX_ERRORCHECK, 1, Define to 1 if PTHREAD_MUTEX_ERRORCHECK is a valid value), orx_result=no )
AC_MSG_RESULT(found)
]
)

