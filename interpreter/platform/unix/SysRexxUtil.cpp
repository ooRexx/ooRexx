/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2019 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                                         */
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
/* REXX unix support                                                          */
/*                                                                            */
/* unix-based system utility functions                                        */
/*                                                                            */
/******************************************************************************/
/**********************************************************************
*                                                                     *
*   This program extends the REXX language by providing many          *
*   REXX external functions.                                          *
*   These are a partial list of functions included:                   *
*       SysCls              -- Clear the screen in a command prompt   *
*                              session                                *
*       SysCurPos           -- Set and/or Query the cursor position   *
*                              in an OS/2 fullscreen or windowed      *
*                              command prompt session.                *
*       SysCurState         -- Make the cursor visible or invisible   *
*                              in an OS/2 fullscreen or windowed      *
*                              command prompt session.                *
*       SysGetKey           -- Returns one by of data indicating the  *
*                              key which was pressed,                 *
*                              in an OS/2 fullscreen or windowed      *
*                              command prompt session.                *
*       SysGetMessage       -- Retrieves a message text from an OS/2  *
*                              message file, substituting values      *
*                              provided.                              *
*       SysMkDir            -- Creates a directory                    *
*       SysVersion          -- Returns the system  Version number     *
*       SysLinVer           -- Returns the Linux Version number       *
*                              command prompt session.                *
*       SysCreateMutexSem   -- Create a Mutex semaphore               *
*       SysOpenMutexSem     -- Open a Mutex semaphore                 *
*       SysCloseMutexSem    -- Close a Mutex semaphore                *
*       SysRequestMutexSem  -- Request a Mutex semaphore              *
*       SysReleaseMutexSem  -- Release a Mutex semaphore              *
*       SysCreateEventSem   -- Create an Event semaphore              *
*       SysOpenEventSem     -- Open an Event semaphore                *
*       SysCloseEventSem    -- Close an Event semaphore               *
*       SysPostEventSem     -- Post an Event semaphore                *
*       SysResetEventSem    -- Reset an Event semaphore               *
*       SysWaitEventSem     -- Wait on an Event semaphore             *
*       SysSetFileDateTime  -- Set the last modified date of a file   *
*       SysGetFileDateTime  -- Get the last modified date of a file   *
*       SysGetErrortext     -- Retrieve textual desc. of error number *
*       SysQueryProcess     -- Get information on current proc/thread *
*                                                                     *
*       SysGetpid           -- CREXX for AIX function support         *
*       SysFork             -- CREXX for AIX function support         *
*       SysWait             -- CREXX for AIX function support         *
*       SysCreatePipe       -- CREXX for AIX function support         *
*                                                                     *
**********************************************************************/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if defined( HAVE_LOCALE_H )
#include <locale.h>
#endif

#include "oorexxapi.h"

#if defined( HAVE_SYS_WAIT_H )
#include <sys/wait.h>
#endif

#include <sys/ipc.h>
#include <memory.h>

#if defined( HAVE_MALLOC_H )
#include <malloc.h>
#endif

#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <math.h>
#include <limits.h>
#include <sys/stat.h>                  /* mkdir() function           */
#include <errno.h>                     /* get the errno variable     */
#include <stddef.h>
#include <sys/types.h>
#if !defined(AIX)
#include <sys/syscall.h>
#endif
#include <sys/utsname.h>
#include <sys/ipc.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <time.h>
#include <netdb.h>


#if defined( HAVE_SYS_SEM_H )
#include <sys/sem.h>
#endif

#include <dirent.h>                    /* directory functions        */
#include <sys/time.h>                  /* needed for the select func */

#include <time.h>                 /* needed for the select func          */

#if defined( HAVE_SYS_SELECT_H )
#include <sys/select.h>           /* needed for the select func          */
#endif

#if defined( HAVE_SYS_LDR_H )
#include <sys/ldr.h>              /* needed for the load   func          */
#endif

#if defined( HAVE_STRINGS_H )
#include <strings.h>
#endif

#include <utime.h>                /* moved, used by AIX & Linux          */

#if defined( HAVE_SYS_UTSNAME_H )
#include <sys/utsname.h>               /* get the uname() function   */
#endif

#include <signal.h>

#if defined( HAVE_SYS_RESOURCE_H )
#include <sys/resource.h>              /* get the getpriority() func */
#endif

#if defined( HAVE_FEATURES_H )
#include <features.h>                 /* catopen()                  */
#endif

#if defined( HAVE_NL_TYPES_H )
#include <nl_types.h>                  /* catopen()                  */
#endif

#include <termios.h>                   /* needed for SysGetKey       */
#include <fnmatch.h>                   /* fnmatch()                  */
#include <libgen.h>                    /* dirname, basename          */

#include "RexxUtilCommon.hpp"
#include "Utilities.hpp"
#include "RexxInternalApis.h"

#if !defined( HAVE_UNION_SEMUN )
union semun
{
     int val;
     struct semid_ds *buf;
     unsigned short *array;
};
#endif

#if defined __APPLE__
#define open64 open
// avoid warning: '(l)stat64' is deprecated: first deprecated in macOS 10.6
#define stat64 stat
#define lstat64 lstat
#endif

#define REXXMESSAGEFILE    "rexx.cat"


/*********************************************************************/
/* Numeric Error Return Strings                                      */
/*********************************************************************/

#define  ERROR_NOMEM      "2"          /* Insufficient memory        */

/******************************************************************************/
/* Defines used by SysGetKey                                                  */
/******************************************************************************/

#define stty(a,b)         (void)tcsetattr(a,TCSANOW,b) /* simple set attr.    */
#define gtty(a,b)         (void)tcgetattr(a,b)         /* simple get attr.    */
#define discard_input(a)  tcflush(a,TCIFLUSH)          /* simple flush        */
#define restore_tty(a)    stty(ttyfd,a)                /* new settings STDIN  */


/* original terminal settings                                                 */
struct termios in_orig;                /* original settings (important!!)     */


/*********************************************************************/
/****************  REXXUTIL Supporting Functions  ********************/
/****************  REXXUTIL Supporting Functions  ********************/
/****************  REXXUTIL Supporting Functions  ********************/
/*********************************************************************/

void restore_terminal(int signal)
{
    stty(STDIN_FILENO, &in_orig);          /* restore the terminal settings        */
    raise(signal);                         /* propagate signal                     */
}

/******************************************************************************/
/* getkey                                                                     */
/******************************************************************************/
/* echo == false => no echo                                                   */
/* echo == true  => echo                                                      */
/******************************************************************************/
int getkey(char *ret, bool echo)
{
    /* restore original TTY settings on exit */

    int ttyfd = STDIN_FILENO;              /* standard tty filedescriptor        */
    /* Set the cleanup handler for unconditional process termination              */
    struct sigaction new_action;


    /* Set up the structure to specify the new action                             */
    new_action.sa_handler = restore_terminal;
    sigfillset(&new_action.sa_mask);
    new_action.sa_flags = SA_RESTART;

    /* Termination signals                                                        */
    sigaction(SIGINT, &new_action, NULL);  /* exitClear on SIGINT signal          */
    sigaction(SIGTERM, &new_action, NULL); /* exitClear on SIGTERM signal         */
    sigaction(SIGQUIT, &new_action, NULL); /* exitClear on SIGQUIT signal         */
    sigaction(SIGHUP, &new_action, NULL);  /* exitClear on SIGHUP signal          */
    sigaction(SIGTSTP, &new_action, NULL); /* exitClear on SIGTSTP signal         */
    sigaction(SIGTTIN, &new_action, NULL); /* exitClear on SIGTTIN signal         */
    sigaction(SIGTTOU, &new_action, NULL); /* exitClear on SIGTTOU signal         */
    /* Error signals                                                              */
    sigaction(SIGSEGV, &new_action, NULL); /* exitClear on SIGSEGV signal         */
    sigaction(SIGFPE, &new_action, NULL);  /* exitClear on SIGFPE signal          */
    sigaction(SIGILL, &new_action, NULL);  /* exitClear on SIGILL signal          */
    sigaction(SIGBUS, &new_action, NULL);  /* exitClear on SIGBUS signal          */
    sigaction(SIGPIPE, &new_action, NULL); /* exitClear on broken pipe            */


    if (!isatty(ttyfd))                 /* connected to a terminal ?          */
    {
        ret[0] = '\0';
        return 0;
    }
    ttyfd = STDIN_FILENO;                 /* STDIN_FILENO is out default fd     */

    /* open tty                           */
    ttyfd = open("/dev/tty", O_RDONLY);   /* get filedescriptor (fd) for tty    */

    struct termios in_raw;                /* global for save reset after SIGNAL */

    gtty(ttyfd, &in_orig);                /* determine existing tty settings */

    /* restore original TTY settings on exit */

    /* change STDIN settings to raw */
    gtty(ttyfd, &in_raw);                 /* save old settings                  */

    in_raw.c_lflag &= ~ICANON;            /* no canonical mode                  */
    if (!echo)                            /* no echo flag set                   */
        in_raw.c_lflag &= ~ECHO;            /* no echo                            */
    in_raw.c_cc[VMIN] = 1;                /* read 1 byte before returning       */
    in_raw.c_cc[VTIME] = 0;               /* return immediatly (no timeout)     */
    stty(ttyfd, &in_raw);                 /* execute settings now!              */


    ret[0] = getchar();                   /* read the char                      */

    ret[1] = '\0';                        /* terminate string                   */

    restore_tty(&in_orig);                /* for standard I/O behavior          */
    close(ttyfd);                         /* close the terminal                 */
    return 0;                             /* everything is fine                 */
}

// below are Windows-specific implementations of TreeFinder methods.
inline char typeOfEntry(mode_t m)
{
    if (S_ISLNK(m))
    {
        return 'l';                // symbolic link
    }
    else if (S_ISBLK(m))
    {
        return 'b';                // block device
    }
    else if (S_ISCHR(m))
    {
        return 'c';                // character device
    }
    else if (S_ISDIR(m))
    {
        return 'd';                // directory
    }
    else if (S_ISFIFO(m))
    {
        return 'p';                // FIFO
    }
    else if (S_ISSOCK(m))
    {
        return 's';                // socket
    }
    else
    {
        return '-';                // regular file
    }
}

/**
 *
 * if this ends in a directory separator, add a *.* wildcard to the end
 */
void TreeFinder::adjustDirectory()
{
    // if just a "*" wildcard, make it a wild card for the current directory
    if (fileSpec == "*")
    {
        fileSpec = "./*";
    }

    // if this ends in a directory separator, add a * wildcard to the end
    else if (fileSpec.endsWith('/'))
    {
        fileSpec += "*";
    }
    // if the end section is either /. or /.., we also add the wildcard
    else if (fileSpec.endsWith("/.") || fileSpec.endsWith(".."))
    {
        fileSpec += "/*";
    }
    // if the spec starts with a ~, we need to expand the name.
    if (fileSpec.startsWith('~'))
    {
        SysFileSystem::canonicalizeName(fileSpec);
    }
}
/**
 * Perform any platform-specific adjustments to the platform specification.
 */
void TreeFinder::adjustFileSpec()
{
    // this is a NOP for unix file systems
}


/**
 * Tests for illegal file name characters in fileSpec. The only really
 * forbidden characters in unix file names are '/' (which is allowed in
 * the path) and '\0' (which is the string terminator. This is really
 * a nop.
 */
void TreeFinder::validateFileSpecName()
{
    // no extra validation required
}


/**
 * Perform platform-specific drive checks on the file spec. This only
 * applies to Windows.
 *
 * @return true if this was processed, false if additional work is required.
 */
bool TreeFinder::checkNonPathDrive()
{
    // not a drive, this will need to be prepended by the current directory
    return false;
}

/**
 * Format the system-specific file time, attribute mask, and size for
 * the given file.
 *
 * @param fileName The name of the file.
 */
void formatFileAttributes(TreeFinder *finder, FileNameBuffer &foundFileLine, SysFileIterator::FileAttributes &attributes)
{
    char fileAttr[256];                 // File attribute string of found file

#ifdef AIX
    struct tm stTimestamp;
    struct tm *timestamp = localtime_r(&(attributes.findFileData.st_mtime), &stTimestamp);
#else
    struct tm *timestamp = localtime(&(attributes.findFileData.st_mtime));
#endif
    if (finder->longTime())
    {

        snprintf(fileAttr, sizeof(fileAttr), "%4d-%02d-%02d %02d:%02d:%02d  ",
                 timestamp->tm_year + 1900, timestamp->tm_mon + 1, timestamp->tm_mday,
                 timestamp->tm_hour, timestamp->tm_min, timestamp->tm_sec);
    }
    else if (finder->editableTime())
    {
        snprintf(fileAttr, sizeof(fileAttr), "%02d/%02d/%02d/%02d/%02d  ",
                 (timestamp->tm_year) % 100, timestamp->tm_mon + 1, timestamp->tm_mday,
                 timestamp->tm_hour, timestamp->tm_min);
    }
    else
    {
        snprintf(fileAttr, sizeof(fileAttr), "%2d/%02d/%02d  %2d:%02d%c  ",
                 timestamp->tm_mon + 1, timestamp->tm_mday, timestamp->tm_year % 100,
                 timestamp->tm_hour < 13 ? timestamp->tm_hour : timestamp->tm_hour - 12,
                 timestamp->tm_min, (timestamp->tm_hour < 12 || timestamp->tm_hour == 24) ? 'a' : 'p');
    }

    // this is the first part of the return value. Copy to the result buffer so we can
    // reuse the buffer
    foundFileLine = fileAttr;

    // now the size information
    if (finder->longSize())
    {
        snprintf(fileAttr, sizeof(fileAttr), "%20jd  ", (intmax_t)attributes.findFileData.st_size);
    }
    else
    {
        if (attributes.findFileData.st_size > 9999999999)
        {
            attributes.findFileData.st_size = 9999999999;
        }
        snprintf(fileAttr, sizeof(fileAttr), "%10jd  ", (intmax_t)attributes.findFileData.st_size);
    }

    // the order is time, size, attributes
    foundFileLine += fileAttr;

    char tp = typeOfEntry(attributes.findFileData.st_mode);

    snprintf(fileAttr, sizeof(fileAttr), "%c%c%c%c%c%c%c%c%c%c  ",
             tp,
             (attributes.findFileData.st_mode & S_IREAD) ? 'r' : '-',
             (attributes.findFileData.st_mode & S_IWRITE) ? 'w' : '-',
             (attributes.findFileData.st_mode & S_IEXEC) ? 'x' : '-',
             (attributes.findFileData.st_mode & S_IRGRP) ? 'r' : '-',
             (attributes.findFileData.st_mode & S_IWGRP) ? 'w' : '-',
             (attributes.findFileData.st_mode & S_IXGRP) ? 'x' : '-',
             (attributes.findFileData.st_mode & S_IROTH) ? 'r' : '-',
             (attributes.findFileData.st_mode & S_IWOTH) ? 'w' : '-',
             (attributes.findFileData.st_mode & S_IXOTH) ? 'x' : '-');

    // add on this section
    foundFileLine += fileAttr;
}


/**
 * Checks if this file should be part of the included result and adds it to the result set
 * if it should be returned.
 *
 * @param attributes The file attributes for the file we're checking.
 */
void TreeFinder::checkFile(SysFileIterator::FileAttributes &attributes)
{

    // we have a directory, if we're not returning directories, we're done.
    // otherwise, we need to perform additional name checks
    if (S_ISDIR(attributes.findFileData.st_mode))
    {
        if (!options[DO_DIRS])
        {
            return;
        }
    }
    else
    {
        // not a directory...if not looking for files, we're done
        if (!options[DO_FILES])
        {
            return;
        }
    }

    // if only the name is requested, create a string object and set the
    // stem varaiable
    if (nameOnly())
    {
        addResult(foundFile);
        return;
    }

    // format all of the attributes and add them to the foundFile result
    formatFileAttributes(this, foundFileLine, attributes);

    // and finally add on the file name
    foundFileLine += foundFile;

    // add this to the stem
    addResult(foundFileLine);
}


/**
 * Platform-specific TreeFinder method for locating the end of the
 * fileSpec directory
 *
 * @return The position of the end of the directory. -1 indicates the file spec
 *         does not contain a directory.
 */
int TreeFinder::findDirectoryEnd()
{
    // Get the maximum position of the last '\'
    int lastSlashPos = (int)fileSpec.length() - 1;

    // Step back through fileSpec until at its beginning or at a '/' character
    while (fileSpec.at(lastSlashPos) != '/' && lastSlashPos >= 0)
    {
        --lastSlashPos;
    }

    return lastSlashPos;
}


/**
 * Perform any platform-specific fixup that might be required with
 * the supplied path.
 */
void TreeFinder::fixupFilePath()
{
    // now go resolve current directories, userid, etc. to get a fully
    // qualified name
    RoutineQualifiedName qualifiedName(context, filePath);

    filePath = (const char *)qualifiedName;
    // make sure this is terminated with a path delimiter
    filePath.addFinalPathDelimiter();
}


/**********************************************************************
* Function:  SysCls                                                   *
*                                                                     *
* Syntax:    call SysCls                                              *
*                                                                     *
* Return:    NO_UTIL_ERROR - Successful.                              *
**********************************************************************/
RexxRoutine0(int, SysCls)
{
    return system("clear");
}


/**
 * SysMkDir creates a directory.  No intermediate directories are created.
 *
 * @param path       The directory path.
 * @param mode       Optional file mode.  Valid values are 0 .. 511.
 *
 * @return zero if the directory was created successfully, else errno.
 */
RexxRoutine2(int, SysMkDir, CSTRING, path, OPTIONAL_int32_t, mode)
{
    RoutineQualifiedName qualifiedName(context, path);

    // Create the directory.  By default, we create with all mode bits set,
    // but the directory will always be created with the restrictions
    // specified with the umask command.
    if (argumentOmitted(2))
    {
        mode = S_IRWXU | S_IRWXG | S_IRWXO;
    }
    return mkdir(qualifiedName, mode) == 0 ? 0 : errno;
}


/*************************************************************************
* Function:  SysVersion                                                  *
*                                                                        *
* Syntax:    call SysVersion                                             *
*                                                                        *
* Return:    Operating System name (LINUX/AIX/WINDOWS) and Version       *
*************************************************************************/
RexxRoutine0(RexxStringObject, SysVersion)
{
    struct utsname info;                 /* info structur              */

    if (uname(&info) < 0)
    {                     /* if no info stored          */
        context->InvalidRoutine();
        return NULLOBJECT;
    }

    char retstr[256];

    snprintf(retstr, sizeof(retstr), "%s %s", info.sysname, info.release);
    return context->NewStringFromAsciiz(retstr);
}


/*************************************************************************
* Semaphore data struct                                                  *
*************************************************************************/

typedef struct RxSemData
{
     bool          named;               /* Named semaphore?           */
     sem_t *handle;              /* Semaphore pointer          */
} RXSEMDATA;


/*************************************************************************
* Function:  SysCreateEventSem                                           *
*                                                                        *
* Syntax:    handle = SysCreateEventSem(<name>)                          *
*                                                                        *
* Params:    name  - optional name for a event semaphore                 *
*                                                                        *
* Return:    handle - token used as a event sem handle for               *
*                     SysPostEventSem, SysClearEventSem,                 *
*                     SysCloseEventSem, and SysOpenEventSem              *
*            '' - Empty string in case of any error                      *
*************************************************************************/

RexxRoutine2(RexxObjectPtr, SysCreateEventSem, OPTIONAL_CSTRING, name, OPTIONAL_CSTRING, reset)
{
    RXSEMDATA *semdata;
    int rc;

    // Note that the reset arg has no meaning on Unix/Linux and is unused.
    semdata = (RXSEMDATA *)malloc(sizeof(RXSEMDATA));
    if (semdata == NULL)
    {
        return context->String("");
    }
    if (name == NULL)
    {
        /* this is an unnamed semaphore */
        semdata->handle = (sem_t *)malloc(sizeof(sem_t));
        rc = sem_init(semdata->handle, 0, 0);
        if (rc == -1)
        {
            free(semdata);
            return context->String("");
        }
        semdata->named = false;
    }
    else
    {
        /* this is a named semaphore */
        semdata->handle = sem_open(name, (O_CREAT | O_EXCL), (S_IRWXU | S_IRWXG), 0);
        if (semdata->handle == SEM_FAILED)
        {
            free(semdata);
            return context->String("");
        }
        semdata->named = true;
    }
    return context->Uintptr((uintptr_t)semdata);
}


/*************************************************************************
* Function:  SysOpenEventSem                                             *
*                                                                        *
* Syntax:    result = SysOpenEventSem(handle)                            *
*                                                                        *
* Params:    handle - token returned from SysCreateEventSem              *
*                                                                        *
* Return:    result - return code from DosOpenEventSem                   *
*************************************************************************/
RexxRoutine1(uintptr_t, SysOpenEventSem, CSTRING, name)
{
    RXSEMDATA *semdata;

    semdata = (RXSEMDATA *)malloc(sizeof(RXSEMDATA));
    if (semdata == NULL)
    {
        return 0;
    }
    semdata->handle = sem_open(name, 0);
    if (semdata->handle == SEM_FAILED)
    {
        return 0;
    }
    semdata->named = true;
    return (uintptr_t)semdata;
}


/*************************************************************************
* Function:  SysResetEventSem                                            *
*                                                                        *
* Syntax:    result = SysResetEventSem(handle)                           *
*                                                                        *
* Params:    handle - token returned from SysCreateEventSem              *
*                                                                        *
* Return:    result - return code from DosResetEventSem                  *
*************************************************************************/

RexxRoutine1(int, SysResetEventSem, uintptr_t, vhandle)
{
    RXSEMDATA *semdata = (RXSEMDATA *)vhandle;

    sem_init(semdata->handle, 1, 0);
    return 0;
}


/*************************************************************************
* Function:  SysPostEventSem                                             *
*                                                                        *
* Syntax:    result = SysPostEventSem(handle)                            *
*                                                                        *
* Params:    handle - token returned from SysCreateEventSem              *
*                                                                        *
* Return:    result - return code from DosPostEventSem                   *
*************************************************************************/

RexxRoutine1(int, SysPostEventSem, uintptr_t, vhandle)
{
    RXSEMDATA *semdata = (RXSEMDATA *)vhandle;
    int rc;

    rc = sem_post(semdata->handle);
    if (rc)
    {
        return 6;
    }
    return 0;
}


/*************************************************************************
* Function:  SysCloseEventSem                                            *
*                                                                        *
* Syntax:    result = SysCloseEventSem(handle)                           *
*                                                                        *
* Params:    handle - token returned from SysCreateEventSem              *
*                                                                        *
* Return:    result - return code from DosCloseEventSem                  *
*************************************************************************/

RexxRoutine1(int, SysCloseEventSem, uintptr_t, vhandle)
{
    RXSEMDATA *semdata = (RXSEMDATA *)vhandle;

    if (semdata->named == false)
    {
        /* this is an unnamed semaphore so we must free the target */
        if (sem_destroy(semdata->handle))
        {
            if (errno == EINVAL)
            {
                return 6;
            }
            else if (errno)
            {
                return 102;
            }
        }
    }
    else
    {
        /* this is a named semaphore */
        if (sem_close(semdata->handle))
        {
            if (errno == EINVAL)
            {
                return 6;
            }
            else if (errno)
            {
                return 102;
            }
        }
    }
    free(semdata);
    return 0;
}


#define SEM_WAIT_PERIOD 100 /* POSIX says this should be 10ms */


/*************************************************************************
* Function:  SysWaitEventSem                                             *
*                                                                        *
* Syntax:    result = SysWaitEventSem(handle, <timeout>)                 *
*                                                                        *
* Params:    handle - token returned from SysWaitEventSem                *
*                                                                        *
* Return:    result - return code from DosWaitEventSem                   *
*************************************************************************/

RexxRoutine2(int, SysWaitEventSem, uintptr_t, vhandle, OPTIONAL_int, timeout)
{
    RXSEMDATA *semdata = (RXSEMDATA *)vhandle;
    int rc = 0;

    if (timeout != 0)
    {
        /* this looping construct will cause us to wait longer than the */
        /* specified timeout due to the latency involved in the loop,   */
        /* but that cannot be helped                                    */
        while (timeout > 0)
        {
            rc = sem_trywait(semdata->handle);
            if (rc == 0)
            {
                break;
            }
            if (usleep(SEM_WAIT_PERIOD * 1000) == 0)
            {
                timeout -= SEM_WAIT_PERIOD;
            }
        }
    }
    else
    {
        rc = sem_wait(semdata->handle);
    }
    if (rc)
    {
        if (errno == EAGAIN)
        {
            return 121;
        }
        else if (errno == EINVAL)
        {
            return 6;
        }
    }
    return 0;
}


/*************************************************************************
* Function:  SysCreateMutexSem                                           *
*                                                                        *
* Syntax:    handle = SysCreateMutexSem(<name>)                          *
*                                                                        *
* Params:    name  - optional name for a event semaphore                 *
*                                                                        *
* Return:    handle - token used as a event sem handle for               *
*                     SysPostEventSem, SysClearEventSem,                 *
*                     SysCloseEventSem, and SysOpenEventSem              *
*            '' - Empty string in case of any error                      *
*************************************************************************/

RexxRoutine1(RexxObjectPtr, SysCreateMutexSem, OPTIONAL_CSTRING, name)
{
    RXSEMDATA *semdata;
    int rc;

    semdata = (RXSEMDATA *)malloc(sizeof(RXSEMDATA));
    if (semdata == NULL)
    {
        return context->String("");
    }
    if (strlen(name) == 0)
    {
        /* this is an unnamed semaphore */
        semdata->handle = (sem_t *)malloc(sizeof(sem_t));
        rc = sem_init(semdata->handle, 0, 0);
        if (rc == -1)
        {
            free(semdata);
            return context->String("");
        }
        semdata->named = false;
    }
    else
    {
        /* this is a named semaphore */
        semdata->handle = sem_open(name, (O_CREAT | O_EXCL), (S_IRWXU | S_IRWXG), 0);
        if (semdata->handle == SEM_FAILED)
        {
            free(semdata);
            return context->String("");
        }
        semdata->named = true;
    }
    rc = sem_post(semdata->handle);
    return context->Uintptr((uintptr_t)semdata);
}


/*************************************************************************
* Function:  SysOpenMutexSem                                             *
*                                                                        *
* Syntax:    result = SysOpenMutexSem(handle)                            *
*                                                                        *
* Params:    handle - token returned from SysCreateMutexSem              *
*                                                                        *
* Return:    result - return code from DosOpenEventSem                   *
*************************************************************************/

RexxRoutine1(uintptr_t, SysOpenMutexSem, CSTRING, name)
{
    RXSEMDATA *semdata;

    semdata = (RXSEMDATA *)malloc(sizeof(RXSEMDATA));
    if (semdata == NULL)
    {
        return 0;
    }
    semdata->handle = sem_open(name, 0);
    if (semdata->handle == SEM_FAILED)
    {
        return 0;
    }
    semdata->named = true;
    return (uintptr_t)semdata;
}


/*************************************************************************
* Function:  SysRequestMutexSem                                          *
*                                                                        *
* Syntax:    result = SysRequestMutexSem(handle, <timeout>)              *
*                                                                        *
* Params:    handle - token returned from SysRequestMutexSem             *
*                                                                        *
* Return:    result - return code from DosWaitEventSem                   *
*************************************************************************/

RexxRoutine2(int, SysRequestMutexSem, uintptr_t, vhandle, OPTIONAL_int, timeout)
{
    RXSEMDATA *semdata = (RXSEMDATA *)vhandle;
    int rc = 0;

    if (timeout != 0)
    {
        /* this looping construct will cause us to wait longer than the */
        /* specified timeout due to the latency involved in the loop,   */
        /* but that cannot be helped                                    */
        while (timeout > 0)
        {
            rc = sem_trywait(semdata->handle);
            if (rc == 0)
            {
                break;
            }
            if (usleep(SEM_WAIT_PERIOD * 1000) == 0)
            {
                timeout -= SEM_WAIT_PERIOD;
            }
        }
    }
    else
    {
        rc = sem_wait(semdata->handle);
    }
    if (rc)
    {
        if (errno == EAGAIN)
        {
            return 121;
        }
        else if (errno == EINVAL)
        {
            return 6;
        }
    }
    return 0;
}


/*************************************************************************
* Function:  SysReleaseMutexSem                                          *
*                                                                        *
* Syntax:    result = SysReleaseMutexSem(handle)                         *
*                                                                        *
* Params:    handle - token returned from SysCreateMutexSem              *
*                                                                        *
* Return:    result - return code from DosCloseEventSem                  *
*************************************************************************/

RexxRoutine1(int, SysReleaseMutexSem, uintptr_t, vhandle)
{
    RXSEMDATA *semdata = (RXSEMDATA *)vhandle;
    int rc;
    int val;

    rc = sem_getvalue(semdata->handle, &val);
    if (rc)
    {
        if (errno == EINVAL)
        {
            return 6;
        }
        else
        {
            return 288;
        }
    }
    if (val == 0)
    {
        rc = sem_post(semdata->handle);
        if (rc)
        {
            return 6;
        }
    }
    return 0;

}


/*************************************************************************
* Function:  SysCloseMutexSem                                            *
*                                                                        *
* Syntax:    result = SysCloseMutexSem(handle)                           *
*                                                                        *
* Params:    handle - token returned from SysCreateMutexSem              *
*                                                                        *
* Return:    result - return code from DosCloseEventSem                  *
*************************************************************************/

RexxRoutine1(int, SysCloseMutexSem, uintptr_t, vhandle)
{
    RXSEMDATA *semdata = (RXSEMDATA *)vhandle;

    if (semdata->named == false)
    {
        /* this is an unnamed semaphore so we must free the target */
        if (sem_destroy(semdata->handle))
        {
            if (errno == EINVAL)
            {
                return 6;
            }
            else if (errno)
            {
                return 102;
            }
        }
    }
    else
    {
        /* this is a named semaphore */
        if (sem_close(semdata->handle))
        {
            if (errno == EINVAL)
            {
                return 6;
            }
            else if (errno)
            {
                return 102;
            }
        }
    }
    free(semdata);
    return 0;
}


/*************************************************************************
* Function:  SysSetPriority                                              *
*                                                                        *
* Syntax:    result = SysSetPriority(Class, Level)                       *
*                                                                        *
* Params:    Class  - The priority class (0-4)                           *
*            Level  - Amount to change (-31 to +31)                      *
*                     (lower to higher priority)                         *
* Return:    0    for correct execution                                  *
*            304  for ERROR_INVALID_PDELTA                               *
*            307  for ERROR_INVALID_PCLASS                               *
*            derived from:                                               *
*            result - return code from DosSetPriority                    *
*                                                                        *
*************************************************************************/
RexxRoutine2(int, SysSetPriority, int32_t, pclass, int32_t, level)
{
    RexxReturnCode    rc;                        /* creation return code                */

    if (pclass == 0)                        /* class 0 -> no change               */
    {
        rc = 0;                             /* no error                           */
    }
    /* change the priority                */
    /* change according to delta          */
    else if (pclass > 0 && pclass <= 4)
    {
        int pid = getpid();                     /* current PID                        */

        /* current priority                   */
        int priority = getpriority(PRIO_PROCESS, getpid());

        /* Set new priority                   */
        setpriority(PRIO_PROCESS, getpid(), -level);
        rc = 0;
    }

    else
    {
        context->InvalidRoutine();
        return 0;
    }

    return rc;
}

/**
 * Retrieve an error message from the repository
 *
 * @param repository The named repository (can be null, which means retrieve the rexx error message)
 * @param setnum     The setnumber in the catalog to use
 * @param msgnum     The target message number
 *
 * @return The error message text or NULL if not found.
 */
const char* getErrorMessage(const char *repository, int setnum, int msgnum)
{
    // if this is the default or explicitly "rexx.cat", then we retrieve this
    // directly from the interpreter
    if (repository == NULL || strcmp(repository, REXXMESSAGEFILE))
    {
        return RexxGetErrorMessageByNumber(msgnum);
    }
#if defined( HAVE_CATOPEN )

#if defined( HAVE_SETLOCALE )
    setlocale(LC_ALL, "en_US");
#endif
    nl_catd catalog;                     /* catalog handle             */
/* open the catalog           */
    if ((catalog = catopen(repository, NL_CAT_LOCALE)) == (nl_catd)-1)
    {
        return "Error: Message catalog not found";
    }

    char *msg = catgets(catalog, setnum, (int)msgnum, "Error: Message catalog not open");
    catclose(catalog);                   /* close the catalog          */

    return *msg == '\0' ? "Error: Message not found" : msg;      // return the message or our error
#else
    // if no catalog support, we just return an error message
    return "Error: Message catalog (catopen) not supported";
#endif
}


/*************************************************************************
* Function:  SysGetMessage                                               *
*                                                                        *
* Syntax:    call SysGetMessage msgnum [,file] [,str1]...[,str9]         *
*                                                                        *
* Params:    file           - Name of message file to get message from.  *
*                              Default is OSO001.MSG.                    *
*            msgnum         - Number of message being queried.           *
*            str1 ... str9  - Insertion strings.  For messages which     *
*                              contain %1, %2, etc, the str1, str2, etc  *
*                              strings will be inserted (if given).      *
*                                                                        *
* Return:    The message with the inserted strings (if given).           *
* Note:      The set number ist always 1.
*************************************************************************/
RexxRoutine3(RexxStringObject, SysGetMessage, positive_wholenumber_t, msgnum, OPTIONAL_CSTRING, msgfile, ARGLIST, args)
{
    // this always uses one for the set number
    const char *message = getErrorMessage(msgfile, 1, msgnum);

    // go format the message with the substitutions.
    return formatMessage(context, message, args, 3);
}



/*************************************************************************
* Function:  SysGetMessageX                                              *
*                                                                        *
* Syntax:    call SysGetMessageX setnum, msgnum [,file] [,str1]...[,str9]*
*                                                                        *
* Params:    file           - Name of message file to get message from.  *
*                              Default is OSO001.MSG.                    *
*            msgnum         - Number of message being queried.           *
*            str1 ... str9  - Insertion strings.  For messages which     *
*                              contain %1, %2, etc, the str1, str2, etc  *
*                              strings will be inserted (if given).      *
*            setnum         - set number in the catalog                  *
*                                                                        *
* Return:    The message with the inserted strings (if given).           *
* Note:      This is a special Unix only version of SysGetMessage which  *
*            supports the selection of a set in the msg catalog.         *
*************************************************************************/

RexxRoutine4(RexxStringObject, SysGetMessageX, positive_wholenumber_t, setnum, positive_wholenumber_t, msgnum, OPTIONAL_CSTRING, msgfile, ARGLIST, args)
{
    // this always uses one for the set number
    const char *message = getErrorMessage(msgfile, (int)setnum, (int)msgnum);

    // go format the message with the substitutions.
    return formatMessage(context, message, args, 4);
}


/*************************************************************************
* Function:  SysGetKey                                                   *
*                                                                        *
* Syntax:    call SysGetKey [echo]                                       *
*                                                                        *
* Params:    echo - Either of the following:                             *
*                    'ECHO'   - Echo the inputted key (default).         *
*                    'NOECHO' - Do not echo the inputted key.            *
*                                                                        *
* Return:    The key striked.                                            *
*************************************************************************/

RexxRoutine1(RexxStringObject, SysGetKey, OPTIONAL_CSTRING, echoArg)
{
    bool echo = true;                  /* Set to false if we         */

    if (echoArg != NULL)                  /* validate arguments         */
    {
        if (!strcasecmp(echoArg, "NOECHO"))
        {
            echo = false;
        }
        else if (strcasecmp(echoArg, "ECHO"))
        {
            invalidOptionException(context, "SysGetKey", "echo", "'ECHO' or 'NOECHO'", echoArg);
        }
    }

    char buffer[4];

    getkey(buffer, echo);         /* call the complicated part  */
    return context->NewStringFromAsciiz(buffer);
}

/*************************************************************************
* Function:  SysFork                                                     *
*                                                                        *
* Description:   Function to migrate CREXX for AIX procedures.           *
*                                                                        *
* Syntax:    call SysFork()                                              *
*                                                                        *
* Return:    Process_ID   ( to parent child''s ID / to child the ID 0 )  *
*************************************************************************/

RexxRoutine0(int, SysFork)
{
    return fork();
}

/*************************************************************************
* Function:  SysWait                                                     *
*                                                                        *
* Description:   Function to migrate CREXX for AIX procedures.           *
*                                                                        *
* Syntax:    call SysWait()                                              *
*                                                                        *
* Return:    exit code of child                                          *
*************************************************************************/

RexxRoutine0(int, SysWait)
{
    int iStatus;
    wait(&iStatus);
    return iStatus;
}

/*************************************************************************
* Function:  SysCreatePipe                                               *
*                                                                        *
* Description:   Function to migrate CREXX for AIX procedures.           *
*                Function creates an unnamed pipe                        *
*                                                                        *
* Syntax:    call SysCreatePipe( Blocking | Nonblocking )                *
*                                                                        *
* Return:    'handle handle'     ( handle for read and handle for write )*
*************************************************************************/

RexxRoutine1(RexxStringObject, SysCreatePipe, OPTIONAL_CSTRING, blocking)
{
    int  iStatus;
    int  iaH[2];
    char cBlocking = 1;                       // default is blocking

    /* One arg, first char is 'b'?    */
    if (blocking != NULL)
    {
        if (blocking[0] == 'n' || blocking[0] == 'N')
        {
            cBlocking = 0;                         // non blocking needed
        }
        /* One arg, first char is 'n'?    */
        else if (blocking[0] != 'b' && blocking[0] != 'B')
        {
            invalidOptionException(context, "SysCreatePipe", "blocking", "'B' or 'N'", blocking);
        }
    }


    if (pipe(iaH))                             /* Create the pipe            */
    {
        perror("*** ERROR: Creating pipe");      /* pipe creation failed       */
        return context->NullString();
    }

    if (!cBlocking)                            /* Non-blocking?              */
    {
        /* Get file status flags ---------- */
        iStatus = fcntl(iaH[0], F_GETFL, NULL);
        iStatus |= O_NONBLOCK;                   /* Turn on NONBLOCK flag      */
        /* Does set work? ----------------- */
        if (fcntl(iaH[0], F_SETFL, iStatus) == -1)
        {
            perror("*** ERROR: Setting NONBLOCK flag");  /* No, tell user        */
            close(iaH[0]);
            close(iaH[1]);                /* Close pipes          */
            return context->NullString();
        }
    }

    char retstr[100];
    snprintf(retstr, sizeof(retstr), "%d %d", iaH[0], iaH[1]); /* Create return string */
    return context->NewStringFromAsciiz(retstr);
}


/*************************************************************************
* Function:  SysGetFileDateTime                                          *
*                                                                        *
* Syntax:    result = SysGetFileDateTime(filename [,timesel])            *
* Params:    filename - name of the file to query                        *
*            timesel  - What filetime to query: Access/Write             *
*                                                                        *
* Return:    -1 - file date/time query failed                            *
*            other - date and time as YYYY-MM-DD HH:MM:SS                *
*************************************************************************/

RexxRoutine2(RexxObjectPtr, SysGetFileDateTime, CSTRING, file, OPTIONAL_CSTRING, timesel)
{
    struct    stat64 buf;
    struct    tm *newtime;


    RoutineQualifiedName qualifiedName(context, file);

    if (stat64(qualifiedName, &buf) < 0)
    {
        return context->Int32ToObject(-1);
    }

    if (timesel != NULL)
    {
        switch (timesel[0])
        {
            case 'a':
            case 'A':
                newtime = localtime(&(buf.st_atime));
                break;
            case 'w':
            case 'W':
                newtime = localtime(&(buf.st_mtime));

                break;
            default:
                invalidOptionException(context, "SysGetFileDateTime", "time selector", "'A' or 'W'", timesel);
                context->InvalidRoutine();
                return NULLOBJECT;
        }
    }
    else
    {
        newtime = localtime(&(buf.st_mtime));
    }

    newtime->tm_year += 1900;
    newtime->tm_mon += 1;

    char retstr[100];

    snprintf(retstr, sizeof(retstr), "%4d-%02d-%02d %02d:%02d:%02d",
             newtime->tm_year,
             newtime->tm_mon,
             newtime->tm_mday,
             newtime->tm_hour,
             newtime->tm_min,
             newtime->tm_sec);
    return context->NewStringFromAsciiz(retstr);
}


/*************************************************************************
* Function:  SysSetFileDateTime                                          *
*                                                                        *
* Syntax:    result = SysSetFileDateTime(filename [,newdate] [,newtime]) *
*                                                                        *
* Params:    filename - name of the file to update                       *
*            newdate  - new date to set in format YYYY-MM-DD (YYYY>1800) *
*            newtime  - new time to set in format HH:MM:SS               *
*                                                                        *
* Return:    0 - file date/time was updated correctly                    *
*            -1 - failure attribute update                               *
*************************************************************************/

RexxRoutine3(int, SysSetFileDateTime, CSTRING, filename, OPTIONAL_CSTRING, newdateSet, OPTIONAL_CSTRING, newtimeSet)
{
    struct utimbuf timebuf;
    struct tm *newtime;
    time_t ltime;
    struct stat64 buf;

    RoutineQualifiedName qualifiedName(context, filename);

    if (stat64(qualifiedName, &buf) < 0)
    {
        return -1;
    }


    if (newdateSet == NULL && newtimeSet == NULL)
    {
        time(&ltime);
        timebuf.modtime = ltime;
        return utime(qualifiedName, &timebuf) < 0 ? -1 : 0;
    }
    else
    {
        newtime = localtime(&(buf.st_mtime));
        if (newdateSet != NULL)
        {

            /* parse new date */
            if (sscanf(newdateSet, "%4d-%2d-%2d", &newtime->tm_year, &newtime->tm_mon, &newtime->tm_mday) != 3)
            {
                return -1;
            }
            newtime->tm_year -= 1900;
            newtime->tm_mon -= 1;
        }
        if (newtimeSet)
        {
            /* parse new time */
            if (sscanf(newtimeSet, "%2d:%2d:%2d", &newtime->tm_hour, &newtime->tm_min, &newtime->tm_sec) != 3)
            {
                return -1;
            }
        }
        ltime = mktime(newtime);
        timebuf.modtime = ltime;

        return utime(qualifiedName, &timebuf) < 0 ? -1 : 0;
    }
}


/**************************************************************************
* Function:  SysQueryProcess                                              *
*                                                                         *
* Params:    "PID"       - (default) returns current process ID           *
*    NEW:    "PPID"      -  returns parent of current process ID          *
*    NEW:    "PGID"      -  returns group ID of current process           *
*    NO      "TID"       -  returns current thread ID                     *
*    YES     "PPRIO"     -  returns current process priority              *
*    NO      "TPRIO"     -  returns current thread priority               *
*    YES     "PTIME"     -  returns current process times                 *
*    NO      "TTIME"     -  returns current thread times                  *
*    NEW:    "PMEM"      -  returns current process max memory size RSS   *
*    NEW:    "PSWAPS"    -  returns current process number of swaps out   *
*    NEW:    "PRCVDSIG"  -  returns current process received signals      *
***************************************************************************/

RexxRoutine1(RexxObjectPtr, SysQueryProcess, OPTIONAL_CSTRING, selector)
{
    unsigned int uiUsedCPUTime  = 0;
    unsigned int uiUsedCPUmsec  = 0;
    unsigned int uiUsedHours    = 0;
    unsigned int uiUsedMinutes  = 0;
    unsigned int uiUsedSeconds  = 0;
    char timebuf[40];
    int iRc = 0;

    if (selector == NULL || !strcasecmp(selector, "PID"))
    {
        return context->Int32ToObject(getpid());
    }
    else if (!strcasecmp(selector, "PPID"))
    {
        return context->Int32ToObject(getppid());
    }
    else if (!strcasecmp(selector, "PGID"))
    {
        return context->Int32ToObject(getpgid(getppid()));
    }
    else if (!strcasecmp(selector, "PPRIO"))
    {
        return context->Int32ToObject(getpriority(PRIO_PROCESS, 0));
    }

    struct rusage struResUse;

    /* ----------------------------------------------------------------- */
    /* Get process usage data and keep calls together at the end of      */
    /* the function SysQueryProcess.                                     */
    iRc = getrusage(RUSAGE_SELF, &struResUse);
    if (iRc)
    {
        char buffer[100];
        snprintf(buffer, sizeof(buffer), "System error; errno = %d", errno);
        return context->NewStringFromAsciiz(buffer);
    }
    if (!strcasecmp(selector, "PTIME")) /* Calculate the used CPU time*/
    {
        uiUsedCPUmsec  = (unsigned int)struResUse.ru_utime.tv_usec / 1000;
        uiUsedCPUmsec += (unsigned int)struResUse.ru_stime.tv_usec / 1000;
        if (uiUsedCPUmsec >= 1000)
        {
            uiUsedCPUTime = uiUsedCPUmsec / 1000;
            uiUsedCPUmsec = uiUsedCPUmsec % 1000;
        }
        uiUsedCPUTime += (unsigned int)struResUse.ru_utime.tv_sec;
        uiUsedCPUTime += (unsigned int)struResUse.ru_stime.tv_sec;
        uiUsedHours   = uiUsedCPUTime / 3600;
        uiUsedMinutes = uiUsedCPUTime / 60;
        if (uiUsedMinutes >= 60)
        {
            uiUsedMinutes = uiUsedMinutes % 60;
        }
        if (uiUsedCPUTime >= 60)
        {
            uiUsedSeconds = uiUsedCPUTime % 60;
        }
        else
        {
            uiUsedSeconds = uiUsedCPUTime;
        }

        char buffer[200];
        snprintf(buffer, sizeof(buffer), "CPU_Time Summary: %2d:%.2d:%.2d:%.3d  Kernel:",
                 uiUsedHours, uiUsedMinutes, uiUsedSeconds, uiUsedCPUmsec);

        uiUsedCPUmsec = (unsigned int)struResUse.ru_stime.tv_usec / 1000;
        uiUsedCPUTime = (unsigned int)struResUse.ru_stime.tv_sec;
        uiUsedHours   = uiUsedCPUTime / 3600;
        uiUsedMinutes = uiUsedCPUTime / 60;
        if (uiUsedMinutes >= 60)
        {
            uiUsedMinutes = uiUsedMinutes % 60;
        }
        if (uiUsedCPUTime >= 60)
        {
            uiUsedSeconds = uiUsedCPUTime % 60;
        }
        else
        {
            uiUsedSeconds = uiUsedCPUTime;
        }

        size_t currLen = strlen(buffer);

        snprintf(buffer + currLen, sizeof(buffer) - currLen, " %2d:%.2d:%.2d:%.3d  User:", uiUsedHours,
                 uiUsedMinutes, uiUsedSeconds, uiUsedCPUmsec);

        currLen = strlen(buffer);

        uiUsedCPUmsec = (unsigned int)struResUse.ru_utime.tv_usec / 1000;
        uiUsedCPUTime = (unsigned int)struResUse.ru_utime.tv_sec;
        uiUsedHours   = uiUsedCPUTime / 3600;
        uiUsedMinutes = uiUsedCPUTime / 60;
        if (uiUsedMinutes >= 60)
        {
            uiUsedMinutes = uiUsedMinutes % 60;
        }
        if (uiUsedCPUTime >= 60)
        {
            uiUsedSeconds = uiUsedCPUTime % 60;
        }
        else
        {
            uiUsedSeconds = uiUsedCPUTime;
        }

        snprintf(buffer + currLen, sizeof(buffer) - currLen, " %2d:%.2d:%.2d:%.3d", uiUsedHours,
                 uiUsedMinutes, uiUsedSeconds, uiUsedCPUmsec);

        return context->NewStringFromAsciiz(buffer);
    }
    else if (!strcasecmp(selector, "PMEM"))  /* Show max memory RSS used   */
    {
        char buffer[100];
        snprintf(buffer, sizeof(buffer), "Max_Memory_RSS: %ld", struResUse.ru_maxrss);
        return context->NewStringFromAsciiz(buffer);
    }
    else if (!strcasecmp(selector, "PSWAPS")) /* Memory has been swapped   */
    {
        char buffer[100];
        snprintf(buffer, sizeof(buffer), "Memory_swaps: %ld", struResUse.ru_nswap);
        return context->NewStringFromAsciiz(buffer);
    }
    else if (!strcasecmp(selector, "PRCVDSIG")) /* Process received signals*/
    {
        char buffer[100];
        snprintf(buffer, sizeof(buffer), "Received_signals: %ld", struResUse.ru_nsignals);
        return context->NewStringFromAsciiz(buffer);
    }

    context->InvalidRoutine();
    return NULLOBJECT;
}

/*************************************************************************
* Function:  SysGetErrortext                                             *
*                                                                        *
* Syntax:    call SysGetErrortext errnumber                              *
*                                                                        *
* Params:    errnumber - error number to be described                    *
*                                                                        *
* Return:    Description or empty string                                 *
*************************************************************************/

RexxRoutine1(RexxStringObject, SysGetErrorText, int32_t, errnum)
{
    const char *errmsg = strerror(errnum);
    if (errmsg == NULL)
    {
        return context->NullString();
    }
    else
    {
        return context->NewStringFromAsciiz(errmsg);
    }
}
