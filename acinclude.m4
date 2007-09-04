dnl
dnl Get the Subversion revision
dnl
AC_DEFUN([ORX_SVN_REV],
[
SVN_REV=$(svnversion)
SVN_REV=${SVN_REV%%[[:a-zA-Z]]*}
if test -z "$SVN_REV"; then
   AC_MSG_WARN([setting Subversion Revision to 0])
   SVN_REV="0"
fi
AC_MSG_RESULT(Subversion revision: $SVN_REV)
])

dnl
dnl Put the ooRexx version numbers into AC_SUBST
dnl substitutable variables
dnl
AC_DEFUN([ORX_VERSION_CHECK],
[
ORX_SUBST_MAJOR=`sed -n -e '/^ORX_MAJOR=/s/ORX_MAJOR=//p' oorexx.ver`
ORX_SUBST_MINOR=`sed -n -e '/^ORX_MINOR=/s/ORX_MINOR=//p' oorexx.ver`
ORX_SUBST_MOD_LVL=`sed -n -e '/^ORX_MOD_LVL=/s/ORX_MOD_LVL=//p' oorexx.ver`
ORX_SUBST_CURRENT=$ORX_SUBST_MAJOR
ORX_SUBST_REVISION=`sed -n -e '/^ORX_REVISION=/s/ORX_REVISION=//p' oorexx.ver`
ORX_SUBST_AGE=`sed -n -e '/^ORX_AGE=/s/ORX_AGE=//p' oorexx.ver`
AC_MSG_RESULT(ooRexx major number: $ORX_SUBST_MAJOR)
AC_MSG_RESULT(ooRexx minor number: $ORX_SUBST_MINOR)
AC_MSG_RESULT(ooRexx mod level number: $ORX_SUBST_MOD_LVL)
AC_MSG_RESULT(ooRexx current number: $ORX_SUBST_CURRENT)
AC_MSG_RESULT(ooRexx revision number: $ORX_SUBST_REVISION)
AC_MSG_RESULT(ooRexx age number: $ORX_SUBST_AGE)
])

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

