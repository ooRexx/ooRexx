dnl
dnl Copyright (c) 2005-2008 Rexx Language Association. All rights reserved.
dnl
dnl This program and the accompanying materials are made available under
dnl the terms of the Common Public License v1.0 which accompanies this
dnl distribution. A copy is also available at the following address:
dnl http://www.oorexx.org/license.html
dnl
dnl Redistribution and use in source and binary forms, with or
dnl without modification, are permitted provided that the following
dnl conditions are met:
dnl
dnl Redistributions of source code must retain the above copyright
dnl notice, this list of conditions and the following disclaimer.
dnl Redistributions in binary form must reproduce the above copyright
dnl notice, this list of conditions and the following disclaimer in
dnl the documentation and/or other materials provided with the distribution.
dnl
dnl Neither the name of Rexx Language Association nor the names
dnl of its contributors may be used to endorse or promote products
dnl derived from this software without specific prior written permission.
dnl
dnl THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
dnl "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
dnl LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
dnl FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
dnl OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
dnl SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
dnl TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
dnl OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
dnl OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
dnl NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
dnl SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
dnl

dnl Process this file with autoconf to produce a configure script
AC_INIT(oorexx, 4.0.0)
AC_PREFIX_DEFAULT(/opt/ooRexx)

dnl This is to fix a problem with distribution prior to Fedora 7. Fedora 7
dnl requires the share/man location, not just man. Prior distributions didn't
dnl care that the share was left out but Fedora 7 absolutely requires it. So to
dnl make things portable we redefine mandir to the correct value so that all
dnl older distributions are happy.
mandir="\$(prefix)/share/man"

PACKAGE=ooRexx
VERSION=4.0.0

AC_CANONICAL_SYSTEM

AM_INIT_AUTOMAKE($PACKAGE, $VERSION)

AM_CONFIG_HEADER(config.h)

AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_RANLIB

AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL

GENERIC_OS="-DLINUX"
GENERIC_OPSYS="-DOPSYS_LINUX"
GENERIC_DEFINES="-DNOOPT -DPTHREAD_KERNEL \
                 -D_POSIX_THREAD -D_REENTRANT  \
                 -D_GNU_SOURCE"

dnl Now we throw in a bunch of warnings.  C and C++ need
dnl different sets here
GCC_WARNINGS="\
        -Wall \
        -funsigned-char \
        -Wmissing-prototypes \
        -Wmissing-declarations \
        -Wstrict-prototypes \
        -Wpointer-arith \
        -Wformat=2 \
        -Wformat-security \
        -Wformat-nonliteral \
        -Wno-format-y2k \
        -Wcast-qual \
        -Wcast-align"

GCXX_WARNINGS="\
        -Wall \
        -funsigned-char \
        -Wpointer-arith \
        -Wcast-qual \
        -Wcast-align \
        -Wshadow \
        -Wwrite-strings \
        -D__cplusplus \
        -Wredundant-decls"

XLC_WARNINGS="\
        -qchars=unsigned \
        -qcpluscmt       \
        -qnodigraph"

XLCXX_WARNINGS="\
        -+ \
        -qchars=unsigned \
        -qnodigraph"

dnl Set platform and compiler specific switches
PACKAGETYPE="none"
ORX_SYS_STR="unknown"
ORX_SHARED_LIBRARY_EXT=".so"
ORX_LDADD_EXECUTABLE="-ldl -lpthread"
ORX_LIBADD_LIBRARY=""
ORX_LDFLAGS_EXECUTABLE=""
ORX_LDFLAGS_LIBRARY=""
ORX_LDFLAGS_PACKAGE="-Xcompiler -nostartfiles"
ORX_PLATFORM="unix"
ORX_INIT_LDFLAGS=""
case "$target" in
   *ibm-aix*)
      GENERIC_OS="-DAIX"
      GENERIC_OPSYS="-DOPSYS_AIX"
      GENERIC_DEFINES="$GENERIC_DEFINES"
#     ORX_SHARED_LIBRARY_EXT=".a"
      case "$CXX" in
         g++)
            CC_WARNINGS="-maix32 -pthread $GCC_WARNINGS"
            CXX_WARNINGS="-maix32 -pthread $GCXX_WARNINGS"
            ;;
         xlC_r)
            CC_WARNINGS="$XLC_WARNINGS"
            CXX_WARNINGS="$XLCXX_WARNINGS"
            ORX_LDFLAGS_PACKAGE=""
            ORX_INIT_LDFLAGS="-Wl,-binitfini:_init:_fini"
            ;;
         *)
dnl         AC_MSG_ERROR("Compiler $CXX is not supported on this platform")
            ;;
      esac
      PACKAGETYPE="aix"
      ORX_SYS_STR="AIX"
      ;;
   *solaris*)
      GENERIC_OPSYS="$GENERIC_OPSYS -DOPSYS_SUN"
      case "$CXX" in
         g++)
            CC_WARNINGS="$GCC_WARNINGS"
            CXX_WARNINGS="$GCXX_WARNINGS"
            ;;
         *)
dnl         AC_MSG_ERROR("Compiler $CXX is not supported on this platform")
            ;;
      esac
      PACKAGETYPE="sun"
      ORX_SYS_STR="SUNOS"
      case "$target" in
         *solaris2.8)
            SPECIFIC_DEFINES="-DLIMITED_SOLARIS_SEMS"
            ;;
         *solaris2.9)
            SPECIFIC_DEFINES="-DLIMITED_SOLARIS_SEMS"
            ;;
         *)
      esac
      ;;
   sparc*sunos*)
      ORX_SYS_STR="SUNOS"
      ;;
   i*86-*linux*)
      CC_WARNINGS="$GCC_WARNINGS"
      CXX_WARNINGS="$GCXX_WARNINGS"
      ORX_SYS_STR="LINUX"
      ;;
   *86_64-*linux*)
      CC_WARNINGS="$GCC_WARNINGS"
      CXX_WARNINGS="$GCXX_WARNINGS"
      ORX_SYS_STR="LINUX"
      ;;
   alpha*-*linux*)
      CC_WARNINGS="$GCC_WARNINGS"
      CXX_WARNINGS="$GCXX_WARNINGS"
      ORX_SYS_STR="LINUX"
      ;;
   i370-*ibm*)
      dnl This a 31 bit system
      ORX_SYS_STR="LINUX"
      GENERIC_DEFINES="$GENERIC_DEFINES -m31"
      CC_WARNINGS="$GCC_WARNINGS"
      CXX_WARNINGS="$GCXX_WARNINGS"
      ;;
   s390x-*ibm*)
      dnl This a 64 bit system
      ORX_SYS_STR="LINUX"
      GENERIC_DEFINES="$GENERIC_DEFINES -m31"
      CC_WARNINGS="$GCC_WARNINGS"
      CXX_WARNINGS="$GCXX_WARNINGS"
      ;;
   s390-*ibm*)
      dnl This a 31 bit system
      ORX_SYS_STR="LINUX"
      CC_WARNINGS="$GCC_WARNINGS"
      CXX_WARNINGS="$GCXX_WARNINGS"
      ;;
   *linux*)
      ORX_SYS_STR="LINUX"
      CC_WARNINGS="$GCC_WARNINGS"
      CXX_WARNINGS="$GCXX_WARNINGS"
      ;;
   *cygnus*)
      CC_WARNINGS="$GCC_WARNINGS"
      CXX_WARNINGS="$GCXX_WARNINGS"
      ORX_SYS_STR="CYGNUS"
      ORX_SHARED_LIBRARY_EXT=".dll"
      ORX_PLATFORM="windows"
      ;;
   *freebsd*)
      ORX_SYS_STR="FREEBSD"
      ORX_LDADD_EXECUTABLE="-pthread"
      ORX_LIBADD_LIBRARY=""
      ORX_LDFLAGS_EXECUTABLE="-pthread"
      ORX_LDFLAGS_LIBRARY="-shared -pthread"
      ORX_LDFLAGS_PACKAGE="-nostartfiles -shared"
      ;;
   *netbsd*)
      ORX_SYS_STR="NETBSD"
      ORX_LDADD_EXECUTABLE="-pthread"
      ORX_LIBADD_LIBRARY=""
      ORX_LDFLAGS_EXECUTABLE="-pthread"
      ORX_LDFLAGS_LIBRARY="-shared -pthread"
      ORX_LDFLAGS_PACKAGE="-nostartfiles -shared"
      ;;
   *openbsd*)
      ORX_SYS_STR="NETBSD"
      ORX_LDADD_EXECUTABLE="-pthread"
      ORX_LIBADD_LIBRARY=""
      ORX_LDFLAGS_EXECUTABLE="-pthread"
      ORX_LDFLAGS_LIBRARY="-shared -pthread"
      ORX_LDFLAGS_PACKAGE="-nostartfiles -shared"
      ;;
   *apple-darwin*)
      ORX_SYS_STR="MACOSX"
      ORX_SHARED_LIBRARY_EXT=".dylib"
      ;;
   *hp-hpux*)
      ORX_SHARED_LIBRARY_EXT=".sl"
      ;;
   *dec-osf*)
      ;;
   *nto-qnx*)
      ;;
   *qnx*)
      ;;
   *beos*)
      ;;
   *)
      ;;
esac

# Find out if we are big endian
AC_C_BIGENDIAN()

#DEVEL_DEFINES="$GENERIC_OS $GENERIC_DEFINES $SPECIFIC_DEFINES"
#AC_SUBST(DEVEL_DEFINES)

AC_SUBST(PACKAGETYPE)
AC_SUBST(ORX_SYS_STR)
AC_SUBST(ORX_SHARED_LIBRARY_EXT)
AC_SUBST(ORX_LDADD_EXECUTABLE)
AC_SUBST(ORX_LIBADD_LIBRARY)
AC_SUBST(ORX_LDFLAGS_EXECUTABLE)
AC_SUBST(ORX_LDFLAGS_LIBRARY)
AC_SUBST(ORX_LDFLAGS_PACKAGE)
AC_SUBST(ORX_PLATFORM)
AC_SUBST(ORX_INIT_LDFLAGS)

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([ \
                  fcntl.h \
                  features.h \
                  filehdr.h \
                  limits.h \
                  locale.h \
                  mesg.h \
                  malloc.h \
                  mesg.h \
                  netinet/in.h \
                  nl_types.h \
                  pthread.h \
                  pwd.h \
                  sched.h \
                  signal.h \
                  stdarg.h \
                  stdlib.h \
                  string.h \
                  stropts.h \
                  sys/filio.h \
                  sys/ldr.h \
                  sys/resource.h \
                  sys/select.h \
                  sys/sem.h \
                  sys/signal.h \
                  sys/socket.h \
                  sys/time.h \
                  sys/utsname.h \
                  sys/wait.h \
                  time.h \
                  unistd.h \
                  usersec.h \
                  ])

dnl Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_VPRINTF
AC_HEADER_TIME

AC_CHECK_FUNCS([ \
                IDtouser \
                getuserattr \
                catopen \
                fstat \
                gcvt \
                geteuid \
                getpgrp \
                getpwuid \
                gettimeofday \
                memset \
                nsleep \
                setlocale \
                strdup \
                sighold \
                sigprocmask \
                ])

dnl Special check for nanosleep
AC_CHECK_FUNC(nanosleep)
if test x"$ac_cv_func_nanosleep" = xno; then
   AC_CHECK_LIB(rt, nanosleep)
   if test x"$ac_cv_lib_rt_nanosleep" = xyes; then
      AC_DEFINE(HAVE_NANOSLEEP, 1, Define to 1 if you have nanosleep function)
   fi
else
   AC_DEFINE(HAVE_NANOSLEEP, 1, Define to 1 if you have nanosleep function)
fi

dnl Special check for pthread_mutexattr_settype
AC_CHECK_FUNC(pthread_mutexattr_settype)
if test x"$ac_cv_func_pthread_mutexattr_settype" = xno; then
   AC_CHECK_LIB(pthread, pthread_mutexattr_settype)
   if test x"$ac_cv_lib_pthread_pthread_mutexattr_settype" = xyes; then
      AC_DEFINE(HAVE_PTHREAD_MUTEXATTR_SETTYPE, 1, Define to 1 if you have pthread_mutexattr_settype function)
   fi
else
   AC_DEFINE(HAVE_PTHREAD_MUTEXATTR_SETTYPE, 1, Define to 1 if you have pthread_mutexattr_settype function)
fi

ORX_SEMUN_DEFINED
ORX_PTHREAD_MUTEXATTR_SETTYPE_ARG2

dnl Checks for FILE struct members
ORX_FILE__CNT
ORX_FILE__IO_READ_PTR

dnl Checks for other helper programs
AC_PATH_PROGS(RPM, rpmbuild)
AC_PATH_PROGS(GENCAT, gencat)
if test -z "$GENCAT"; then
   AC_MSG_WARN([cannot find gencat, no Rexx error messages available])
   REXXCAT=""
else
   REXXCAT="rexx.cat"
fi
AC_SUBST(REXXCAT)

xsl=''
AC_PATH_PROGS([xsl], [Xalan xalan])
if test -z "$xsl"; then
   AC_MSG_WARN([cannot find xalan, Xalan])
else
   # $1="OUT" $2="XML" $3="XSL"
   case "$xsl" in
      *Xalan)
         xsl="$xsl -o \$1 \$2 \$3"
         ;;
      *xalan)
         xsl="$xsl -out \$1 -in \$2 -xsl \$3"
         ;;
      *)
         xsl="echo Cannot transform XSL as Xalan not installed;exit 1"
         ;;
   esac
fi

ORX_SVN_REV
AC_SUBST(SVN_REV)
ORX_VERSION_CHECK
AC_SUBST(ORX_SUBST_MAJOR)
AC_SUBST(ORX_SUBST_MINOR)
AC_SUBST(ORX_SUBST_MOD_LVL)
AC_SUBST(ORX_SUBST_CURRENT)
AC_SUBST(ORX_SUBST_REVISION)
AC_SUBST(ORX_SUBST_AGE)

dnl remerge the warnings into the FLAGS
GENERAL_CXXFLAGS="\
              $SPECIFIC_DEFINES \
              $GENERIC_DEFINES \
              $GENERIC_OS \
              $GENERIC_OPSYS"

GENERAL_CFLAGS="\
              $SPECIFIC_DEFINES \
              $GENERIC_DEFINES \
              $GENERIC_OS \
              $GENERIC_OPSYS"
CXXFLAGS="$CXXFLAGS $CFLAGS $CXX_WARNINGS $GENERAL_CXXFLAGS"
CFLAGS="$CFLAGS $CC_WARNINGS $GENERAL_CFLAGS"

AC_SUBST(TOP_SRCDIR,$top_srcdir)

AC_CONFIG_FILES([
        Makefile
	platform/unix/oorexx.spec
	platform/unix/debian/changelog
	platform/unix/debian/postinst
	rexxapi/server/platform/unix/rxapid
        samples/Makefile
        samples/unix/Makefile
        samples/unix/api/Makefile
        samples/unix/api/callrexx/Makefile
        samples/unix/api/wpipe1/Makefile
        samples/unix/api/wpipe2/Makefile
        samples/unix/api/wpipe3/Makefile
        samples/windows/Makefile
        samples/windows/api/Makefile
        samples/windows/api/callrxnt/Makefile
        samples/windows/api/callrxwn/Makefile
        samples/windows/api/rexxexit/Makefile
        samples/windows/api/wpipe/Makefile
        samples/windows/api/wpipe/wpipe1/Makefile
        samples/windows/api/wpipe/wpipe2/Makefile
        samples/windows/api/wpipe/wpipe3/Makefile
        samples/windows/misc/Makefile
        samples/windows/ole/Makefile
        samples/windows/ole/adsi/Makefile
        samples/windows/ole/apps/Makefile
        samples/windows/ole/methinfo/Makefile
        samples/windows/ole/oleinfo/Makefile
        samples/windows/ole/wmi/Makefile
        samples/windows/ole/wmi/sysinfo/Makefile
        samples/windows/oodialog/Makefile
        samples/windows/oodialog/bmp/Makefile
        samples/windows/oodialog/examples/Makefile
        samples/windows/oodialog/examples/resources/Makefile
        samples/windows/oodialog/ooRexxTry/Makefile
        samples/windows/oodialog/ooRexxTry/doc/Makefile
        samples/windows/oodialog/rc/Makefile
        samples/windows/oodialog/res/Makefile
        samples/windows/oodialog/tutorial/Makefile
        samples/windows/oodialog/wav/Makefile
        samples/windows/rexutils/Makefile
        samples/windows/winsystem/Makefile
        samples/windows/wsh/Makefile
        xsl.sh
        platform/unix/oorexx-config
 ])

AC_OUTPUT
