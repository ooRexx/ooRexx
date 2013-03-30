/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2013-2013 Rexx Language Association. All rights reserved.    */
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

#ifndef oodShellObjects_Included
#define oodShellObjects_Included


#define HINT_ID                     0x00003749
#define DEFAULT_BIF_FLAGS           BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_RETURNFSANCESTORS

#define WRONG_IDL_TYPE_LIST         "a CSIDL_xxx keyword, a full path name, or a pointer to an Item ID List"
#define WRONG_IDL_TYPE_LIST_SHORT   "a CSIDL_xxx keyword or a full path name"
#define NO_ITEMID_MSG               "the Windows Shell did not return the item ID for %s"

#define BFF_TITLE                   "ooDialog Browse for Folder"
#define BFF_BANNER                  "Select the folder needed"
#define BFF_HINT                    "If the needed folder does not exist it can be created"
#define BFF_STARTDIR                ""


/* Struct for the BrowseForFolder CSelf */
typedef struct _bffCSelf
{
    LPITEMIDLIST    root;
    RexxObjectPtr   rexxOwner;
    HWND            hOwner;
    char           *startDir;
    char           *dlgTitle;
    char           *hint;
    char           *banner;
    size_t          countCoInitialized;
    uint32_t        bifFlags;
    uint32_t        coThreadID;
    bool            useHint;
    bool            usePathForHint;
} CBrowseForFolder;
typedef CBrowseForFolder *pCBrowseForFolder;

// Identifies an attribute of the BrowseForFolder object.
typedef enum
{
    DlgTitle,
    DlgBanner,
    DlgHint,
    DlgStartDir,
    BffRoot
} BffAttributeType;



#endif
