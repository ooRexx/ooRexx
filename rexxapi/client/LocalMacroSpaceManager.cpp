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

#include "LocalMacroSpaceManager.hpp"
#include "LocalAPIManager.hpp"
#include "SysLibrary.hpp"
#include "ClientMessage.hpp"
#include "SysFile.hpp"
#include "rexx.h"
#include <stdio.h>


/**
 * Destructor to force the macro space file to close.
 */
MacroSpaceFile::~MacroSpaceFile()
{
    // we're being terminated with the file still open...delete, and
    // erase the file we have so far.
    if (fileInst != NULL)
    {

        fileInst->close();
        // if we were trying to create this file, erase the
        // partially created one.
        if (creating)
        {
            remove(fileName);
        }
    }
}

/**
 * Explicitly close the macro space file.
 */
void MacroSpaceFile::close()
{
    fileInst->close();
    delete fileInst;
    fileInst = NULL;
}


/**
 * Open a macrospace file for loading.
 *
 * @return The count of macros in the file.
 */
size_t MacroSpaceFile::openForLoading()
{
    MacroSpaceFileHeader header;
    bool opened;

    // open the file
    fileInst = new SysFile();
    opened = fileInst->open(fileName, RX_O_RDONLY, 0, RX_SH_DENYRW);
    if (opened == false)
    {
        throw new ServiceException(FILE_CREATION_ERROR, "Unable to open macrospace file");
    }
    creating = false;                // we're just reading this
    read(&header, sizeof(header));   // read the header information

    if (memcmp(header.version, RXVERSION, RXVERSIZE) != 0)
    {
        throw new ServiceException(MACROSPACE_VERSION_ERROR, "Incompatible macro space version");
    }

    if (header.magicNumber != SIGNATURE)
    {
        throw new ServiceException(MACROSPACE_SIGNATURE_ERROR, "Incompatible macro space signature");
    }
    descriptorBase = sizeof(header);     // now mark the position of the descriptors
                                         // and the calculated start position of the
                                         // image data.
    imageBase = sizeof(MacroSpaceDescriptor) * header.count;

    return header.count;                 // we have a size, return it.
}


/**
 * Retrieve the next macro from the macrospace file.
 *
 * @param name   The returned macro name.
 * @param image  The macro image information.
 * @param order  The macro ordering information.
 */
void MacroSpaceFile::nextMacro(char *name, ManagedRxstring &image, size_t &order)
{
    setFilePosition(descriptorBase);
    descriptorBase += sizeof(MacroSpaceDescriptor);
    MacroSpaceDescriptor desc;

    read(&desc, sizeof(desc));
    strcpy(name, desc.name);
    order = desc.position;
    setFilePosition(imageBase);
    imageBase += desc.imageSize;
    read(image, desc.imageSize);
}


/**
 * Step to the next macro, reading the information if it
 * is in the target list.
 *
 * @param names  The table of names.
 * @param name   The returned name, if read.
 * @param image  The macro image (if read).
 * @param order  The macro order information.
 */
void MacroSpaceFile::nextMacro(NameTable names, char *name, ManagedRxstring &image, size_t &order)
{
    setFilePosition(descriptorBase);
    descriptorBase += sizeof(MacroSpaceDescriptor);
    MacroSpaceDescriptor desc;

    read(&desc, sizeof(desc));

    // we only read the image data in if this is in the requested list
    if (names.inTable(desc.name))
    {
        strcpy(name, desc.name);
        order = desc.position;
        setFilePosition(imageBase);
        imageBase += desc.imageSize;
        read(image, desc.imageSize);
    }
    else
    {
        // even though not reading this, we need to update the read position
        imageBase += desc.imageSize;

    }
}


/**
 * Explicitly set the file postion.
 *
 * @param p      The new file position.
 */
void MacroSpaceFile::setFilePosition(size_t p)
{
    int64_t position;
    if (fileInst->seek(p, SEEK_SET, position) == false)
    {
        throw new ServiceException(FILE_READ_ERROR, "Error reading from macrospace file");
    }
}


/**
 * Create a macro space file with an initial header table
 * for the indicated number of macros.
 *
 * @param count  The number of macros to store in the file.
 */
void MacroSpaceFile::create(size_t count)
{
    bool opened;
    // create the file
    fileInst = new SysFile;
    opened = fileInst->open(fileName, RX_O_CREAT | RX_O_TRUNC | RX_O_WRONLY, RX_S_IREAD | RX_S_IWRITE, RX_SH_DENYRW);

    if (opened == false)
    {
        throw new ServiceException(FILE_CREATION_ERROR, "Unable to create macrospace file");
    }
    creating = true;

    MacroSpaceFileHeader header(count);
    write(&header, sizeof(header));
}

/**
 * Write a macro descriptor out to the file.
 *
 * @param name   The name of the macro to write.
 * @param size   The size of the macro being written.
 * @param order  The macro order information.
 */
void MacroSpaceFile::writeMacroDescriptor(const char *name, size_t size, size_t order)
{
    MacroSpaceDescriptor desc(name, size, order);

    write(&desc, sizeof(desc));
}


/**
 * Write a buffer of data to the macro file.
 *
 * @param data   The data buffer pointer.
 * @param length The length to write.
 */
void MacroSpaceFile::write(const void *data, size_t length)
{
    size_t bytesWritten;
    fileInst->write((const char *)data, length, bytesWritten);
    if (bytesWritten != (size_t)length)
    {
        throw new ServiceException(FILE_WRITE_ERROR, "Error writing to macrospace file");
    }
}


/**
 * Read a buffer of data from the macrospace file.
 *
 * @param data   The target data buffer.
 * @param length The size to read.
 */
void MacroSpaceFile::read(void *data, size_t length)
{
    size_t bytesRead;
    fileInst->read((char *)data, length, bytesRead);
    if (bytesRead != (size_t)length)
    {
        throw new ServiceException(FILE_READ_ERROR, "Error reading from macrospace file");
    }
}


/**
 * Read a buffer of data into a managed RXSTRING structure.
 *
 * @param data   The target RXSTRING
 * @param length The length to read.
 */
void MacroSpaceFile::read(ManagedRxstring &data, size_t length)
{
    data.ensureCapacity(length);
    read(data.strptr, length);
    data.strlength = length;
}


LocalMacroSpaceManager::LocalMacroSpaceManager() : LocalAPISubsystem()
{
    // no state in this
}


/**
 * Load a macrospace file into our space.
 *
 * @param target The target file name.
 */
RexxReturnCode LocalMacroSpaceManager::loadMacroSpace(const char *target)
{
    // now open and read the file header
    MacroSpaceFile file(target);
    // validate the file
    size_t count = file.openForLoading();

    ManagedRxstring image;    // this is outside the loop, which gives us the chance to reuse the buffer

    for (size_t i = 0; i < count; i++)
    {
        char macroName[MacroSpaceDescriptor::MACRONAMESIZE];
        size_t order;

        file.nextMacro(macroName, image, order);

        ClientMessage message(MacroSpaceManager, ADD_MACRO, macroName);
        message.parameter1 = image.strlength;
        message.parameter2 = order;
        // attach the queue item to the message.
        message.setMessageData(image.strptr, image.strlength);

        // request the next one.
        message.send();
        // NB:  The only error that can occur here is a critical exception.  We don't
        // need to check the return
    }
    file.close();
    return RXMACRO_OK;
}


/**
 * Load a macrospace file, using just the subset of names
 * in the file.
 *
 * @param target    The target macrospace file.
 * @param nameList  The list of names to load.
 * @param nameCount The number of items to load.
 */
RexxReturnCode LocalMacroSpaceManager::loadMacroSpace(const char *target, const char **nameList, size_t nameCount)
{
    NameTable names(nameList, nameCount);

    // now open and read the file header
    MacroSpaceFile file(target);
    // validate the file
    size_t count = file.openForLoading();

    ManagedRxstring image;    // this is outside the loop, which gives us the chance to reuse the buffer

    for (size_t i = 0; i < count; i++)
    {
        char macroName[MacroSpaceDescriptor::MACRONAMESIZE];
        size_t order;

        file.nextMacro(names, macroName, image, order);

        ClientMessage message(MacroSpaceManager, ADD_MACRO, macroName);
        message.parameter1 = image.strlength;
        message.parameter2 = order;

        // attach the queue item to the message.
        message.setMessageData(image.strptr, image.strlength);

        // request the next one.
        message.send();
        // NB:  The only error that can occur here is a critical exception.  We don't
        // need to check the return
    }
    file.close();
    return RXMACRO_OK;
}


/**
 * Save the currently loaded macros into a file.
 *
 * @param target The target file name.
 */
RexxReturnCode LocalMacroSpaceManager::saveMacroSpace(const char *target)
{
    ClientMessage message(MacroSpaceManager, ITERATE_MACRO_DESCRIPTORS);

    message.send();

    // we're empty, no point in this.
    if (message.parameter1 == 0)
    {
        return RXMACRO_OK;
    }

    // now open and write the file header
    MacroSpaceFile file(target);
    file.create(message.parameter1);
    message.operation = NEXT_MACRO_DESCRIPTOR;

    for (;;)
    {
        // request the next one.
        message.send();
        if (message.result == NO_MORE_MACROS)
        {
            break;
        }
        file.writeMacroDescriptor(message.nameArg, message.parameter1, message.parameter2);
    }
    // now iterate the images
    message.operation = ITERATE_MACROS;
    message.send();

    message.operation = NEXT_MACRO_IMAGE;
    for (;;)
    {
        // request the next one.
        message.send();
        if (message.result == NO_MORE_MACROS)
        {
            break;
        }
        file.write(message.getMessageData(), message.getMessageDataLength());
        message.freeMessageData();
    }
    // all done!
    file.close();
    return RXMACRO_OK;
}


/**
 * Retrieve a macro from the daemon server.
 *
 * @param target The target macro name.
 * @param image  The returned image data.
 */
RexxReturnCode LocalMacroSpaceManager::getMacro(const char *target, RXSTRING &image)
{
    ClientMessage message(MacroSpaceManager, GET_MACRO_IMAGE, target);

    // request, then receive the image data
    message.send();
    RexxReturnCode ret = mapReturnResult(message);
    // if this worked, transfer the image data
    if (ret == RXMACRO_OK)
    {
        message.transferMessageData(image);
    }
    return ret;
}

/**
 * Save the currently loaded macrospace using a subset of the
 * loaded macros.
 *
 * @param target The file target.
 * @param names  The list of names.
 * @param count  The number of names in the list.
 */
RexxReturnCode LocalMacroSpaceManager::saveMacroSpace(const char *target, const char **names, size_t count)
{
    // now open and write the file header
    MacroSpaceFile file(target);
    file.create(count);
    size_t i;

    ClientMessage message(MacroSpaceManager, GET_MACRO_DESCRIPTOR);

    for (i = 0; i < count; i++)
    {
        strcpy(message.nameArg, names[i]);
        // request the next one.
        message.send();
        // if not there, time to bail out
        if (message.result == MACRO_DOES_NOT_EXIST)
        {
            return mapReturnResult(message);
        }
        file.writeMacroDescriptor(message.nameArg, message.parameter1, message.parameter2);
    }
    // now iterate the images
    message.operation = GET_MACRO_IMAGE;

    for (i = 0; i < count; i++)
    {
        strcpy(message.nameArg, names[i]);
        // request the next one.  This will throw an exception if it doesn't exist.
        message.send();
        // if not there, time to bail out
        if (message.result == MACRO_DOES_NOT_EXIST)
        {
            return mapReturnResult(message);
        }
        file.write(message.getMessageData(), message.getMessageDataLength());
    }
    // all done!
    file.close();
    return RXMACRO_OK;
}


/**
 * Clear all macros from the macro space.
 */
RexxReturnCode LocalMacroSpaceManager::clearMacroSpace()
{
    ClientMessage message(MacroSpaceManager, CLEAR_MACRO_SPACE);
    message.send();
    return mapReturnResult(message);
}


/**
 * Remove a macro from the macrospace.
 *
 * @param name   The name of the macro to remove.
 */
RexxReturnCode LocalMacroSpaceManager::removeMacro(const char *name)
{
    ClientMessage message(MacroSpaceManager, REMOVE_MACRO, name);
    message.send();
    return mapReturnResult(message);
}


/**
 * Check the macro space for a give item, returning
 * the order information.
 *
 * @param name   The name to check.
 * @param pos
 */
RexxReturnCode LocalMacroSpaceManager::queryMacro(const char *name, size_t *pos)
{
    ClientMessage message(MacroSpaceManager, QUERY_MACRO, name);
    message.send();
    *pos = message.parameter1;
    return mapReturnResult(message);
}


/**
 * Change the search order for a macro item.
 *
 * @param name   The name of the target macro.
 * @param pos    The new search order.
 */
RexxReturnCode LocalMacroSpaceManager::reorderMacro(const char *name, size_t pos)
{
    ClientMessage message(MacroSpaceManager, REORDER_MACRO, name);
    message.parameter1 = pos;
    message.send();
    return mapReturnResult(message);
}


/**
 * Load a macro from a file and store into the macrospace.
 *
 * @param name       The name of the macro.
 * @param sourceFile The target source file.
 * @param position   The macro search position.
 */
RexxReturnCode LocalMacroSpaceManager::addMacroFromFile(const char *name, const char *sourceFile, size_t position)
{
    ManagedRxstring imageData;

    // translate the image
    translateRexxProgram(sourceFile, imageData);
    return addMacro(name, imageData, position);
}


/**
 * Add a macro from image data into the macrospace.
 *
 * @param name      The name of the macro.
 * @param imageData The source image data
 * @param position  The search order position.
 */
RexxReturnCode LocalMacroSpaceManager::addMacro(const char *name, ManagedRxstring &imageData, size_t position)
{
    ClientMessage message(MacroSpaceManager, ADD_MACRO, name);
    // attach the image data
    message.setMessageData(imageData.strptr, imageData.strlength);
                                           // set the additional arguments
    message.parameter1 = imageData.strlength;
    message.parameter2 = position;     // make sure we have the add order

    message.send();
    return mapReturnResult(message);
}


/**
 * Translate a source file into a Rexx program.
 *
 * @param sourceFile The source file name.
 * @param imageData  The returned image data.
 */
void LocalMacroSpaceManager::translateRexxProgram(const char *sourceFile, ManagedRxstring &imageData)
{
    bool opened;

    SysFile *fileInst = new SysFile;
    opened = fileInst->open(sourceFile, RX_O_RDONLY, 0, RX_SH_DENYWR);
    if (opened == false)
    {
        throw new ServiceException(MACRO_SOURCE_NOT_FOUND, "Unable to open macrospace source file");
    }

    int64_t fsize;
    if (fileInst->getSize(fsize) == false)
    {
        throw new ServiceException(MACRO_SOURCE_READ_ERROR, "Unable to read macrospace source file");
    }

    // we define imageData outside this block and sourceData inside.  Once we've
    // translated the file, we're finished with the source, so it can be released
    // once we exit the block.
    {
        SysLibrary lib;
        if (!lib.load("rexx"))
        {
            throw new ServiceException(MACRO_TRANSLATION_ERROR, "Unable to compile Rexx program");
        }

        void *proc = lib.getProcedure("RexxTranslateInstoreProgram");
        if (proc == NULL)
        {
            throw new ServiceException(MACRO_TRANSLATION_ERROR, "Unable to compile Rexx program");
        }

        RexxReturnCode (RexxEntry *compiler)(const char *, CONSTRXSTRING *, RXSTRING *);

        compiler = (RexxReturnCode (RexxEntry *)(const char *, CONSTRXSTRING *, RXSTRING *))proc;

        ManagedRxstring sourceData;
        readRxstringFromFile(fileInst, sourceData, (size_t)fsize);
        fileInst->close();
        imageData.strptr = NULL;
        imageData.strlength = 0;

        RexxReturnCode rc = (*compiler)(sourceFile, (CONSTRXSTRING *)&sourceData, (RXSTRING *)&imageData);
        if (rc != 0)
        {
            throw new ServiceException(MACRO_TRANSLATION_ERROR, "Unable to compile Rexx program");
        }
    }
}

/**
 * Read a buffer of data from a file and return in a
 * ManagedRxString.
 *
 * @param fileInst The source file.
 * @param target   The rxstring used to return the file data.
 * @param size     The size to read.
 */
void LocalMacroSpaceManager::readRxstringFromFile(SysFile * fileInst, ManagedRxstring &target, size_t size)
{
    size_t bytesRead;

    target.strlength = size;
    if (size > 0)                          // if bytes to read           */
    {
        target.ensureCapacity(size);

        fileInst->read(target.strptr, size, bytesRead);
        if (bytesRead != size)
        {
            throw new ServiceException(MACROSPACE_FILE_READ_ERROR, "Unable to read macro space file");
        }
    }
}


/**
 * Translate a service exception into the appropriate
 * API return code.
 *
 * @param e      The returned exception.
 *
 * @return The return code corresponding to the exception information.
 */
RexxReturnCode LocalMacroSpaceManager::processServiceException(ServiceException *e)
{
    switch (e->getErrorCode())
    {
        case MACRO_SOURCE_NOT_FOUND:
        case MACRO_TRANSLATION_ERROR:
            return RXMACRO_SOURCE_NOT_FOUND;

        case MACRO_SOURCE_READ_ERROR:
        case FILE_CREATION_ERROR:
        case MACROSPACE_FILE_READ_ERROR:
        case FILE_READ_ERROR:
        case FILE_WRITE_ERROR:
            return RXMACRO_FILE_ERROR;

        case MACROSPACE_VERSION_ERROR:
        case MACROSPACE_SIGNATURE_ERROR:
            return RXMACRO_SIGNATURE_ERROR;

        case MACRO_DOES_NOT_EXIST:
            return RXMACRO_NOT_FOUND;

        case MACRO_LOAD_REXX:
        default:
            return RXMACRO_NO_STORAGE;
    }
}


/**
 * Process an result returned from the server and
 * map it into an API return code.
 *
 * @param m      The return message.
 *
 * @return The mapped return code.
 */
RexxReturnCode LocalMacroSpaceManager::mapReturnResult(ServiceMessage &m)
{
    switch (m.result)
    {
        // this is generally the only error returned by the server
        case MACRO_DOES_NOT_EXIST:
            return RXMACRO_NOT_FOUND;
        default:
            return RXMACRO_OK;
    }
}
