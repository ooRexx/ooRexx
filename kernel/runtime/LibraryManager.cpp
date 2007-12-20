/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                          */
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

#include "RexxCore.h"
#include "LibraryManager.hpp"
#include "DirectoryClass.hpp"
#include "PointerClass.hpp"

// the table of loaded libraries
RexxDirectory *LibraryManager::libraries = OREF_NULL;


/**
 * Set up the initial libraray manager during image save.  During
 * image create, we'll use a copy of the saved library.
 */
void LibraryManager::init()
{
    // create the new library directory
    libraries = new_directory();
}


/**
 * Restore all of our image-referenced external libraries
 * on an image restore.
 *
 * @param savedLibraries
 *               The directory of libraries saved as part of the image.
 */
void LibraryManager::reload()
{
    // since this was originally saved in the oldspace table, get a fresh copy
    // to prevent new libraries from needing to be anchored in the old-to-new
    // table.
    libraries = (RexxDirectory *)libraries->copy();

    for (HashLink i = libraries->first(); libraries->available(i); i = libraries->next(i))
    {
                                         /* then re-resolve all addresses     */
        reloadLibrary((RexxString *)libraries->index(i), (RexxDirectory *)libraries->value(i));
    }
}


/**
 * Restore all of our image-referenced external libraries
 * on an image restore.
 *
 * @param savedLibraries
 *               The directory of libraries saved as part of the image.
 */
void LibraryManager::restore(RexxDirectory *savedLibraries)
{
    // just save this for now.  We'll process it later once the memory manager gets
    // fully initialized.
    libraries = savedLibraries;
}


/**
 * Normal garbage collection marking
 */
void LibraryManager::live(size_t liveMark)
{
    memory_mark(libraries);
}


/**
 * The generalized object marking method.
 */
void LibraryManager::liveGeneral(int reason)
{
    if (!memoryObject.savingImage())
    {
        memory_mark_general(libraries);
    }
}


/**
 * Create a native code object from a reference to an external
 * library and procedure name.
 *
 * @param procedure The name of the procedure.
 * @param library   The name of the library holding the procedure.
 *
 * @return A native code object used to invoke the resolved method.
 */
RexxNativeCode *LibraryManager::createNativeCode(RexxString *procedure, RexxString *library)
{
    // try to load the library.  If we can't get this, then just return
    // an un-resolved library method.
    RexxDirectory *libraryInfo = loadLibrary(library);
    if (libraryInfo == OREF_NULL)
    {
        return new RexxNativeCode (procedure, library, NULL);
    }

    // we might have resolved this one already, so check the directory for the
    // resolved routines
    RexxNativeCode *newCode = (RexxNativeCode *)libraryInfo->entry(procedure);
    if (newCode != OREF_NULL)
    {
        return newCode;                 // this is cool, just return it
    }
    // we need to resolve this procedure
    PNMF entry = (PNMF)SysLoadProcedure((RexxPointer *)libraryInfo->at(OREF_NULLSTRING), procedure);
    // and create a code object from this entry point
    newCode = new RexxNativeCode (procedure, library, entry);
    // add this to the resolved procedure cache
    libraryInfo->setEntry(procedure, (RexxObject *)newCode);
    return newCode;
}

/**
 * Resolve the entry point address for a given external method.
 *
 * @param procedure The procedure name.
 * @param library   The library name.
 *
 * @return The entry point for the method.
 */
PNMF LibraryManager::resolveExternalMethod(RexxString *procedure, RexxString *library)
{
    // try to load the library.  If we can't get this, then just return
    // an un-resolved library method.
    RexxDirectory *libraryInfo = loadLibrary(library);
    if (libraryInfo == OREF_NULL)
    {
        return NULL;
    }

    // we need to resolve this procedure
    return (PNMF)SysLoadProcedure((RexxPointer *)libraryInfo->at(OREF_NULLSTRING), procedure);

}


/**
 * Create a native external method that's resident in the interpreter.
 *
 * @param index  The method table index of the target method entry point.
 *
 * @return A native code object for that index.
 */
RexxNativeCode *LibraryManager::createInternalNativeCode(size_t index)
{
  return (RexxNativeCode *)new RexxNativeCode (internalMethodTable[index].entryPoint, index);
}


/**
 * Reload a library and re-resolve all of the entry points defined
 * for that library.
 *
 * @param libraryName
 *               The library name.
 * @param libraryInfo
 *               The library info directory object containing the resolved method
 *               definitions.
 */
void LibraryManager::reloadLibrary(RexxString *libraryName, RexxDirectory *libraryInfo)
{
    // reload the library and get the handle
    RexxPointer *libraryHandle = SysLoadLibrary(libraryName);
    // and stuff the new handle into the library definition
    libraryInfo->setEntry(OREF_NULLSTRING, libraryHandle);
                                         /* now traverse the entire table     */
    for (HashLink i = libraryInfo->first(); libraryInfo->available(i); i = libraryInfo->next(i))
    {
        RexxString *procedureName = (RexxString *)libraryInfo->index(i);
        if (procedureName != OREF_NULLSTRING)
        {
            // for each real procedure, have it initialize from its hosting library
            RexxNativeCode *code = (RexxNativeCode *)libraryInfo->value(i);
            code->reinit(libraryHandle);
        }
    }
}


/**
 * Load (or located an already loaded) external library used
 * for native methods.
 *
 * @param libraryName
 *               The library name.
 *
 * @return A directory object used to hold the library information.
 */
RexxDirectory *LibraryManager::loadLibrary(RexxString *libraryName)
{
    // just fail silently if no library is given
    if (libraryName == OREF_NULL)
    {
        return OREF_NULL;
    }

    RexxDirectory *libraryInfo = (RexxDirectory *)libraries->get(libraryName);
    // if nothing there, add a new entry to our list, using the uppercase name
    if (libraryInfo == OREF_NULL)
    {
        libraryInfo = new_directory();
        libraries->setEntry(libraryName, libraryInfo);
        // now load the library using the original library name
        libraryInfo->setEntry(OREF_NULLSTRING, SysLoadLibrary(libraryName));
    }
    return libraryInfo;
}


/**
 * Resolve a reference to an internal native method.
 *
 * @param name   The name of the internal entry.
 *
 * @return The index of the resolved entry.  Raises a syntax error if not located.
 */
size_t LibraryManager::resolveInternalMethod(RexxString *name)
{
    // scan the table looking for the named entry
    size_t index = 0;
    for (; internalMethodTable[index].entryName != NULL; index++)
    {
        if (name->strCompare(internalMethodTable[index].entryName))
        {
            return index;
        }
    }
    return SIZE_MAX;    // this indicates an error
}


#define INTERNAL_METHOD(name) char * REXXENTRY name(void **);

extern "C" {
#include "NativeMethods.h"             /* bring in the internal list        */
}


#undef  INTERNAL_METHOD
#define INTERNAL_METHOD(name) InternalMethodEntry(#name , (PNMF)name),

InternalMethodEntry LibraryManager::internalMethodTable[] =
{
#include "NativeMethods.h"             /* bring in the internal method table*/
   InternalMethodEntry(NULL, NULL)     /* final empty entry                 */
};

