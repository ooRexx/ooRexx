/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.ibm.com/developerworks/oss/CPLv1.0.htm                          */
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

#ifndef LocalMacroSpaceManager_HPP_INCLUDED
#define LocalMacroSpaceManager_HPP_INCLUDED

#include "LocalAPISubsystem.hpp"
#include "ServiceMessage.hpp"
#include "rexx.h"
#include "Rxstring.hpp"
#include "Utilities.hpp"
#include "SysFile.hpp"

// Macro space version information
#define RXVERSION  "REXX-ooRexx 6.00"
#define RXVERSIZE  (sizeof(RXVERSION) - 1)
// Macrospace signature
#define SIGNATURE  0xddd5

class MacroSpaceDescriptor
{
public:
enum { MACRONAMESIZE = 256 };

    MacroSpaceDescriptor() { ; }
    MacroSpaceDescriptor(const char *n, size_t s, size_t o)
    {
        strcpy(name, n);
        image.strlength = s;
        image.strptr = NULL;
        imageSize = s;
        position = o;
    }

    void    *reserved;               // this is the next pointer in old platforms, but saved in file
    char     name[MACRONAMESIZE];    // function name
    RXSTRING image;                  // place holder only
    size_t   imageSize;              // size of image
    size_t   position;               // preorder/postorder flag
};


class MacroSpaceFileHeader
{
public:
    MacroSpaceFileHeader() { ; }

    MacroSpaceFileHeader(size_t c)
    {
        memcpy(version, RXVERSION, RXVERSIZE);
        magicNumber = SIGNATURE;
        count = c;
    }

    char     version[RXVERSIZE];     // version of the Rexx interpreter
    size_t   magicNumber;            // macro space magic number
    size_t   count;                  // count of macros in the saved file
};


class NameTable
{
public:
    NameTable(const char **n, size_t c)
    {
        names = n;
        count = c;
    }

    bool inTable(const char *name)
    {
        for (size_t i = 0; i < count; i++)
        {
            if (Utilities::strCaselessCompare(name, names[i]) == 0)
            {
                return true;
            }
        }
        return false;
    }

protected:
    const char **names;         // pointer to list of names
    size_t count;               // name count
};

class MacroSpaceFile
{
public:
    MacroSpaceFile(const char *name) : fileName(name), fileInst(NULL) { ; }

    ~MacroSpaceFile();
    void close();
    size_t openForLoading();
    void nextMacro(char *name, ManagedRxstring &image, size_t &order);
    void nextMacro(NameTable names, char *name, ManagedRxstring &image, size_t &order);
    void setFilePosition(size_t p);
    void create(size_t count);
    void writeMacroDescriptor(const char *name, size_t size, size_t order);
    void read(void *data, size_t length);
    void read(ManagedRxstring &data, size_t length);
    void write(const void *data, size_t length);

    inline void write(const char *str)
    {
        write(str, strlen(str) + 1);
    }

    inline void write(size_t i)
    {
        write((void *)&i, sizeof(size_t));
    }

    inline void write(RXSTRING &data)
    {
        write(data.strptr, data.strlength);
    }

protected:
    bool    creating;           // indicates whether we are reading or creating
    const char *fileName;
    SysFile *fileInst;
    size_t  descriptorBase;
    size_t  imageBase;
};



// local instance of the macro API...this is a proxy that communicates with the
// server that manages the macrospace
class LocalMacroSpaceManager : public LocalAPISubsystem
{
public:

    LocalMacroSpaceManager();

    RexxReturnCode loadMacroSpace(const char *target);
    RexxReturnCode loadMacroSpace(const char *target, const char **nameList, size_t nameCount);
    RexxReturnCode saveMacroSpace(const char *target);
    RexxReturnCode queryMacro(const char *target, size_t *pos);
    RexxReturnCode reorderMacro(const char *target, size_t pos);
    RexxReturnCode getMacro(const char *target, RXSTRING &image);
    RexxReturnCode saveMacroSpace(const char *target, const char **names, size_t count);
    RexxReturnCode clearMacroSpace();
    RexxReturnCode removeMacro(const char *name);
    RexxReturnCode addMacroFromFile(const char *name, const char *sourceFile, size_t position);
    RexxReturnCode addMacro(const char *name, ManagedRxstring &imageData, size_t position);
    void translateRexxProgram(const char *sourcefile, ManagedRxstring &imageData);
    void readRxstringFromFile(SysFile *file, ManagedRxstring &target, size_t size);
    RexxReturnCode mapReturnResult(ServiceMessage &m);
    virtual RexxReturnCode processServiceException(ServiceException *e);
};


#endif

