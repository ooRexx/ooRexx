/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2009-2021 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                                        */
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


#ifndef ORXUNIXAPI_H
#define ORXUNIXAPI_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <oorexxapi.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <signal.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <netdb.h>
#include <arpa/inet.h>
#ifdef HAVE_WORDEXP_H
#include <wordexp.h>
#endif
#ifdef HAVE_ALLOCA
#include <alloca.h>
#endif
#include <pthread.h>
#include <errno.h>
#include <dirent.h>

#ifdef HAVE_SYS_XATTR_H
#define HAVE_XATTR 1
#include <sys/xattr.h>
#endif

/*----------------------------------------------------------------------------*/
/* Definitions                                                                */
/*----------------------------------------------------------------------------*/

#if defined(__APPLE__)
# define stat64 stat

// on DARWIN the xattr functions have additional arguments
// ssize_t getxattr(const char *path, const char *name, void *value, size_t size, u_int32_t position, int options);
// int setxattr(const char *path, const char *name, const void *value, size_t size, u_int32_t position, int options);
// int removexattr(const char *path, const char *name, int options);
// ssize_t listxattr(const char *path, char *namebuff, size_t size, int options);
#define GetXattr(path, name, value, size) getxattr(path, name, value, size, 0, 0)
#define SetXattr(path, name, value, size, options) setxattr(path, name, value, size, 0, options)
#define RemoveXattr(path, name) removexattr(path, name, 0)
#define ListXattr(path, namebuff, size) listxattr(path, namebuff, size, 0)
#else
#define GetXattr getxattr
#define SetXattr setxattr
#define RemoveXattr removexattr
#define ListXattr listxattr
#endif

#if defined(OPSYS_NETBSD) || defined(OPSYS_FREEBSD) || defined(OPSYS_OPENBSD)
#define stat64 stat
#endif

#if !defined(HOST_NAME_MAX) && defined(_POSIX_HOST_NAME_MAX)
# define HOST_NAME_MAX _POSIX_HOST_NAME_MAX
#endif

/*----------------------------------------------------------------------------*/
/* Global variables                                                           */
/*----------------------------------------------------------------------------*/


#endif /* ORXUNIXAPI_H */

