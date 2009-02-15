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

#ifndef MacroSpaceManager_HPP_INCLUDED
#define MacroSpaceManager_HPP_INCLUDED

#include "ServiceMessage.hpp"
#include "SysSemaphore.hpp"

class MacroItem
{
public:
    MacroItem(const char *n, const char *, size_t l, size_t p);

    void update(const char *, size_t l, size_t p);
    ~MacroItem()
    {
        delete [] name;             // release the name and
        ServiceMessage::releaseResultMemory((void *)imageBuffer);
    }

    MacroItem *next;                   // next macro in chain
    const char *name;                  // the macro space name
    const char *imageBuffer;           // the image data
    size_t     imageSize;              // size of the image data
    size_t     searchPosition;         // the saved position
};


// a table of queues
class MacroTable
{
public:

    MacroTable()
    {
        macros = NULL;
        iterator = NULL;
    }

    void clear();
    // locate a named data queue
    MacroItem *locate(const char *name);
    // locate and remove a named data queue
    MacroItem *remove(const char *name);

    inline void reorder(MacroItem *current, MacroItem *previous)
    {
        if (previous != NULL)            // if we have a predecessor
        {
            // rearrange to get "most recently used" behavior
            previous->next = current->next;
            current->next = macros;
            macros = current;
        }
    }

    inline void removeMacro(MacroItem *current, MacroItem *previous)
    {
        if (previous != NULL)            // if we have a predecessor
        {
            // rearrange to get "most recently used" behavior
            previous->next = current->next;
        }
        else
        {
            macros = current->next;
        }
    }

    inline size_t macroCount()
    {
        size_t count = 0;
        MacroItem *current = macros;
        while (current != NULL)
        {
            count++;
            current = current->next;
        }
        return count;
    }

    inline bool iterating() { return iterator != NULL; }
    inline void startIteration() { iterator = macros; }
    inline bool hasMore() { return iterator != NULL; }
    inline MacroItem * getNext()
    {
        if (iterator == NULL)
        {
            return NULL;
        }
        MacroItem *current = iterator;
        iterator = iterator->next;
        return current;
    }

    // locate a named data queue
    inline void add(MacroItem *macro)
    {
        macro->next = macros;
        macros = macro;
        iterator = NULL;         // this invalidates any iterator we may have
    }

    inline bool isEmpty()
    {
        return macros == NULL;
    }

protected:
    MacroItem *macros;           // head of the data queue chain
    MacroItem *iterator;         // current iteration position
};

class ServerMacroSpaceManager
{
public:
    enum
    {
        MACRO_PREORDER,
        MACRO_POSTORDER
    };


    ServerMacroSpaceManager() : lock(), macros() { lock.create(); }

    void terminateServer();
    void addMacro(ServiceMessage &message);
    void deleteMacro(ServiceMessage &message);
    void clear(ServiceMessage &message);
    void queryMacro(ServiceMessage &message);
    void reorderMacro(ServiceMessage &message);
    void iterateMacros(ServiceMessage &message);
    void nextDescriptor(ServiceMessage &message);
    void nextImage(ServiceMessage &message);
    void getDescriptor(ServiceMessage &message);
    void getImage(ServiceMessage &message);
    void dispatch(ServiceMessage &message);
    void cleanupProcessResources(SessionID session);

    inline bool isStoppable()
    {
        return macros.isEmpty();
    }

protected:
    SysMutex     lock;                 // our subsystem lock
    MacroTable   macros;               // all of the manaaged macros.
};

#endif
