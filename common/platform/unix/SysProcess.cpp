/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2021 Rexx Language Association. All rights reserved.    */
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
/*****************************************************************************/
/*                                                                           */
/* Process support for Unix based systems.                                   */
/*                                                                           */
/*****************************************************************************/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_PWD_H
# include <pwd.h>
#endif

#ifdef HAVE_DLFCN_H
# include <dlfcn.h>
#endif

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "SysProcess.hpp"
#include "SysThread.hpp"
#include "rexx.h"
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <errno.h>
#ifdef HAVE_KDMKTONE
# include <linux/kd.h>
#endif
#ifdef HAVE_NSGETEXECUTABLEPATH
# include <mach-o/dyld.h>
#endif
#if defined HAVE_KERN_PROC_PATHNAME || defined HAVE_KERN_PROC_ARGV
# include <sys/sysctl.h>
#endif


// full path of the currently running executable
const char *SysProcess::executableFullPath = NULL;
// directory of our Rexx shared libraries
const char *SysProcess::libraryLocation = NULL;

/**
 * Get the current user name information.
 *
 * @param buffer The buffer (of at least MAX_USERID_LENGTH characters) into
 * which the userid is copied.
 */
void SysProcess::getUserID(char *buffer)
{
    struct passwd *pstUsrDat;

    pstUsrDat = getpwuid(geteuid());
    strncpy(buffer, pstUsrDat->pw_name, MAX_USERID_LENGTH-1);
}


/**
 * Determine the location of the running program. This returns the path
 * of the current executable.
 *
 * @return A character string of the location (does not need to be freed by the caller)
 */
const char* SysProcess::getExecutableFullPath()
{
    if (executableFullPath != NULL)
    {
        return executableFullPath;
    }

    char path[PATH_MAX] = ""; // we have no valid path yet
    const char *path_p = path;

    // run Darwin/Solaris/BSD-specific functions to retrieve the path
    // in some cases they may fail to retrieve a valid path (e. g. on
    // NetBSD where HAVE_KERN_PROC_PATHNAME is defined, sysctl succeeds,
    // but returns len == 0)
#ifdef HAVE_NSGETEXECUTABLEPATH
    // Darwin
    uint32_t length = sizeof(path);
    if (_NSGetExecutablePath(path, &length) != 0)
    {
        path[0] = '\0';
    }
#elif defined HAVE_GETEXECNAME
    // Solaris/OpenIndiana
    path_p = getexecname();
    if (path_p == NULL)
    {
        path_p = path;
        path[0] = '\0';
    }
#elif defined HAVE_KERN_PROC_PATHNAME
    // FreeBSD, DragonFly BSD
    int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1};
    size_t len = PATH_MAX;
    if (sysctl(mib, 4, path, &len, NULL, 0) == -1 || len == 0)
    {
        // sysctl has failed or len was returned as zero, maybe this is
        // NetBSD which uses different arguments
        mib[1] = KERN_PROC_ARGS;
        mib[2] = -1;
        mib[3] = KERN_PROC_PATHNAME;
        len = PATH_MAX;
        if (sysctl(mib, 4, path, &len, NULL, 0) == -1 || len == 0)
        {
            path[0] = '\0';
        }
    }
#elif defined HAVE_KERN_PROC_ARGV
    // OpenBSD
    // no means to retrieve the executable path, need to resort to argv[0]
    int mib[4] = {CTL_KERN, KERN_PROC_ARGS, getpid(), KERN_PROC_ARGV};
    size_t len;
    char **argv;
    path[0] = '\0';
    if (sysctl(mib, 4, NULL, &len, NULL, 0) != -1 &&
       (argv = (char **)malloc(len)) != NULL)
    {
        if (sysctl(mib, 4, argv, &len, NULL, 0) != -1 && len > 0)
        {
            // to be 100% reliable, only accept an absolute path
            if (argv[0][0] == '/')
            {
                strcpy(path, argv[0]);
            }
        }
        free(argv);
    }
#endif

    // if we have no OS-specific functions defined, or they failed to
    // retrieve a valid path, try procfs
    if (path[0] == '\0')
    {
        const char *procfs[4];
        char proc_path[32];

        procfs[0] = "/proc/self/exe";     // Linux, NetBSD
        procfs[1] = "/proc/curproc/exe";  // NetBSD
        procfs[2] = "/proc/curproc/file"; // FreeBSD, DragonFly BSD
        snprintf(proc_path, sizeof(proc_path), "/proc/%d/path/a.out", getpid());
        procfs[3] = proc_path;            // Solaris/OpenIndiana

        ssize_t bytes = 0;
        for (int i = 0; i < sizeof(procfs) / sizeof(procfs[0]) && bytes == 0; i++)
        {
            bytes = readlink(procfs[i], path, sizeof(path));
            if (bytes == -1 || bytes == sizeof(path))
            {
                bytes = 0;
            }
        }
        path[bytes] = '\0'; // we must always add a trailing NUL
    }

    // this is the file location with any symbolic links resolved.
    char *modulePath = realpath(path_p, NULL);
    if (modulePath == NULL)
    {
        return NULL;
    }

    // save this for future use
    executableFullPath = modulePath;
    return executableFullPath;
}


/**
 * Determine the location of the Rexx shared libraries. This returns the
 * directory portion of the library path with a trailing slash.
 *
 * @return A character string of the location (does not need to be freed by the caller)
 */
const char* SysProcess::getLibraryLocation()
{
    if (libraryLocation != NULL)
    {
        return libraryLocation;
    }

#ifdef HAVE_DLADDR
    Dl_info dlInfo;
    if (dladdr((void *)RexxCreateQueue, &dlInfo) == 0)
    {
        // a zero return means this could not be resolved. Should
        // not be possible, but we'll just return NULL.
        return NULL;
    }

    // this is the file location with any symbolic links
    // resolved.
    char *modulePath = realpath(dlInfo.dli_fname, NULL);

    size_t pathLength = strlen(modulePath);

    // scan backwards to find the last directory delimiter
    for (; pathLength > 0; pathLength--)
    {
        // is this the directory delimiter?
        if (modulePath[pathLength - 1] == '/')
        {
            // terminate the string after the first encountered slash and quit
            modulePath[pathLength] = '\0';
            break;
        }
    }

    // belt-and-braces, make sure we found a directory
    if (pathLength == 0)
    {
        free(modulePath);
        return NULL;
    }

    // save this for future use
    libraryLocation = modulePath;
    return libraryLocation;
#else
    // no means to determine this, so we always return NULL
    return NULL;
#endif
}


/**
 * Sound the speaker.
 *
 * @param frequency The frequency to beep at
 * @param duration  The duration to beep (in milliseconds)
 *
 * @return true if we were able to play this, false otherwise
 */
bool SysProcess::playSpeaker(int frequency, int duration)
{
#ifdef HAVE_KDMKTONE

    const char *console[] =
    {
        "/dev/tty0",
        "/dev/tty1",
        "/dev/tty",
        "/dev/console",
        "/dev/vc/0"
    };

    int fd;
    int io = -1;

    // We need a file descriptor to run ioctl on the console, which will
    // typically require root access rights.  Try a few devices and see
    // if we can successfully open one of them.
    for (int i = 0; i < sizeof(console) / sizeof(console[0]) && io < 0; i++)
    {
        // according to the docs open() may have unwanted side effects
        // that can be avoided under Linux with the O_NONBLOCK flag
        fd = open(console[i], O_RDWR | O_NONBLOCK);
        if (fd >= 0)
        {
            // test KDMKTONE with zero just to see whether this will work
            io = ioctl(fd, KDMKTONE, 0);
            if (io >=0)
            {
                // 1193180 is the magic number of clock cycles that the docs
                // tell you to use to get a frequency in clock cycles
                int pitch = 1193180 / frequency;
                ioctl(fd, KDMKTONE, (duration << 16) | pitch);

                // the sound is on, now wait for duration milliseconds
                // MAX_DURATION is 60000, so there can be no overflow
                SysThread::longSleep(duration * 1000);

                // turn sound off again
                ioctl(fd, KDMKTONE, 0);

                close(fd);
                return true;
            }
            close(fd);
        }
    }
#endif
    // not available, need to use the low tech version
    return false;
}



/**
 * do a beep tone
 *
 * @param frequency The frequency to beep at
 * @param duration  The duration to beep (in milliseconds)
 */
void SysProcess::beep(int frequency, int duration)
{
    // try to directly activate the speaker. If this fails, just send a bell
    // character to the console.
    if (!playSpeaker(frequency, duration))
    {
        printf("\a");
    }
}
