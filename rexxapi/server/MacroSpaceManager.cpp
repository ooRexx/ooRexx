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

#include "MacroSpaceManager.hpp"
#include "Utilities.hpp"

/**
 * Create a macro item entry.
 *
 * @param n      The name of the macro.
 * @param data   The image data for the macro.
 * @param l      The size of the image data.
 * @param p      The search position.
 */
MacroItem::MacroItem(const char *n, const char *data, size_t l, size_t p)
{
    next = NULL;
    name = dupString(n);
    imageBuffer = data;
    imageSize = l;
    searchPosition = p;
}

/**
 * Update the image data for a macro item.
 *
 * @param data   The new image data.
 * @param l      The length of the new data.
 * @param p      The position data.
 */
void MacroItem::update(const char *data, size_t l, size_t p)
{
    ServiceMessage::releaseResultMemory((void *)imageBuffer);
    imageBuffer = data;
    imageSize = l;
    searchPosition = p;
}


/**
 * Clear the macro table.
 */
void MacroTable::clear()
{
    iterator = NULL;         // this invalidates any iterator we may have
    MacroItem *current = macros;
    while (current != NULL)
    {
        MacroItem *next = current->next;
        delete current;
        current = next;
    }
    macros = NULL;          // clear the anchor
}


/**
 * locate a named macro item.
 *
 * @param name   The required macro item.
 *
 * @return The located macro item entry.
 */
MacroItem *MacroTable::locate(const char *name)
{
    MacroItem *current = macros;    // start the search
    MacroItem *previous = NULL;     // no previous one

    while (current != NULL)              /* while more macros          */
    {
        // find the one we want?
        if (Utilities::strCaselessCompare(name, current->name) == 0)
        {
            // move this to the front so we find it quickly
            reorder(current, previous);
            return current;
        }
        previous = current;                /* remember this block        */
        current = current->next;           /* step to the next block     */
    }
    return NULL;
}

/**
 * locate and remove a named macro space
 *
 * @param name   The name of the target macro.
 *
 * @return The removed table item, or NULL if this is not found.
 */
MacroItem *MacroTable::remove(const char *name)
{
    MacroItem *current = macros;    // start the search
    MacroItem *previous = NULL;     // no previous one

    while (current != NULL)              /* while more macros          */
    {
        iterator = NULL;         // this invalidates any iterator we may have
        // find the one we want?
        if (Utilities::strCaselessCompare(name, current->name) == 0)
        {
            // move this to the front so we find it quickly
            removeMacro(current, previous);
            return current;
        }
        previous = current;                /* remember this block        */
        current = current->next;           /* step to the next block     */
    }
    return NULL;
}


// Add an item to the macro space.  The message arguments have the
// following meanings:
//
// parameter1 -- length of the macro image
// parameter2 -- order flag
// nameArg    -- ASCII-Z name of the macro
void ServerMacroSpaceManager::addMacro(ServiceMessage &message)
{
    MacroItem *item = macros.locate(message.nameArg);
    // already exists?
    if (item == NULL)
    {
        item = new MacroItem(message.nameArg, (const char *)message.getMessageData(), message.getMessageDataLength(), message.parameter2);
        macros.add(item);
    }
    else
    {
        item->update((const char *)message.getMessageData(), message.getMessageDataLength(), message.parameter2);
    }
    // we're keeping the storage here, so detach it from the message.
    message.clearMessageData();
    message.setResult(MACRO_ADDED);
}


// Remove an item from the macro space.  The message arguments have the
// following meanings:
//
// nameArg    -- ASCII-Z name of the macro
void ServerMacroSpaceManager::deleteMacro(ServiceMessage &message)
{
    MacroItem *item = macros.locate(message.nameArg);
    // already exists?
    if (item != NULL)
    {
        macros.remove(message.nameArg);
        message.setResult(MACRO_REMOVED);
    }
    else
    {
        message.setResult(MACRO_DOES_NOT_EXIST);
        return;
    }
}

// Remove all macros from the macro space.
void ServerMacroSpaceManager::clear(ServiceMessage &message)
{
    macros.clear();
    message.setResult(MACRO_SPACE_CLEARED);
}


// Query an item from the macro space.  The message arguments have the
// following meanings:
//
// nameArg    -- ASCII-Z name of the macro
void ServerMacroSpaceManager::queryMacro(ServiceMessage &message)
{
    MacroItem *item = macros.locate(message.nameArg);
    // already exists?
    if (item != NULL)
    {
        message.setResult((ServiceReturn)item->searchPosition);
    }
    else
    {
        message.setResult(MACRO_DOES_NOT_EXIST);
    }
}


// Change the order of a macro space item.  The message arguments have the
// following meanings:
//
// parameter1 -- postorder or preorder flag
// nameArg    -- ASCII-Z name of the macro
void ServerMacroSpaceManager::reorderMacro(ServiceMessage &message)
{
    MacroItem *item = macros.locate(message.nameArg);
    // already exists?
    if (item != NULL)
    {
        item->searchPosition = message.parameter1;
        message.setResult(MACRO_ORDER_CHANGED);
    }
    else
    {
        message.setResult(MACRO_DOES_NOT_EXIST);
    }
}


// Start iteration through the macro list.  The message arguments have the
// following meanings:
//
// parameter1 -- used to return count of macros
void ServerMacroSpaceManager::iterateMacros(ServiceMessage &message)
{
    macros.startIteration();
    message.parameter1 = macros.macroCount();
    message.setResult(MACRO_ITERATION_STARTED);
}


// Get next macro descriptor from iterator.  The message arguments are
// empty, but the definition is returned in the message.
//
// parameter1 -- size of the macro image
// parameter2 -- postorder/preorder flag
// nameArg    -- ASCII-Z name of the macro
void ServerMacroSpaceManager::nextDescriptor(ServiceMessage &message)
{
    MacroItem *item = macros.getNext();
    // no item, we're done
    if (item == NULL)
    {
        // this is an end indication
        message.setResult(NO_MORE_MACROS);
    }
    else
    {
        message.parameter1 = item->imageSize;
        message.parameter2 = item->searchPosition;
        strcpy(message.nameArg, item->name);
        // this is an end indication
        message.setResult(MACRO_RETURNED);
    }
}

// Get next macro descriptor from iterator.  The message arguments are
// empty, but the definition is returned in the message.
//
// parameter1 -- size of the macro image
// parameter2 -- postorder/preorder flag
// nameArg    -- ASCII-Z name of the macro
void ServerMacroSpaceManager::nextImage(ServiceMessage &message)
{
    MacroItem *item = macros.getNext();
    // no item, we're done
    if (item == NULL)
    {
        // this is an end indication
        message.setResult(NO_MORE_MACROS);
    }
    else
    {
        message.parameter1 = item->imageSize;
        message.parameter2 = item->searchPosition;
        strcpy(message.nameArg, item->name);

        // get the macro data
        message.setMessageData((void *)item->imageBuffer, item->imageSize);
        // this data is retained after the result send.
        message.retainMessageData = true;
    }
}


// Get the descriptor for a macro.  The message arguments is
// just the name of the macro.
//
// parameter1 -- size of the macro image
// parameter2 -- postorder/preorder flag
// nameArg    -- ASCII-Z name of the macro
void ServerMacroSpaceManager::getDescriptor(ServiceMessage &message)
{
    MacroItem *item = macros.locate(message.nameArg);
    // no item, we're done
    if (item == NULL)
    {
        // this is an end indication
        message.setResult(MACRO_DOES_NOT_EXIST);
    }
    else
    {
        // copy the specifics
        message.parameter1 = item->imageSize;
        message.parameter2 = item->searchPosition;
        // this is an end indication
        message.setResult(MACRO_RETURNED);
    }
}


// Get the full descriptor for a macro, including the image data.
// The message arguments is
// just the name of the macro.
//
// parameter1 -- size of the macro image
// parameter2 -- postorder/preorder flag
// nameArg    -- ASCII-Z name of the macro
void ServerMacroSpaceManager::getImage(ServiceMessage &message)
{
    MacroItem *item = macros.locate(message.nameArg);
    // no item, we're done
    if (item == NULL)
    {
        // this is an end indication
        message.setResult(MACRO_DOES_NOT_EXIST);
    }
    else
    {
        // copy the specifics
        message.parameter1 = item->imageSize;
        message.parameter2 = item->searchPosition;
        // get the macro data
        message.setMessageData((void *)item->imageBuffer, item->imageSize);
        // this data is retained after the result send.
        message.retainMessageData = true;
    }
}


/**
 * Dispatch an inbound operation to this service manager.
 *
 * @param message The message to process.
 */
void ServerMacroSpaceManager::dispatch(ServiceMessage &message)
{
    switch (message.operation)
    {
        case ADD_MACRO:
            addMacro(message);
            break;
        case ITERATE_MACRO_DESCRIPTORS:
            iterateMacros(message);
            break;
        case NEXT_MACRO_DESCRIPTOR:
            nextDescriptor(message);
            break;
        case GET_MACRO_IMAGE:
            getImage(message);
            break;
        case GET_MACRO_DESCRIPTOR:
            getDescriptor(message);
            break;
        case CLEAR_MACRO_SPACE:
            clear(message);
            break;
        case REMOVE_MACRO:
            deleteMacro(message);
            break;
        case QUERY_MACRO:
            queryMacro(message);
            break;
        case REORDER_MACRO:
            reorderMacro(message);
            break;
        case ITERATE_MACROS:
            iterateMacros(message);
            break;
        case NEXT_MACRO_IMAGE:
            nextImage(message);
            break;
        default:
            message.setExceptionInfo(SERVER_FAILURE, "Invalid macro space manager operation");
            break;
    }
}

void ServerMacroSpaceManager::cleanupProcessResources(SessionID session)
{
    // this is a NOP for the macro space
}
