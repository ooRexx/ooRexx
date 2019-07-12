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
/*                                                                            */
/* Non-platform-specific rexxutil functions                                   */
/*                                                                            */
/******************************************************************************/

#include <time.h>
#include <algorithm>
#include <math.h>
#include <ctype.h>
#include <cmath>
#include <cfloat>
#include "oorexxapi.h"
#include "PackageManager.hpp"
#include "RexxUtilCommon.hpp"
#include "RexxInternalApis.h"
#include "ExternalFileBuffer.hpp"
#include "SysFile.hpp"
#include "SysFileSystem.hpp"
#include "SystemInterpreter.hpp"
#include "Utilities.hpp"
#include "Numerics.hpp"


const int INVALID_FILE_NAME = 123;       // a return value for a SysFileTree name problem

const char TreeFinder::AttributeMask::maskChars[6] = "ADHRS";

/**
 * A simple class for reading lines from a file.
 */
class LineReader
{
 public:
     LineReader() : buffer(NULL), bufferSize(0), file()
     { }
     ~LineReader()
     {
         close();
     }

     /**
      * Open the file and set up for reading
      *
      * @param fileName The name of the file
      *
      * @return true if we were able to open the file, false for any failures.
      */
     bool open(const char *fileName)
     {
         // if this is a directory or we can't open the file, return
         if (SysFileSystem::isDirectory(fileName) ||
             !file.open(fileName, RX_O_RDONLY, RX_S_IREAD, RX_SH_DENYWR))
         {
             return false;
         }
         bufferSize = InitialBufferSize;
         buffer = (char *)malloc(bufferSize);
         return buffer != NULL;
     }


     /**
      * Read an entire line into buffer, expanding buffer as necessary.
      *
      * @return true if a line could be read, false otherwise.
      */
     bool getLine(const char *&line, size_t &size)
     {
         size = 0;
         // loop until an entire line has been read.
         for (;;)
         {
             line = buffer;
             size_t bytesRead = 0;
             if (!file.gets(buffer + size, bufferSize - size, bytesRead))
             {
                 return false;
             }
             // update the size of the line now
             size += bytesRead;

             // Check for new line character first.  If we are at eof and the last
             // line ended in a new line, we don't want the \n in the returned
             // string.

             // If we have a new line character in the last position, we have
             // a line.  The gets() function has translated crlf sequences into
             // single lf characters.
             if (buffer[size - 1] == '\n')
             {
                 size--;
                 return true;
             }

             // No new line but we hit end of file reading this?  This will be the
             // entire line then.
             if (file.atEof())
             {
                 return true;
             }
             bufferSize += BufferExpansionSize;
             if (!buffer.realloc(bufferSize))
             {
                 return false;
             }
         }
     }


     /**
      * Close the read operation.
      */
     void close()
     {
         file.close();
     }


 protected:
     // the initial size of our buffer
     const size_t InitialBufferSize = 0x01000;
     // amount we extend the buffer by if a line is longer
     const size_t BufferExpansionSize = 0x01000;

     AutoFree buffer;              // the current buffer
     size_t   bufferSize;          // current size of the buffer
     SysFile  file;                // the file we are reading
};


/**
 * Generate a unique file name based off of a template.
 */
void getUniqueFileName(const char *fileTemplate, char filler, FileNameBuffer &file)
{
    int j = 0;
    int max = 1;

    // count the number of filler chars we have
    for (int x = 0; fileTemplate[x] != 0; x++)
    {
        if (fileTemplate[x] == filler)
        {
            max = max * 10;
            j++;
        }
    }

    // return NULL string if no or more than the allowed number of filler chars
    if (j == 0 || j > 9)
    {
        file = "";
        return;
    }

    // generate a random starting point
    // we don't seed as this gives us a better distribution
    size_t num = rand();
    if (RAND_MAX < max)
    {   // MAX_RAND is guaranteed to be at least 32767
        num = num * 32768 + rand();
    }
    num = num % max;

    // create a working copy of the template that we can alter
    AutoFree buffer = (char *)strdup(fileTemplate);
    // remember our starting number in case we loop around
    size_t start = num;

    // loop until we find a unique name
    while (true)
    {
        char numstr[10];
        // get the random number as a set of 9 character digits
        snprintf(numstr, sizeof(numstr), "%09zu", num);

        // now substitute our generated characters for the filler characters
        int i = 9 - j;

        for (int x = 0; fileTemplate[x] != 0; x++)
        {
            // if we have a filler, fill it in
            if (fileTemplate[x] == filler)
            {
                buffer[x] = numstr[i++];
            }
        }

        // get this as a fully qualified name
        file.truncate(0); // qualifyStreamName requires empty string
        SysFileSystem::qualifyStreamName(buffer, file);

        // if there's no matching file or directory, we're finished.
        if (!SysFileSystem::exists(file))
        {
            return;
        }

        // generate a new number for filling in the name
        num = (num + 1) % max;

        // if we've wrapped around to where we started, time to give up
        if (num == start)
        {
            file = "";
            return;
        }
    }
}


/**
 * SysFileTree() implementation.  Searches for files in a directory tree
 * matching the specified search pattern.
 *
 * @param  fSpec  [required] The search pattern, may contain glob characters
 *                 and full or partial path informattion. E.g., *.bat, or
 *                 ../../abc*.txt, or C:\temp.  May not contain illegal file name
 *                 characters which are: ", <, >, |, and :  The semicolon is
 *                 only legal if it is exactly the second character.  Do not use
 *                 a double quote in fSpec, it is not needed and is taken as a
 *                 character in a file name, which is an illegal character.
 *
 * @param  files  [required] A stem to contain the returned results.  On return,
 *                files.0 contains the count N of found files and files.1
 *                through files.N will contain the found files.
 *
 * @param  opts   [optional] Any combination of the following letters that
 *                specify how the search takes place, or how the returned found
 *                file line is formatted.  Case is not significant:
 *
 *                  'B' - Search for files and directories.
 *                  'D' - Search for directories only.
 *                  'F' - Search for files only.
 *                  'O' - Only output file names.
 *                  'S' - Recursively scan subdirectories.
 *                  'T' - Combine time & date fields into one.
 *                  'L' - Long time format
 *                  'H' - Large file size format
 *                  'I' - Case Insensitive search.
 *
 *                The default is 'B' using normal time (neither 'T' nor 'L'.)
 *                The 'I'option is meaningless on Windows.
 *
 * @param targetAttr  [optional] Target attribute mask.  Only files with these
 *                    attributes will be searched for.  The default is to ignore
 *                    the attributes of the files found, so all files found are
 *                    returned.
 *
 * @param newAttr     [optional] New attribute mask.  Each found file will have
 *                    its attributes set (changed) to match this mask.  The
 *                    default is to not change any attributes.
 *
 * @return  0 on success, non-zero on error.  For all errors, a condition is
 *          raised.
 */
RexxRoutine5(uint32_t, SysFileTree, CSTRING, fileSpec, RexxStemObject, files, OPTIONAL_CSTRING, opts,
             OPTIONAL_CSTRING, targetAttr, OPTIONAL_CSTRING, newAttr)
{
    try
    {
        // initialize the search finder with the argument data
        TreeFinder finder(context, fileSpec, files, opts, targetAttr, newAttr);

        // go perform the search
        finder.findFiles();
    }
    catch (TreeFinder::TreeFinderException e)
    {
        if (e == TreeFinder::InvalidFileName)
        {
            return INVALID_FILE_NAME;
        }
    }
    return 0;
}


// this next section are platform-independent methods for the TreeFinder class.
// platform-specific methods are located in the appropriate SysRexxUtil.cpp file.
TreeFinder::TreeFinder(RexxCallContext *c, const char *f, RexxStemObject s, const char *opts, const char *targetAttr, const char *newAttr) :
    context(c), count(0), files(s), filePath(c), fileSpec(c),
    foundFile(c), foundFileLine(c), nameSpec(c)
{
    // clear any existing count to be zero before we start looking
    context->SetStemArrayElement(files, 0, context->WholeNumber(0));
    // save the initial file spec
    fileSpec = f;
    // validate the file specification
    validateFileSpec();
    // process the SysFileTreeOptions
    getOptions(opts);

    // validate the new and target attributes
    parseMask(targetAttr, targetAttributes, 4);
    parseMask(newAttr, newAttributes, 5);
}


/**
 * The destructor for the tree finder, which finalizes the return stem value.
 */
TreeFinder::~TreeFinder()
{
    // make sure we finalized the count before returning.
    context->SetStemArrayElement(files, 0, context->WholeNumber(count));
}


/**
 * Main method for driving the validation of options and file names and
 * then performing the file searches.
 *
 * @return Any approriate error codes, or 0 for success.
 */
void TreeFinder::findFiles()
{
    // Get the full path segment and the file name segment by expanding the
    // file specification string.  It seems highly unlikely, but possible, that
    // this could fail.
    getFullPath();

    // now start the search
    recursiveFindFile(filePath);
}


/**
 * The file specification consists of the search string as sent by the Rexx
 * user, with possibly some glob characters added.
 *
 * If the file speicfication ends in a slash ('\') or a period ('.') or two
 * periods ('..'), then a wild card specification ('*') is
 * appended.
 *
 * However, note that there is also the case where a single '.' at the end of
 * the file specification is not used as a directory specifier, but rather is
 * tacked on to the end of a file name.
 *
 * Windows has a sometimes used convention that a '.' at the end of a file name
 * can be used to indicate the file has no extension. For example, given a file
 * named: MyFile a command of "dir MyFile." will produce a listing of "MyFile".
 *
 * In this case we want to leave the file specification alone. that is, do not
 * append a "*.*". A command of "dir *." will produce a directory listing of all
 * files that do not have an extension.
 */
void TreeFinder::validateFileSpec()
{
    size_t len = fileSpec.length();
    if (len == 0)
    {
        nullStringException(context, "SysFileTree", 1);
    }

    // apply any platform-specific rules to the file spec name.
    validateFileSpecName();

    // apply platform rules to adjust for directories.
    adjustDirectory();

    // perform any adjustments needed to the spec
    adjustFileSpec();
}


/**
 * Checks the validity of an attribute mask argument and converts the character
 * based mask into an integer based mask.
 *
 * @param mask   The mask specified on the call.
 * @param flags  The set of flags to set.
 * @param argPos The argument position for error reporting
 */
void TreeFinder::parseMask(const char *mask, AttributeMask &flags, size_t argPos)
{
    if (mask != NULL && strlen(mask) > 0)
    {
        if (strlen(mask) > 5)
        {
            badMaskException(argPos, mask);
        }

        if (!flags.parseMask(mask))
        {
            badMaskException(argPos, mask);
        }
    }
}


/**
 * Throw an error for bad options.
 *
 * @param actual The specified options string
 */
void TreeFinder::badSFTOptsException(const char *actual)
{
    char buf[256] = { 0 };
    snprintf(buf, sizeof(buf),
             "SysFileTree options argument must be a combination of F, D, B, S, T, L, I, O, or H; found \"%s\"",
             actual);

    context->ThrowException1(Rexx_Error_Incorrect_call_user_defined, context->String(buf));
}

/**
 * Throw an exception for an invalid mask argument.
 *
 * @param pos    The argument position
 * @param actual The specified argument
 */
void TreeFinder::badMaskException(size_t pos, const char *actual)
{
    char buf[256] = { 0 };
    snprintf(buf, sizeof(buf),
             "SysFileTree argument %zd must be 5 characters or less in length containing only '+', '-', or '*'; found \"%s\"",
             pos, actual);

    context->ThrowException1(Rexx_Error_Incorrect_call_user_defined, context->String(buf));
}


/**
 * Checks the validity of the options argument to SysFileTree and converts the
 * character based argument to the proper set of flags.
 *
 * @param opts   The call arguments
 *
 */
void TreeFinder::getOptions(const char *opts)
{
    // by default, we return both file and directories
    options.set(DO_FILES, DO_DIRS);

    if (opts != NULL)
    {
        // a NULL string is not a valid option
        if (strlen(opts) == 0)
        {
            nullStringException(context, "SysFileTree", 3);
        }

        if (!goodOpts(opts))
        {
            badSFTOptsException(opts);
        }
    }
}


/**
 * Determines the options by converting the character based argument to the
 * correct set of flags.
 *
 * @param opts   The options string
 *
 * @return true if the options were ok, false if any invalid options were encountered.
 */
bool TreeFinder::goodOpts(const char *opts)
{
    while (*opts)
    {
        switch (toupper(*opts))
        {
            case 'S':                      // recurse into subdirectories
                options[RECURSE] = true;
                break;

            case 'O':                      // only return names
                options[NAME_ONLY] = true;
                break;

            case 'T':                      // use short time format, ignored if L is used
                options[EDITABLE_TIME] = true;
                break;

            case 'L':                      // use long time format
                options[LONG_TIME] = true;
                break;

            case 'F':                      // include only files
                options[DO_DIRS] = false;
                options[DO_FILES] = true;
                break;

            case 'D':                      // include only directories
                options[DO_DIRS] = true;
                options[DO_FILES] = false;
                break;

            case 'B':                      // include both files and directories
                options[DO_DIRS] = true;
                options[DO_FILES] = true;
                break;

            case 'I':                      // case insensitive? no op on Windows
                options[CASELESS] = true;
                break;

            case 'H':                      // use a larger field for the file size.
                options[LONG_SIZE] = true;
                break;


            default:                       // error, unknown option
                return false;
        }
        opts++;
    }

    return true;
}


/**
 * This function expands the file spec into its full path name.
 * The full path name is then split into the path portion and
 * the file name portion.  The path portion is then returned in
 * filePath and the file name portion is returned in nameSpec.
 *
 * The filePOath portion will end with the PathDelimiter
 * character if the filespec contains a path.
 *
 */
void TreeFinder::getFullPath()
{
    // Find the position of the last slash in fSpec
    int lastSlashPos = findDirectoryEnd();

    // If lastSlashPos is less than 0, then there is no directory present in
    // fileSpec.
    if (lastSlashPos < 0)
    {
        // this does not have a path on it, so add one
        expandNonPath2fullPath();
    }
    else
    {
        // we need to split the path off from the file spec
        expandPath2fullPath(lastSlashPos);
    }

    // the above operations have also split off the nameSpec section
}


/**
 * Create a path from a file spec that does not include path information.
 *
 * @param path   The returned path
 */
void TreeFinder::expandNonPath2fullPath()
{
    // fileSpec could be a drive designator (Windows only)
    if (!checkNonPathDrive())
    {
        // start out with the current directory for the file path
        SysFileSystem::getCurrentDirectory(filePath);

        // If fileSpec is exactly "." or ".." then add this to the end of the current path.
        if (fileSpec == ".")
        {
            // just a wild card for the current name we're searching for
            nameSpec = "*";
        }
        // previous directory, add the path delimiter to the
        // end of the path and use "*" for the search path
        else if (fileSpec == "..")
        {
            // make sure we have a path delimiter on the end
            // add the .., the final path delimiter is added below
            filePath += "..";
            // the name spec if again a wild card
            nameSpec = "*";
        }
        else
        {
            // copy every thing else over to the name spec
            nameSpec = fileSpec;
        }
    }

    // If we need a trailing slash in the file path, add one.
    filePath.addFinalPathDelimiter();
}


/**
 * Splits the the file spec into a path portion and a name
 * portion. There is at least one directory delimiter in the
 * fileSpec.
 *
 * @param lastSlashPos
 *               Position of the last directory character in the file spec.
 */
void TreeFinder::expandPath2fullPath(size_t lastSlashPos)
{
    // we have a spec with at least one directory delimiter, so we need
    // to split this into name and directory portion. The name part is easy,
    // the path portion may require a little system work.
    if (fileSpec.endsWith(SysFileSystem::PathDelimiter))
    {
        // the spec has no file portion, so use a wild card for the
        // name specification
        nameSpec = "*";
    }
    else
    {
        // copy over the trailing part
        nameSpec = (const char *)fileSpec + (lastSlashPos + 1);
    }
    // ok, the leading part we copy over to the filePath and then see if
    // there are any additional platform-specific fixups required.
    filePath.set(fileSpec, lastSlashPos + 1);

    // now do any platform-specific resolution that might be needed here
    fixupFilePath();
}


/**
 * Finds all files matching a file specification, formats a file name line and
 * adds the formatted line to a stem.  Much of the data to complete this
 * operation is contained in the treeData struct.
 *
 * This is a recursive function that may search through subdirectories if the
 * recurse option is used.
 *
 * @param path        Current directory we are searching.
 *
 */
void TreeFinder::recursiveFindFile(FileNameBuffer &path)
{
    RoutineFileNameBuffer tempFileName(context, path.length()+ nameSpec.length()+ 1);

    // get a file iterator to search through the names
    SysFileIterator finder(path, nameSpec, tempFileName, options[CASELESS]);
    // the attributes returned for each located file
    SysFileIterator::FileAttributes attributes;

    while (finder.hasNext())
    {
        // we can just use the temp name buffer now
        finder.next(tempFileName, attributes);
        // we skip the dot directories. We're already searching the first, and
        // the second would send us backwards
        if (tempFileName == "." || tempFileName == "..")
        {
            continue;
        }

        // compose the full name of the result
        foundFile = path;
        foundFile += tempFileName;

        // go check this file to see if it is a good match. If it is, it will be
        // directly added to the results.
        checkFile(attributes);
    }

    finder.close();

    // is this a recursive request?
    //
    if (options[RECURSE])
    {
        // reset the pattern to search for everything at this level.
        tempFileName = path;
        // get a file iterator to search through the names
        SysFileIterator dirFinder(path, NULL, tempFileName, false);

        RoutineFileNameBuffer directoryName(context);

        while (dirFinder.hasNext())
        {
            // we can use the temp name buffer now
            dirFinder.next(tempFileName, attributes);

            // skip non directories
            if (!attributes.isDirectory())
            {
                continue;
            }

            // we skip the dot directories. We're already searching the first, and
            // the second would send us backwards
            if (tempFileName == "." || tempFileName == "..")
            {
                continue;
            }

            // build a new search path
            directoryName = path;
            directoryName += tempFileName;

            // add a path delimiter to the end for the search
            directoryName += SysFileSystem::PathDelimiter;

            // Search the next level.
            recursiveFindFile(directoryName);
        }
        // close the directory finder
        dirFinder.close();
    }
}


/**
 * add a result to the return stem
 *
 * @param v      The string value to add as the next stem element.
 */
void TreeFinder::addResult(const char *v)
{
    RexxStringObject t = context->String(v);

    // Add the file name to the stem and be done with it.
    count++;
    context->SetStemArrayElement(files, count, t);
    context->ReleaseLocalReference(t);
}


/**
 * Determines if the string 'needle' is in the
 * string 'haystack' by returning it's position or
 * a NULL if not found.  The length of haystack and
 * needle are given by 'hlen' and 'nlen' respectively.
 *
 * If 'sensitive' is true, then the search is case
 * sensitive, else it is case insensitive.
 *
 * @param haystack  The haystack to search in.
 * @param needle    The needle to search for
 * @param hlen      the length of the haystack string
 * @param nlen      The lenght of the needle string
 * @param sensitive if true, a case sensitive search is performed. if false, the search is case insensitive.
 *
 * @return A pointer to the first match position or NULL if the string is not found.
 */
const char* mystrstr(const char *haystack, const char *needle, size_t hlen, size_t nlen, bool sensitive)
{
    // we never return a hit for a null string search or if the needle is longer than the haystack
    if (nlen == 0 || nlen > hlen)
    {
        return NULL;
    }

    // different loops for the sensitive and insensitive searches
    if (sensitive)
    {
        // we scan, looking for a hit on the first character
        char firstChar = needle[0];

        size_t current = 0;
        // this is maximum number of places we can get a hit
        size_t limit = hlen - nlen + 1;
        for (current = 0; current < limit; current++)
        {
            // if we have a hit on the first character, check the entire string
            if (firstChar == haystack[current])
            {
                // if everything compares, return the hit
                if (memcmp(haystack + current, needle, nlen) == 0)
                {
                    return haystack + current;
                }
            }
        }
    }
    else
    {
        // we scan, looking for a hit on the first character
        char firstChar = toupper(needle[0]);

        size_t current = 0;
        // this is maximum number of places we can get a hit
        size_t limit = hlen - nlen + 1;
        for (current = 0; current < limit; current++)
        {
            // if we have a hit on the first character, check the entire string
            if (firstChar == toupper(haystack[current]))
            {
                // if everything compares, return the hit
                if (Utilities::memicmp(haystack + current, needle, nlen) == 0)
                {
                    return haystack + current;
                }
            }
        }
    }
    // if the loops terminate, there are no hits
    return NULL;
}



/*************************************************************************
* Function:  SysDropFuncs                                                *
*                                                                        *
* Syntax:    call SysDropFuncs                                           *
*                                                                        *
* Return:    NO_UTIL_ERROR - Successful.                                 *
*************************************************************************/
RexxRoutine0(int, SysDropFuncs)
{
    // this is a nop
    return 0;
}


/*************************************************************************
* Function:  SysLoadFuncs                                                *
*                                                                        *
* Syntax:    call SysLoadFuncs [option]                                  *
*                                                                        *
* Params:    none                                                        *
*                                                                        *
* Return:    null string                                                 *
*************************************************************************/
RexxRoutine0(int, SysLoadFuncs)
{
    // this is a nop
    return 0;
}


/*************************************************************************
* Function:  SysAddRexxMacro                                             *
*                                                                        *
* Syntax:    result = SysAddRexxMacro(name, file, <order>)               *
*                                                                        *
* Params:    name   - loaded name of the macro file                      *
*            file   - file containing the loaded macro                   *
*            order  - Either 'B'efore or 'A'fter.  The default is 'B'    *
*                                                                        *
* Return:    return code from RexxAddMacro                               *
*************************************************************************/

RexxRoutine3(int, SysAddRexxMacro, CSTRING, name, CSTRING, file, OPTIONAL_CSTRING, option)
{
    size_t position;         /* added position             */

    position = RXMACRO_SEARCH_BEFORE;    /* set default search position*/
    if (option != NULL)                  /* have an option?            */
    {
        switch (*option)
        {
            case 'B':     // 'B'efore
            case 'b':
                position = RXMACRO_SEARCH_BEFORE;
                break;

            case 'A':     // 'A'fter
            case 'a':
                position = RXMACRO_SEARCH_AFTER;
                break;

            default:
            {
                invalidOptionException(context, "SysAddRexxMacro", "order", "'A' or 'B'", option);
            }
        }
    }
    // try to add the macro
    return (int)RexxAddMacro(name, file, position);
}


/*************************************************************************
* Function:  SysReorderRexxMacro                                         *
*                                                                        *
* Syntax:    result = SysReorderRexxMacro(name, order)                   *
*                                                                        *
* Params:    name   - loaded name of the macro file                      *
*            order  - Either 'B'efore or 'A'fter.                        *
*                                                                        *
* Return:    return code from RexxReorderMacro                           *
*************************************************************************/
RexxRoutine2(int, SysReorderRexxMacro, CSTRING, name, CSTRING, option)
{
    size_t position;        /* added position             */

    switch (*option)
    {
        case 'B':     // 'B'efore
        case 'b':
            position = RXMACRO_SEARCH_BEFORE;
            break;

        case 'A':     // 'A'fter
        case 'a':
            position = RXMACRO_SEARCH_AFTER;
            break;

        default:
            invalidOptionException(context, "SysReorderRexxMacro", "order", "'A' or 'B'", option);
            return 0;
    }
    return (int)RexxReorderMacro(name, position);
}


/*************************************************************************
* Function:  SysDropRexxMacro                                            *
*                                                                        *
* Syntax:    result = SysDropRexxMacro(name)                             *
*                                                                        *
* Params:    name   - name of the macro space function                   *
*                                                                        *
* Return:    return code from RexxDropMacro                              *
*************************************************************************/
RexxRoutine1(int, SysDropRexxMacro, CSTRING, name)
{
    return (int)RexxDropMacro(name);
}


/*************************************************************************
* Function:  SysQueryRexxMacro                                           *
*                                                                        *
* Syntax:    result = SysQueryRexxMacro(name)                            *
*                                                                        *
* Params:    name   - name of the macro space function                   *
*                                                                        *
* Return:    position of the macro ('B' or 'A'), returns null for errors.*
*************************************************************************/
RexxRoutine1(CSTRING, SysQueryRexxMacro, CSTRING, name)
{
    unsigned short position;         /* returned position          */

    if (RexxQueryMacro(name, &position) != 0)
    {
        return "";
    }
    // before?
    if (position == RXMACRO_SEARCH_BEFORE)
    {
        return "B";
    }
    else
    {
        return "A";                    /* must be 'A'fter            */
    }
}


/*************************************************************************
* Function:  SysClearRexxMacroSpace                                      *
*                                                                        *
* Syntax:    result = SysClearRexxMacroSpace()                           *
*                                                                        *
* Params:    none                                                        *
*                                                                        *
* Return:    return code from RexxClearMacroSpace()                      *
*************************************************************************/
RexxRoutine0(int, SysClearRexxMacroSpace)
{
    return (int)RexxClearMacroSpace();          /* clear the macro space      */
}

/*************************************************************************
* Function:  SysSaveRexxMacroSpace                                       *
*                                                                        *
* Syntax:    result = SysSaveRexxMacroSpace(file)                        *
*                                                                        *
* Params:    file   - name of the saved macro space file                 *
*                                                                        *
* Return:    return code from RexxSaveMacroSpace()                       *
*************************************************************************/

RexxRoutine1(int, SysSaveRexxMacroSpace, CSTRING, file)
{
    return (int)RexxSaveMacroSpace(0, NULL, file);
}

/*************************************************************************
* Function:  SysLoadRexxMacroSpace                                       *
*                                                                        *
* Syntax:    result = SysLoadRexxMacroSpace(file)                        *
*                                                                        *
* Params:    file   - name of the saved macro space file                 *
*                                                                        *
* Return:    return code from RexxLoadMacroSpace()                       *
*************************************************************************/

RexxRoutine1(int, SysLoadRexxMacroSpace, CSTRING, file)
{
    return (int)RexxLoadMacroSpace(0, NULL, file);
}


/*************************************************************************
* Function:  SysStemSort                                                 *
*                                                                        *
* Syntax:    result = SysStemSort(stem, order, type, start, end,         *
*                                 firstcol, lastcol)                     *
*                                                                        *
* Params:    stem - name of stem to sort                                 *
*            order - 'A' or 'D' for sort order                           *
*            type - 'C', 'I', 'N' for comparision type                   *
*            start - first index to sort                                 *
*            end - last index to sort                                    *
*            firstcol - first column to use as sort key                  *
*            lastcol - last column to use as sort key                    *
*                                                                        *
* Return:    0 - sort was successful                                     *
*            -1 - sort failed                                            *
*************************************************************************/

RexxRoutine7(int, SysStemSort, RexxObjectPtr, stemArgument, OPTIONAL_CSTRING, order, OPTIONAL_CSTRING, type,
             OPTIONAL_positive_wholenumber_t, first, OPTIONAL_positive_wholenumber_t, last,
             OPTIONAL_positive_wholenumber_t, firstCol, OPTIONAL_positive_wholenumber_t, lastCol)
{
    int sortType = SORT_CASESENSITIVE;
    int sortOrder = SORT_ASCENDING;

    RexxStemObject stem;     // the resolved stem object we're going to sort
    const char *tailExtension = NULL;    // any compound tail section to add to the

    // try to resolve this directly. This is normally either a stem object already or a
    // valid stem name.
    stem = context->ResolveStemVariable(stemArgument);

    // this could be an invalid argument, but it might be a name like "a.1", where the
    // tail needs to be extracted
    if (stem == NULLOBJECT)
    {
        // This is a bit of a pain. It would be nice to create this argument as a stem
        // object, but people have been coding stem sorts using a compound variable rather than
        // a stem name because the old code was lax and it just happened to work. We need to process this
        // as a name and make sure it has a period at the end.

        // we need to get a stem name, and also check for extension. We only process this if it
        // is a string object
        if (!context->IsString(stemArgument))
        {
            context->ThrowException2(Rexx_Error_Incorrect_call_nostem, context->WholeNumberToObject(1), stemArgument);
        }

        // now get the name
        const char *stemName = context->ObjectToStringValue(stemArgument);
        size_t stemLength = strlen(stemName);
        // find the location of the first period
        const char *periodLocation = strstr(stemName, ".");

        // if this does not have a period at all or the first period is at the end, this is
        // easy, this must be an invalid name
        if (periodLocation == NULL || periodLocation == stemName + stemLength - 1)
        {
            context->ThrowException2(Rexx_Error_Incorrect_call_nostem, context->WholeNumberToObject(1), stemArgument);
        }
        // we have a stem plus a section of tail. We need to separate these.
        else
        {
            // copy the tail part
            tailExtension = periodLocation + 1;
            // the resolve call requires a RexxObject, so make a string version of it
            RexxStringObject stemString = context->NewString(stemName, (periodLocation - stemName) + 1);
            // go resolve the stem using the argument directly.
            stem = context->ResolveStemVariable(stemString);

            // this is a error if we can't resolve this
            if (stem == NULLOBJECT)
            {
                context->ThrowException2(Rexx_Error_Incorrect_call_nostem, context->WholeNumberToObject(1), stemArgument);
            }
        }
    }

    // check other parameters.  sort order
    if (order != NULL)
    {
        switch (order[0])
        {
            case 'A':
            case 'a':
                sortOrder = SORT_ASCENDING;
                break;
            case 'D':
            case 'd':
                sortOrder = SORT_DECENDING;
                break;
            default:
                invalidOptionException(context, "SysStemSort", "sort order", "'A' or 'D'", order);
        }
    }

    // sort type
    if (type != NULL)
    {
        switch (type[0])
        {
            case 'C':
            case 'c':
                sortType = SORT_CASESENSITIVE;
                break;
            case 'I':
            case 'i':
                sortType = SORT_CASEIGNORE;
                break;
            default:
                invalidOptionException(context, "SysStemSort", "sort type", "'C' or 'I'", type);
        }
    }

    // first element to sort
    if (argumentOmitted(4))
    {
        // start with the first element if not specified
        first = 1;
    }

    // last element to sort
    if (argumentOmitted(5))
    {
        // do everything
        last = Numerics::MAX_WHOLENUMBER;
    }

    // can't go backwards
    if (last < first)
    {
        relativeOptionException(context, "last", last, "first", first);
    }

    // first column to sort
    if (argumentOmitted(6))
    {
        firstCol = 1;
    }


    // last column to sort
    if (argumentOmitted(7))
    {
        lastCol = Numerics::MAX_WHOLENUMBER;
    }

    if (lastCol < firstCol)
    {
        relativeOptionException(context, "last column", lastCol, "first column", firstCol);
    }

    // the sorting is done in the interpreter
    if (!RexxStemSort(stem, tailExtension, sortOrder, sortType, first, last, firstCol, lastCol))
    {
        context->InvalidRoutine();
    }

    return 0;
}


/*************************************************************************
* Function:  SysStemDelete                                               *
*                                                                        *
* Syntax:    result = SysStemDelete(stem, startitem [,itemcount])        *
*                                                                        *
* Params:    stem - name of stem where item will be deleted              *
*            startitem - index of item to delete                         *
*            itemcount - number of items to delete if more than 1        *
*                                                                        *
* Return:    0 - delete was successful                                   *
*            -1 - delete failed                                          *
*************************************************************************/
RexxRoutine3(int, SysStemDelete, RexxStemObject, toStem, positive_wholenumber_t, start, OPTIONAL_positive_wholenumber_t, count)
{
    if (argumentOmitted(3))
    {
        count = 1;
    }

    wholenumber_t items;

    RexxObjectPtr temp = context->GetStemArrayElement(toStem, 0);
    if (temp == NULLOBJECT || !context->WholeNumber(temp, &items) || items < 0)
    {
        unsetStemException(context);
    }

    // make sure the deletion site is within the bounds
    if (start + count - 1 > items)
    {
        context->ThrowException1(Rexx_Error_Incorrect_call_stem_range, context->StringSizeToObject(items));
    }

    wholenumber_t index;
    /* now copy the remaining indices up front */
    for (index = start;  index + count <= items; index++)
    {
        // copy from the old index to the new index
        RexxObjectPtr value = context->GetStemArrayElement(toStem, index + count);
        // is this a sparse array?
        if (value == NULLOBJECT)
        {
            // return this as a failure
            context->ThrowException1(Rexx_Error_Incorrect_call_stem_sparse_array, context->WholeNumberToObject(index));
        }
        context->SetStemArrayElement(toStem, index, value);
    }

    /* now delete the items at the end */
    for (index = items - count + 1; index <= items; index++)
    {
        context->DropStemArrayElement(toStem, index);
    }

    context->SetStemArrayElement(toStem, 0, context->StringSize(items - count));
    return 0;
}


/*************************************************************************
* Function:  SysStemInsert                                               *
*                                                                        *
* Syntax:    result = SysStemInsert(stem, position, value)               *
*                                                                        *
* Params:    stem - name of stem where item will be inserted             *
*            position - index where new item will be inserted            *
*            value - new item value                                      *
*                                                                        *
* Return:    0 - insert was successful                                   *
*            -1 - insert failed                                          *
*************************************************************************/
RexxRoutine3(int, SysStemInsert, RexxStemObject, toStem, positive_wholenumber_t, position, RexxObjectPtr, newValue)
{
    wholenumber_t count;

    RexxObjectPtr temp = context->GetStemArrayElement(toStem, 0);
    if (temp == NULLOBJECT || !context->WholeNumber(temp, &count))
    {
        unsetStemException(context);
    }

    /* check whether new position is within limits */
    if (position > count + 1)
    {
        context->ThrowException1(Rexx_Error_Incorrect_call_stem_range, context->WholeNumberToObject(count));
    }

    for (wholenumber_t index = count; index >= position; index--)
    {
        // copy from the old index to the new index
        RexxObjectPtr value = context->GetStemArrayElement(toStem, index);
        // is this a sparse array?
        if (value == NULLOBJECT)
        {
            context->ThrowException1(Rexx_Error_Incorrect_call_stem_sparse_array, context->WholeNumberToObject(index));
        }
        context->SetStemArrayElement(toStem, index + 1, value);
    }

    // now set the new value and increase the count at stem.0
    context->SetStemArrayElement(toStem, position, newValue);
    context->SetStemArrayElement(toStem, 0, context->WholeNumber(count + 1));
    return 0;
}


/*************************************************************************
* Function:  SysStemCopy                                                 *
*                                                                        *
* Syntax:    result = SysStemCopy(fromstem, tostem, from, to, count      *
*                                 [,insert])                             *
*                                                                        *
* Params:    fromstem - name of source stem                              *
*            tostem - - name of target stem                              *
*            from  - first index in source stem to copy                  *
*            to - position where items are copied/inserted in target stem*
*            count - number of items to copy/insert                      *
*            insert - 'I' to indicate insert instead of 'O' overwrite    *
*                                                                        *
* Return:    0 - stem copy was successful                                *
*            -1 - stem copy failed                                       *
*************************************************************************/
RexxRoutine6(int, SysStemCopy, RexxStemObject, fromStem, RexxStemObject, toStem,
             OPTIONAL_positive_wholenumber_t, from, OPTIONAL_positive_wholenumber_t, to, OPTIONAL_nonnegative_wholenumber_t, count,
             OPTIONAL_CSTRING, option)
{
    bool inserting = false;

    /* get copy type */
    if (option != NULL)
    {
        switch (*option)
        {
            case 'I':
            case 'i':
                inserting = true;
                break;
            case 'O':
            case 'o':
                inserting = false;
                break;
            default:
            {
                invalidOptionException(context, "SysStemCopy", "sort type", "'I' or 'O'", option);
            }
        }
    }

    wholenumber_t fromCount;

    RexxObjectPtr temp = context->GetStemArrayElement(fromStem, 0);
    if (temp == NULLOBJECT || !context->WholeNumber(temp, &fromCount))
    {
        unsetStemException(context);
    }

    // default from location is the first element
    if (argumentOmitted(3))
    {
        from = 1;
    }

    if (argumentOmitted(4))
    {
        to = 1;
    }

    // was a count explicitly specified?
    if (argumentExists(5))
    {
        // this must be in range
        if ((count > (fromCount - from + 1)) || (fromCount == 0))
        {
            context->ThrowException1(Rexx_Error_Incorrect_call_stem_range, context->WholeNumberToObject(fromCount));
        }
    }
    else
    {
        // default is to copy everything from the starting position.
        count = fromCount - from + 1;
    }

    wholenumber_t toCount = 0;
    // but if it is set, then use that value
    temp = context->GetStemArrayElement(toStem, 0);
    if (temp != NULLOBJECT && !context->WholeNumber(temp, &toCount))
    {
        unsetStemException(context);
    }

    // copying out of range?  Error
    if (to > toCount + 1)
    {
        context->ThrowException1(Rexx_Error_Incorrect_call_stem_range, context->WholeNumberToObject(toCount));
    }

    if (inserting)
    {
        /* if we are about to insert the items we have to make room */
        for (wholenumber_t index = toCount; index >= to; index--)
        {
            // copy from the old index to the new index
            RexxObjectPtr value = context->GetStemArrayElement(toStem, index);
            // is this a sparse array?
            if (value == NULLOBJECT)
            {
                context->ThrowException1(Rexx_Error_Incorrect_call_stem_sparse_array, context->WholeNumberToObject(index));
            }
            context->SetStemArrayElement(toStem, index + count, value);
        }


        // set the new count value in the target
        toCount += count;
        context->SetStemArrayElement(toStem, 0, context->WholeNumberToObject(toCount));
    }
    /* now do the actual copying from the source to target */
    for (wholenumber_t index = 0; index < count; index++)
    {
        // just retrieve and copy
        RexxObjectPtr value = context->GetStemArrayElement(fromStem, from + index);
        // is this a sparse array?
        if (value == NULLOBJECT)
        {
            context->ThrowException1(Rexx_Error_Incorrect_call_stem_sparse_array, context->WholeNumberToObject(index));
        }
        context->SetStemArrayElement(toStem, to + index, value);
    }

    // do we need to update the size?
    if (to + count - 1 > toCount)
    {
        context->SetStemArrayElement(toStem, 0, context->StringSize(to + count - 1));
    }
    return 0;
}


/*************************************************************************
* Function:  SysUtilVersion                                              *
*                                                                        *
* Syntax:    Say SysUtilVersion                                          *
*                                                                        *
* Return:    REXXUTIL.DLL Version                                        *
*************************************************************************/
RexxRoutine0(RexxStringObject, SysUtilVersion)
{
    char buffer[256];
    /* format into the buffer     */
    snprintf(buffer, sizeof(buffer), "%d.%d.%d", ORX_VER, ORX_REL, ORX_MOD);
    return context->String(buffer);
}


/**
 * Write out the name/value pair for an individual variable to the output stream.
 *
 * @param file    The output target
 * @param context The call context
 * @param name    The name of the variable
 * @param value   The object value
 *
 * @return an error code if there is a failure.
 */
void writeVariable(SysFile &file, RexxCallContext *context, const char *name, RexxObjectPtr value)
{
    size_t nameLength = strlen(name);

    RexxStringObject stringValue = context->ObjectToString(value);

    size_t valueLength = context->StringLength(stringValue);
    const char *valueData = context->StringData(stringValue);

    size_t bytesWritten;

    file.write("Name=", strlen("Name="), bytesWritten);
    file.write(name, nameLength, bytesWritten);
    file.write(", Value='", strlen(", Value='"), bytesWritten);
    file.write(valueData, valueLength, bytesWritten);
    file.write("'\n", strlen("'\n"), bytesWritten);

    // now release the local references

    context->ReleaseLocalReference(stringValue);
    context->ReleaseLocalReference(value);
}


/**
 * Write out the compound name/value pair for an individual
 * compound variable to the output stream.
 *
 * @param file   The output target
 * @param name   The name of the variable
 * @param value  The object value
 *
 * @return an error code if there is a failure.
 */
void writeVariable(SysFile &file, RexxCallContext *context, const char *stem, RexxStringObject tail, RexxObjectPtr value)
{
    size_t nameLength = context->StringLength(tail);
    const char *nameData = context->StringData(tail);

    RexxStringObject stringValue = context->ObjectToString(value);

    size_t valueLength = context->StringLength(stringValue);
    const char *valueData = context->StringData(stringValue);

    size_t bytesWritten;

    file.write("Name=", strlen("Name="), bytesWritten);
    file.write(stem, strlen(stem), bytesWritten);
    file.write(nameData, nameLength, bytesWritten);
    file.write(", Value='", strlen(", Value='"), bytesWritten);
    file.write(valueData, valueLength, bytesWritten);
    file.write("'\r\n", strlen("'\r\n"), bytesWritten);

    // now release the local references

    context->ReleaseLocalReference(tail);
    context->ReleaseLocalReference(stringValue);
    context->ReleaseLocalReference(value);
}


/*************************************************************************
* Function:  SysDumpVariables                                            *
*                                                                        *
* Syntax:    result = SysDumpVariables([filename])                       *
*                                                                        *
* Params:    filename - name of the file where variables are appended to *
*                       (dump is written to stdout if omitted)           *
*                                                                        *
* Return:    0 - dump completed OK                                       *
*************************************************************************/
RexxRoutine1(int, SysDumpVariables, OPTIONAL_CSTRING, fileName)
{
    SysFile   outFile;

    if (fileName != NULL)
    {
        RoutineQualifiedName qualifiedName(context, fileName);

        if (!outFile.open(qualifiedName, RX_O_WRONLY | RX_O_APPEND | RX_O_CREAT, RX_S_IWRITE | RX_S_IREAD, RX_SH_DENYRW))
        {
            context->InvalidRoutine();
            return 0;
        }
    }
    else
    {
        outFile.setStdOut();
    }

    // get a snapshot of the variables
    RexxDirectoryObject variables = context->GetAllContextVariables();

    // get a supplier for the variables
    RexxSupplierObject variableSupplier = (RexxSupplierObject)context->SendMessage0(variables, "SUPPLIER");

    while (context->SupplierAvailable(variableSupplier))
    {
        RexxObjectPtr variableName = context->SupplierIndex(variableSupplier);
        CSTRING name = context->ObjectToStringValue(variableName);

        // if the name ends in a period, this is a stem. We need to get the stem object
        // and iterate over the elements.
        if (name[strlen(name) - 1] == '.')
        {
            // get the stem value
            RexxStemObject stem = (RexxStemObject)context->SupplierItem(variableSupplier);

            RexxObjectPtr stemValue = context->GetStemValue(stem);

            // write out the stem name first
            writeVariable(outFile, context, name, stemValue);

            // now iterate on the compound variables
            RexxDirectoryObject compoundVariables = context->GetAllStemElements(stem);

            // get a supplier for the variables
            RexxSupplierObject compoundSupplier = (RexxSupplierObject)context->SendMessage0(compoundVariables, "SUPPLIER");

            // now iterate over the compound elements
            while (context->SupplierAvailable(compoundSupplier))
            {
                RexxStringObject tailName = (RexxStringObject)context->SupplierIndex(compoundSupplier);
                RexxObjectPtr compoundValue = context->SupplierItem(compoundSupplier);

                writeVariable(outFile, context, name, tailName, compoundValue);

                context->SupplierNext(compoundSupplier);
            }

            // release the stem object reference. Other references have already been released by
            // writeVariable
            context->ReleaseLocalReference(stem);
        }
        // a simple variable
        else
        {
            // get the variable value
            RexxObjectPtr value = context->SupplierItem(variableSupplier);

            // and write it out
            writeVariable(outFile, context, name, value);
        }
        // release the reference to the variable name
        context->ReleaseLocalReference(variableName);
        // step to the next variable
        context->SupplierNext(variableSupplier);
    }

    outFile.close();
    return 0;
}


/**
 * Test if the file exists
 */
RexxRoutine1(logical_t, SysFileExists, CSTRING, name)
{
    RoutineQualifiedName qualifiedName(context, name);

    return SysFileSystem::exists(qualifiedName);
}


/*************************************************************************
* Function:  SysIsFileLink                                               *
*                                                                        *
* Syntax:    call SysIsFileLink file                                     *
*                                                                        *
* Params:    file - file to check if it is a Link (Alias).               *
*                                                                        *
* Return:    Logical.                                                    *
*************************************************************************/

RexxRoutine1(logical_t, SysIsFileLink, CSTRING, file)
{
    RoutineQualifiedName qualifiedName(context, file);

    return SysFileSystem::isLink(qualifiedName);
}


/*************************************************************************
* Function:  SysIsFile                                                   *
*                                                                        *
* Syntax:    call SysFileExist file                                      *
*                                                                        *
* Params:    file - file to check existance of.                          *
*                                                                        *
* Return:    true if this is a file object                               *
*************************************************************************/
RexxRoutine1(logical_t, SysIsFile, CSTRING, file)
{
    RoutineQualifiedName qualifiedName(context, file);

    return SysFileSystem::isFile(qualifiedName);
}

/*************************************************************************
* Function:  SysDirExist                                                 *
*                                                                        *
* Syntax:    call SysDirExist dir                                        *
*                                                                        *
* Params:    dir - dir to check existance of.                            *
*                                                                        *
* Return:    Logical.                                                    *
*************************************************************************/

RexxRoutine1(logical_t, SysIsFileDirectory, CSTRING, file)
{
    RoutineQualifiedName qualifiedName(context, file);

    return SysFileSystem::isDirectory(qualifiedName);
}


/*************************************************************************
* Function:  SysRmDir                                                    *
*                                                                        *
* Syntax:    call SysRmDir dir                                           *
*                                                                        *
* Params:    dir - Directory to be removed.                              *
*                                                                        *
* Return:    NO_UTIL_ERROR                                               *
*                                                                        *
*************************************************************************/
RexxRoutine1(int, SysRmDir, CSTRING, path)
{
    RoutineQualifiedName qualifiedName(context, path);

    return SysFileSystem::deleteDirectory(qualifiedName);
}

/*************************************************************************
* Function:  SysFileDelete                                               *
*                                                                        *
* Syntax:    call SysFileDelete file                                     *
*                                                                        *
* Params:    file - file to be deleted.                                  *
*                                                                        *
* Return:    Return code from DosDelete() function.                      *
*************************************************************************/

RexxRoutine1(int, SysFileDelete, CSTRING, path)
{
    RoutineQualifiedName qualifiedName(context, path);

    return SysFileSystem::deleteFile(qualifiedName);
}


/**
 * SysFileSearch searches a file for lines containing needle.
 *
 * @param needle  The string to search for.
 * @param file    The name of the file to search.
 * @param stem    The name of the stem variable that will receive each
 *                file line containing needle.
 * @param opts    A string of options.  A combination of the following chars:
 *                'C' - Case-sensitive search (non-default)
 *                'I' - Case-insensitive search (default)
 *                'N' - Precede each found string in result stem
 *                      with its line number in file (non-default)
* @return  0 on success, non-zero on error.
 *         ERROR_FILEOPEN if file cannot be openend.
 *         ERROR_NOMEM if not enough memory.
 */

RexxRoutine4(CSTRING, SysFileSearch, RexxStringObject, needle, CSTRING, file, RexxStemObject, stem, OPTIONAL_CSTRING, opts)
{
    bool        linenums = false;        // should line numbers be inclued in the output
    bool        sensitive = false;       // how should searchs be performed

    // was the option specified?
    if (opts != NULL)
    {
        for (size_t i = 0; i < strlen(opts); i++)
        {
            switch (toupper(opts[i]))
            {
                case 'N':
                {
                    linenums = true;
                    break;
                }

                case 'I':
                {
                    sensitive = false;
                    break;
                }

                case 'C':
                {
                    sensitive = true;
                    break;
                }

                default:
                {
                    char buf[256] = { 0 };
                    snprintf(buf, sizeof(buf),
                             "SysFileSearch options argument must be a combination of C, I, or N; found \"%s\"",
                             opts);

                    context->ThrowException1(Rexx_Error_Incorrect_call_user_defined, context->String(buf));
                }
            }
        }
    }

    LineReader fileSource;
    RoutineQualifiedName qualifiedName(context, file);

    // if we can't open, return the error indicator
    if (!fileSource.open(qualifiedName))
    {
        return ERROR_FILEOPEN;
    }

    const char *line;
    size_t lineLength;

    const char *needleString = context->StringData(needle);
    size_t needleLength = context->StringLength(needle);

    size_t currentLine = 0;

    size_t currentStemIndex = 0;

    // keep reading while we find lines
    while (fileSource.getLine(line, lineLength))
    {
        currentLine++;
        const char *ptr = mystrstr(line, needleString, lineLength, needleLength, sensitive);

        if (ptr != NULL)
        {
            if (linenums)
            {
                char lineNumber[16];
                snprintf(lineNumber, sizeof(lineNumber), "%zu ", currentLine);

                size_t totalLineSize = strlen(lineNumber) + lineLength;

                AutoFree lineBuffer = (char *)malloc(totalLineSize + 8);
                if (lineBuffer == NULL)
                {
                    // make sure we update the count with the number of return items
                    context->SetStemArrayElement(stem, 0, context->StringSizeToObject(currentStemIndex));
                    return ERROR_NOMEM;
                }

                // now build the return value
                strncpy((char *)lineBuffer, lineNumber, sizeof(totalLineSize));
                memcpy((char *)lineBuffer + strlen(lineNumber), line, lineLength);

                RexxStringObject returnValue = context->NewString(lineBuffer, totalLineSize);

                context->SetStemArrayElement(stem, ++currentStemIndex, returnValue);
                context->ReleaseLocalReference(returnValue);
            }
            else
            {
                RexxStringObject returnValue = context->NewString(line, lineLength);

                context->SetStemArrayElement(stem, ++currentStemIndex, returnValue);
                context->ReleaseLocalReference(returnValue);
            }
        }
    }

    fileSource.close();

    // make sure we update the count with the number of return items
    context->SetStemArrayElement(stem, 0, context->StringSizeToObject(currentStemIndex));
    return "0"; // success
}


/*************************************************************************
* Function:  SysSearchPath                                               *
*                                                                        *
* Syntax:    call SysSearchPath path, file [, options]                   *
*                                                                        *
* Params:    path - Environment variable name which specifies a path     *
*                    to be searched (ie 'PATH', 'DPATH', etc).           *
*            file - The file to search for.                              *
*            options -  'C' - Current directory search first (default).  *
*                       'N' - No Current directory search. Only searches *
*                             the path as specified.                     *
*                                                                        *
* Return:    other  - Full path and filespec of found file.              *
*            ''     - Specified file not found along path.               *
*************************************************************************/
RexxRoutine3(RexxStringObject, SysSearchPath, CSTRING, path, CSTRING, file, OPTIONAL_CSTRING, options)
{
    RoutineFileNameBuffer fullPath(context);

    char opt = 'C'; // this is the default
    if (options != NULL)
    {
        opt = toupper(options[0]);
        if (opt != 'C' && opt != 'N')
        {
            invalidOptionException(context, "SysSearchPath", "option", "'C' or 'N'", options);
        }
    }

    RoutineFileNameBuffer pathValue(context);
    // get the name of the path variable
    SystemInterpreter::getEnvironmentVariable(path, pathValue);

    if (opt == 'N')
    {
        fullPath = pathValue;
    }
    // search current directory first. We construct the patch with the
    // current directory in front of the path.
    else if (opt == 'C')
    {
        RoutineFileNameBuffer currentDir(context);
        SysFileSystem::getCurrentDirectory(currentDir);
        // place the current dir at the front
        fullPath = currentDir;
        // if we have a path, add it to the end
        if (pathValue.length() > 0)
        {
            fullPath += SysFileSystem::getPathSeparator();
            fullPath += pathValue;
        }
    }

    RoutineFileNameBuffer resolvedFile(context);

    // we can do this unconditionally since resolvedFile will be a null
    // string if not found, which is our error return value.
    SysFileSystem::searchPath(file, fullPath, resolvedFile);

    return context->NewStringFromAsciiz(resolvedFile);
}


/*************************************************************************
* Function:  SysSleep                                                    *
*                                                                        *
* Syntax:    call SysSleep secs                                          *
*                                                                        *
* Params:    secs - Number of seconds to sleep.                          *
*                   must be in the range 0 .. 2147483                    *
*                                                                        *
* Return:    0                                                           *
*************************************************************************/
RexxRoutine1(int, SysSleep, RexxStringObject, delay)
{
    // it would be nice to pass this directly as a double, but if there is an error
    // we need the original form to report the error.

    double seconds;
    // try to convert the provided delay to a valid floating point number
    if (context->ObjectToDouble(delay, &seconds) == 0 ||
        std::isnan(seconds) || seconds == HUGE_VAL || seconds == -HUGE_VAL)
    {
        // 88.902 The &1 argument must be a number; found "&2"
        context->RaiseException2(Rexx_Error_Invalid_argument_number, context->String("delay"), delay);
        return 1;
    }

    // there is a maximum delay that can be specifed of 0x7FFFFFF milliseconds
    // according to MSDN the maximum is USER_TIMER_MAXIMUM (0x7FFFFFFF) milliseconds,
    // which translates to 2147483.647 seconds
    if (seconds < 0.0 || seconds > 2147483.0)
    {
        // 88.907 The &1 argument must be in the range &2 to &3; found "&4"
        context->RaiseException(Rexx_Error_Invalid_argument_range,
                                context->ArrayOfFour(context->String("delay"),
                                                     context->String("0"), context->String("2147483"), delay));
        return 1;
    }

    // convert to microseconds, no overflow possible
    uint64_t microseconds = (uint64_t)(seconds * 1000000);

    // go do the sleep
    SysThread::longSleep(microseconds);
    return 0;
}

/*************************************************************************
* Function:  SysFileCopy                                                 *
*                                                                        *
* Syntax:    call SysFileCopy FROMfile TOfile                            *
*                                                                        *
* Params:    FROMfile - file to be moved.                                *
*            TOfile - target file of move operation.                     *
*                                                                        *
* Return:    Return code from from the copy operation                    *
*************************************************************************/
RexxRoutine2(int, SysFileCopy, CSTRING, from, CSTRING, to)
{
    RoutineQualifiedName fromFile(context, from);
    RoutineQualifiedName toFile(context, to);

    return SysFileSystem::copyFile(fromFile, toFile);
}


/*************************************************************************
* Function:  SysFileMove                                                 *
*                                                                        *
* Syntax:    call SysFileMove FROMfile TOfile                            *
*                                                                        *
* Params:    FROMfile - file to be moved.                                *
*            TOfile - target file of move operation.                     *
*                                                                        *
* Return:    Return code from MoveFile() function.                       *
*************************************************************************/
RexxRoutine2(int, SysFileMove, CSTRING, from, CSTRING, to)
{
    RoutineQualifiedName fromFile(context, from);
    RoutineQualifiedName toFile(context, to);

    return SysFileSystem::moveFile(fromFile, toFile);
}


/*************************************************************************
* Function:  SysTempFileName                                             *
*                                                                        *
* Syntax:    call SysTempFileName template [,filler]                     *
*                                                                        *
* Params:    template - Description of filespec desired.  For example:   *
*                        C:\TEMP\FILE.???                                *
*            filler   - A character which when found in template will be *
*                        replaced with random digits until a unique file *
*                        or directory is found.  The default character   *
*                        is '?'.                                         *
*                                                                        *
* Return:    other - Unique file/directory name.                         *
*            ''    - No more files exist given specified template.       *
*************************************************************************/
RexxRoutine2(RexxStringObject, SysTempFileName, CSTRING, fileTemplate, OPTIONAL_CSTRING, fillerOpt)
{
    char filler = '?';

    if (fillerOpt != NULL)
    {
        if (strlen(fillerOpt) != 1)
        {
            RexxArrayObject subs = context->NewArray(3);
            context->ArrayAppendString(subs, "SysTempFileName", strlen("SysTempFileName"));
            context->ArrayAppendString(subs, "filler", strlen("filler"));
            context->ArrayAppendString(subs, fillerOpt, strlen(fillerOpt));

            context->ThrowException(Rexx_Error_Incorrect_call_pad, subs);
        }
        filler = fillerOpt[0];
    }

    RoutineFileNameBuffer fileName(context);
    getUniqueFileName(fileTemplate, filler, fileName);

    return context->NewStringFromAsciiz(fileName);
}


/**
 * Format a message using passed in arguments.
 *
 * @param context The call context.
 * @param message The message test
 * @param args    The array of arguments to the function.
 * @param firstSubstitution
 *                The first substitution argument in the array.
 *
 * @return The formated test as a Rexx String object.
 */
RexxStringObject formatMessage(RexxCallContext *context, const char *message, RexxArrayObject args, size_t firstSubstitution)
{
    size_t numargs = context->ArraySize(args);

    size_t subcount = numargs >= firstSubstitution ? numargs - firstSubstitution + 1 : 0;

    // we only support 9 substitution values
    if (subcount > 9)
    {
        context->ThrowException1(Rexx_Error_Incorrect_call_external, context->String("SysFormatMessage"));
    }

    // array of string values
    const char *substitutions[9];
    size_t msg_length = 0;               // length of the return msg

    // get the string value for each of the substitutions
    for (size_t i = firstSubstitution; i <= numargs; i++)
    {
        // get each of the argument objects as a string value.
        // omitted arguments are not permitted.
        RexxObjectPtr o = context->ArrayAt(args, i);
        if (o == NULLOBJECT)
        {
            // use a null string for any omitted ones
            substitutions[i - firstSubstitution] = "";
        }
        else
        {
            substitutions[i - firstSubstitution] = context->ObjectToStringValue(o);
        }
    }

    // now scan the message calculating the size of the final message
    size_t messageLength = strlen(message);

    // scan for the substitutions and figure out how much is added
    const char *temp = message;

    while ((temp = strstr(temp, "&")))
    {
        char sub = *(temp + 1);
        if (sub >= '1' && sub <= '9')
        {
            size_t subPosition = (size_t)(sub - '1');
            // if we have a substitution value for this, use the
            // real length
            if (subPosition < subcount)
            {
                messageLength += strlen(substitutions[subPosition]);
            }
            // subtract out the substitution characters from the length
            messageLength -= 2;
            temp += 2;
        }
        else
        {
            // just step over the character
            temp++;
        }
    }

    // now we know how long the final message is going to be, we can
    // get a raw string object and fill it in.

    RexxBufferStringObject result = context->NewBufferString(messageLength);

    char *resultBuffer = (char *)context->BufferStringData(result);

    const char *start = message;
    temp = start;

    // now go and make the substitutions
    while ((temp = strstr(temp, "&")))
    {
        char sub = *(temp + 1);
        if (sub >= '1' && sub <= '9')
        {
            size_t subPosition = (size_t)(sub - '1');
            // copy the next section of message text
            size_t leadSize = temp - start;
            if (leadSize > 0)
            {
                memcpy(resultBuffer, start, leadSize);
                resultBuffer += leadSize;
            }
            // if we have a substitution value for this, use the
            // real length
            if (subPosition < subcount)
            {
                size_t sublen = strlen(substitutions[subPosition]);
                memcpy(resultBuffer, substitutions[subPosition], sublen);
                resultBuffer += sublen;
            }
            // subtract out the substitution characters from the length
            start = temp + 2;
            temp = start;
        }
        else
        {
            // just step over the character
            temp++;
        }
    }
    // handle any trailing message part
    size_t tailLength = strlen(message) - (start - message);
    if (tailLength > 0)
    {
        memcpy(resultBuffer, start, tailLength);
    }

    return context->FinishBufferString(result, messageLength);
}


/*************************************************************************
* Function:  SysFormatMessage                                            *
*                                                                        *
* Syntax:    SysFormatMessage(message, subs)                             *
*                                                                        *
* Params:    message        - String error message with insertion        *
*                             indicators.                                *
*            substitutions  - An array of insertion strings.  For        *
*                             messages containing &1, &2, .., any given  *
*                             substitutions[] strings will be inserted.  *
*                                                                        *
* Return:    The message with the inserted strings (if given).           *
*************************************************************************/
RexxRoutine2(RexxStringObject, SysFormatMessage, CSTRING, message, RexxArrayObject, substitutions)
{
    return formatMessage(context, message, substitutions, 1);
}


#define INTERNAL_ROUTINE(name, entry) REXX_TYPED_ROUTINE_PROTOTYPE(entry)

#include "SysRexxUtilFunctions.h"          // generate prototypes for the system functions.

// now redefine to generate the table entries
#undef  INTERNAL_ROUTINE
#define INTERNAL_ROUTINE(name, entry) REXX_TYPED_ROUTINE(name, entry),


// now build the actual entry list
RexxRoutineEntry rexxutil_routines[] =
{
    REXX_TYPED_ROUTINE(SysFileTree,            SysFileTree),
    REXX_TYPED_ROUTINE(SysUtilVersion,         SysUtilVersion),
    REXX_TYPED_ROUTINE(SysAddRexxMacro,        SysAddRexxMacro),
    REXX_TYPED_ROUTINE(SysDropRexxMacro,       SysDropRexxMacro),
    REXX_TYPED_ROUTINE(SysReorderRexxMacro,    SysReorderRexxMacro),
    REXX_TYPED_ROUTINE(SysQueryRexxMacro,      SysQueryRexxMacro),
    REXX_TYPED_ROUTINE(SysClearRexxMacroSpace, SysClearRexxMacroSpace),
    REXX_TYPED_ROUTINE(SysLoadRexxMacroSpace,  SysLoadRexxMacroSpace),
    REXX_TYPED_ROUTINE(SysSaveRexxMacroSpace,  SysSaveRexxMacroSpace),
    REXX_TYPED_ROUTINE(SysDropFuncs,           SysDropFuncs),
    REXX_TYPED_ROUTINE(SysLoadFuncs,           SysLoadFuncs),
    REXX_TYPED_ROUTINE(SysDumpVariables,       SysDumpVariables),
    REXX_TYPED_ROUTINE(SysStemSort,            SysStemSort),
    REXX_TYPED_ROUTINE(SysStemDelete,          SysStemDelete),
    REXX_TYPED_ROUTINE(SysStemInsert,          SysStemInsert),
    REXX_TYPED_ROUTINE(SysStemCopy,            SysStemCopy),
    REXX_TYPED_ROUTINE(SysUtilVersion,         SysUtilVersion),
    REXX_TYPED_ROUTINE(SysFileExists,          SysFileExists),
    REXX_TYPED_ROUTINE(SysIsFileLink,          SysIsFileLink),
    REXX_TYPED_ROUTINE(SysIsFile,              SysIsFile),
    REXX_TYPED_ROUTINE(SysIsFileDirectory,     SysIsFileDirectory),
    REXX_TYPED_ROUTINE(SysRmDir,               SysRmDir),
    REXX_TYPED_ROUTINE(SysFileDelete,          SysFileDelete),
    REXX_TYPED_ROUTINE(SysFileSearch,          SysFileSearch),
    REXX_TYPED_ROUTINE(SysSearchPath,          SysSearchPath),
    REXX_TYPED_ROUTINE(SysSleep,               SysSleep),
    REXX_TYPED_ROUTINE(SysFileMove,            SysFileMove),
    REXX_TYPED_ROUTINE(SysFileCopy,            SysFileCopy),
    REXX_TYPED_ROUTINE(SysTempFileName,        SysTempFileName),
    REXX_TYPED_ROUTINE(SysFormatMessage,       SysFormatMessage),
#include "SysRexxUtilFunctions.h"
    REXX_LAST_ROUTINE()
};

RexxPackageEntry rexxutil_package_entry =
{
    STANDARD_PACKAGE_HEADER
    REXX_INTERPRETER_4_0_0,              // anything after 4.0.0 will work
    "REXXUTIL",                          // name of the package
    "5.0.0",                             // package information
    NULL,                                // no load/unload functions
    NULL,
    rexxutil_routines,                   // the exported functions
    NULL                                 // no methods in this package
};

// and finally plug this in to the package manager.
RexxPackageEntry *PackageManager::rexxutilPackage = &rexxutil_package_entry;
