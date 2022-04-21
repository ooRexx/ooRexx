/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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

/**
 * oodResources.cpp
 *
 * Contains the classes used for objects representing Windows resources and
 * "resource-like" things.  .Image, .ResourceImage, .ImageList, etc..
 */
#include "ooDialog.hpp"     // Must be first, includes windows.h, commctrl.h, and oorexxapi.h

#include <shlwapi.h>

#if 0
// Future enhancement.
#include <gdiplus.h>
using namespace Gdiplus;
#endif

#include "APICommon.hpp"
#include "ooShapes.hpp"
#include "oodCommon.hpp"
#include "oodControl.hpp"
#include "oodDeviceGraphics.hpp"
#include "oodResources.hpp"


/**
 * Initializes the string to int map for IDs and flags used by images and image
 * lists.  This will included things like a button control's alignment flags for
 * an image list, image list creation flags, OEM icon IDs, etc..
 *
 * @return String2Int*
 *
 * @note  All IDs are included here, except the obsolete ones, and things like
 *        OBM_OLD*, all of which were for 16-bit Windows.
 */
static String2Int *imageInitMap(void)
{
    String2Int *cMap = new String2Int;

    cMap->insert(String2Int::value_type("IDI_APPLICATION", 32512));
    cMap->insert(String2Int::value_type("IDI_HAND",        32513));
    cMap->insert(String2Int::value_type("IDI_QUESTION",    32514));
    cMap->insert(String2Int::value_type("IDI_EXCLAMATION", 32515));
    cMap->insert(String2Int::value_type("IDI_ASTERISK",    32516));
    cMap->insert(String2Int::value_type("IDI_WINLOGO",     32517));

    cMap->insert(String2Int::value_type("IMAGE_BITMAP",      0));
    cMap->insert(String2Int::value_type("IMAGE_ICON",        1));
    cMap->insert(String2Int::value_type("IMAGE_CURSOR",      2));
    cMap->insert(String2Int::value_type("IMAGE_ENHMETAFILE", 3));

    cMap->insert(String2Int::value_type("OCR_NORMAL",      32512));
    cMap->insert(String2Int::value_type("OCR_IBEAM",       32513));
    cMap->insert(String2Int::value_type("OCR_WAIT",        32514));
    cMap->insert(String2Int::value_type("OCR_CROSS",       32515));
    cMap->insert(String2Int::value_type("OCR_UP",          32516));
    cMap->insert(String2Int::value_type("OCR_SIZENWSE",    32642));
    cMap->insert(String2Int::value_type("OCR_SIZENESW",    32643));
    cMap->insert(String2Int::value_type("OCR_SIZEWE",      32644));
    cMap->insert(String2Int::value_type("OCR_SIZENS",      32645));
    cMap->insert(String2Int::value_type("OCR_SIZEALL",     32646));
    cMap->insert(String2Int::value_type("OCR_NO",          32648));
    cMap->insert(String2Int::value_type("OCR_HAND",        32649));
    cMap->insert(String2Int::value_type("OCR_APPSTARTING", 32650));

    cMap->insert(String2Int::value_type("OBM_CLOSE",      32754));
    cMap->insert(String2Int::value_type("OBM_UPARROW",    32753));
    cMap->insert(String2Int::value_type("OBM_DNARROW",    32752));
    cMap->insert(String2Int::value_type("OBM_RGARROW",    32751));
    cMap->insert(String2Int::value_type("OBM_LFARROW",    32750));
    cMap->insert(String2Int::value_type("OBM_REDUCE",     32749));
    cMap->insert(String2Int::value_type("OBM_ZOOM",       32748));
    cMap->insert(String2Int::value_type("OBM_RESTORE",    32747));
    cMap->insert(String2Int::value_type("OBM_REDUCED",    32746));
    cMap->insert(String2Int::value_type("OBM_ZOOMD",      32745));
    cMap->insert(String2Int::value_type("OBM_RESTORED",   32744));
    cMap->insert(String2Int::value_type("OBM_UPARROWD",   32743));
    cMap->insert(String2Int::value_type("OBM_DNARROWD",   32742));
    cMap->insert(String2Int::value_type("OBM_RGARROWD",   32741));
    cMap->insert(String2Int::value_type("OBM_LFARROWD",   32740));
    cMap->insert(String2Int::value_type("OBM_MNARROW",    32739));
    cMap->insert(String2Int::value_type("OBM_COMBO",      32738));
    cMap->insert(String2Int::value_type("OBM_UPARROWI",   32737));
    cMap->insert(String2Int::value_type("OBM_DNARROWI",   32736));
    cMap->insert(String2Int::value_type("OBM_RGARROWI",   32735));
    cMap->insert(String2Int::value_type("OBM_LFARROWI",   32734));
    cMap->insert(String2Int::value_type("OBM_SIZE",       32766));
    cMap->insert(String2Int::value_type("OBM_BTSIZE",     32761));
    cMap->insert(String2Int::value_type("OBM_CHECK",      32760));
    cMap->insert(String2Int::value_type("OBM_CHECKBOXES", 32759));
    cMap->insert(String2Int::value_type("OBM_BTNCORNERS", 32758));

    cMap->insert(String2Int::value_type("LR_DEFAULTCOLOR",     0x0000));
    cMap->insert(String2Int::value_type("LR_MONOCHROME",       0x0001));
    cMap->insert(String2Int::value_type("LR_COLOR",            0x0002));
    cMap->insert(String2Int::value_type("LR_COPYRETURNORG",    0x0004));
    cMap->insert(String2Int::value_type("LR_COPYDELETEORG",    0x0008));
    cMap->insert(String2Int::value_type("LR_LOADFROMFILE",     0x0010));
    cMap->insert(String2Int::value_type("LR_LOADTRANSPARENT",  0x0020));
    cMap->insert(String2Int::value_type("LR_DEFAULTSIZE",      0x0040));
    cMap->insert(String2Int::value_type("LR_VGACOLOR",         0x0080));
    cMap->insert(String2Int::value_type("LR_LOADMAP3DCOLORS",  0x1000));
    cMap->insert(String2Int::value_type("LR_CREATEDIBSECTION", 0x2000));
    cMap->insert(String2Int::value_type("LR_COPYFROMRESOURCE", 0x4000));
    cMap->insert(String2Int::value_type("LR_SHARED",           0x8000));

    // ImageList_Create flags
    cMap->insert(String2Int::value_type("ILC_MASK", 0x0001));
    cMap->insert(String2Int::value_type("ILC_COLOR", 0x0000));
    cMap->insert(String2Int::value_type("ILC_COLORDDB", 0x00FE));
    cMap->insert(String2Int::value_type("ILC_COLOR4", 0x0004));
    cMap->insert(String2Int::value_type("ILC_COLOR8", 0x0008));
    cMap->insert(String2Int::value_type("ILC_COLOR16", 0x0010));
    cMap->insert(String2Int::value_type("ILC_COLOR24", 0x0018));
    cMap->insert(String2Int::value_type("ILC_COLOR32", 0x0020));
    cMap->insert(String2Int::value_type("ILC_PALETTE", 0x0800));
    cMap->insert(String2Int::value_type("ILC_MIRROR", 0x2000));
    cMap->insert(String2Int::value_type("ILC_PERITEMMIRROR", 0x8000));

    // Button image list alignment values
    cMap->insert(String2Int::value_type("BUTTON_IMAGELIST_ALIGN_LEFT",   0));
    cMap->insert(String2Int::value_type("BUTTON_IMAGELIST_ALIGN_RIGHT",  1));
    cMap->insert(String2Int::value_type("BUTTON_IMAGELIST_ALIGN_TOP",    2));
    cMap->insert(String2Int::value_type("BUTTON_IMAGELIST_ALIGN_BOTTOM", 3));
    cMap->insert(String2Int::value_type("BUTTON_IMAGELIST_ALIGN_CENTER", 4));

    cMap->insert(String2Int::value_type("LVSIL_NORMAL", 0));
    cMap->insert(String2Int::value_type("LVSIL_SMALL", 1));
    cMap->insert(String2Int::value_type("LVSIL_STATE", 2));

    cMap->insert(String2Int::value_type("TVSIL_NORMAL", 0));
    cMap->insert(String2Int::value_type("TVSIL_STATE", 2));

    //cMap->insert(String2Int::value_type("", ));

    return cMap;
}

uint32_t mapSymbol(RexxMethodContext *c, CSTRING symbol, size_t argPos, bool raiseException)
{
    static String2Int *imageConstantsMap = NULL;

    if ( imageConstantsMap == NULL )
    {
        imageConstantsMap = imageInitMap();
    }
    int idValue = getKeywordValue(imageConstantsMap, symbol);
    if ( idValue == -1 && raiseException )
    {
        wrongArgValueException(c->threadContext, argPos, "the Image class symbol IDs", symbol);
    }
    return (uint32_t)idValue;
}

/**
 * Gets the image list type from the specified argument object, where the object
 * could be the numeric value, a string keyword, or omitted altogether.
 *
 * @param context
 * @param _type
 * @param defType   The value to use if the argument is omitted.
 * @param argPos
 *
 * @return The image type or OOD_NO_VALUE on error.  An exception has been
 *         raised on error.
 */
static uint32_t getImageTypeArg(RexxMethodContext *context, RexxObjectPtr _type, uint32_t defType, size_t argPos)
{
    uint32_t type = defType;

    if ( argumentExists(argPos) )
    {
        if ( ! context->UnsignedInt32(_type, &type) )
        {
            CSTRING image = context->ObjectToStringValue(_type);
            if (      StrCmpI("BITMAP", image)       == 0 ) type = IMAGE_BITMAP;
            else if ( StrCmpI("ICON", image)         == 0 ) type = IMAGE_ICON;
            else if ( StrCmpI("CURSOR", image)       == 0 ) type = IMAGE_CURSOR;
            else if ( StrCmpI("ENHMETAFILE", image)  == 0 ) type = IMAGE_ENHMETAFILE;
            else
            {
                wrongArgValueException(context->threadContext, argPos, IMAGE_TYPE_LIST, _type);
                type = OOD_NO_VALUE;
            }
        }

        if ( type != OOD_NO_VALUE && type > IMAGE_ENHMETAFILE )
        {
            wrongRangeException(context->threadContext, argPos, IMAGE_BITMAP, IMAGE_ENHMETAFILE, type);
            type = OOD_NO_VALUE;
        }
    }
    return type;
}

/**
 * Gets the size for an image where the size argument must be a .Size object, or
 * could be omitted.
 *
 * @param context
 * @param size      Rexx .Size Object, could be omitted
 * @param defSize   Default size if size is omitted, used to return the new size
 *                  if argument is not omitted.
 * @param argPos
 *
 * @return True on success, false on error.  On error an exception has been
 *         raised.
 */
static bool getImageSizeArg(RexxMethodContext *context, RexxObjectPtr size, SIZE *defSize, size_t argPos)
{
    if ( argumentExists(argPos) )
    {
        SIZE *p = (PSIZE)rxGetSize(context, size, argPos);
        if ( p == NULL )
        {
            return false;
        }
        defSize->cx = p->cx;
        defSize->cy = p->cy;
    }
    return true;
}


/**
 * Gets the image load resource flags the specified argument object, where the
 * object could be the numeric value, a string keyword, or omitted altogether.
 *
 * @param context
 * @param _flags
 * @param defFlags   The value to use if the argument is omitted.
 * @param argPos
 *
 * @return The image flags or OOD_NO_VALUE on error.  An exception has been
 *         raised on error.
 */
static uint32_t getImageFlagsArg(RexxMethodContext *context, RexxObjectPtr _flags, uint32_t defFlags, size_t argPos)
{
    uint32_t flags = defFlags;

    if ( argumentExists(argPos) )
    {
        if ( ! context->UnsignedInt32(_flags, &flags) )
        {
            CSTRING lr = context->ObjectToStringValue(_flags);

            char *dup = strdupupr(lr);
            if ( dup == NULL )
            {
                outOfMemoryException(context->threadContext);
                return OOD_NO_VALUE;
            }

            flags = 0;
            char *token = strtok(dup, " ");
            while ( token != NULL )
            {
                if (      StrCmpI(token, "DEFAULTCOLOR")     == 0 ) flags |= LR_DEFAULTCOLOR           ;
                else if ( StrCmpI(token, "MONOCHROME")       == 0 ) flags |= LR_MONOCHROME             ;
                else if ( StrCmpI(token, "COLOR")            == 0 ) flags |= LR_COLOR                  ;
                else if ( StrCmpI(token, "COPYRETURNORG")    == 0 ) flags |= LR_COPYRETURNORG          ;
                else if ( StrCmpI(token, "COPYDELETEORG")    == 0 ) flags |= LR_COPYDELETEORG          ;
                else if ( StrCmpI(token, "LOADFROMFILE")     == 0 ) flags |= LR_LOADFROMFILE           ;
                else if ( StrCmpI(token, "LOADTRANSPARENT")  == 0 ) flags |= LR_LOADTRANSPARENT        ;
                else if ( StrCmpI(token, "DEFAULTSIZE")      == 0 ) flags |= LR_DEFAULTSIZE            ;
                else if ( StrCmpI(token, "VGACOLOR")         == 0 ) flags |= LR_VGACOLOR               ;
                else if ( StrCmpI(token, "LOADMAP3DCOLORS")  == 0 ) flags |= LR_LOADMAP3DCOLORS        ;
                else if ( StrCmpI(token, "CREATEDIBSECTION") == 0 ) flags |= LR_CREATEDIBSECTION       ;
                else if ( StrCmpI(token, "COPYFROMRESOURCE") == 0 ) flags |= LR_COPYFROMRESOURCE       ;
                else if ( StrCmpI(token, "SHARED")           == 0 ) flags |= LR_SHARED                 ;
                else
                {
                    wrongArgKeywordsException(context->threadContext, argPos, IMAGE_FLAGS_LIST, _flags);
                    flags = OOD_NO_VALUE;
                    break;
                }

                token = strtok(NULL, " ");
            }
            free(dup);
        }

        if ( flags != OOD_NO_VALUE )
        {
            // The user specified flags.  Use some safeguards, determined by the
            // value of the default flags.  In all other cases, assume the user
            // knows best.

            if ( defFlags == LR_LOADFROMFILE )
            {
                // Ensure the user did not use shared and did use load from file.
                flags = (flags &  ~LR_SHARED) | LR_LOADFROMFILE;
            }
            else if ( defFlags == (LR_SHARED | LR_DEFAULTSIZE) )
            {
                // Ensure the user did not use load from file and did use shared.
                flags = (flags &  ~LR_LOADFROMFILE) | LR_SHARED;
            }
        }
    }
    return flags;
}

uint32_t getSystemImageID(RexxMethodContext *c, RexxObjectPtr id, size_t argPos)
{
    uint32_t resourceID;

    if ( ! c->UnsignedInt32(id, &resourceID) )
    {
        CSTRING keyword = c->ObjectToStringValue(id);

        if (      StrCmpI(keyword, "IDI_HAND"       )     == 0 ) resourceID = 32513                     ;
        else if ( StrCmpI(keyword, "IDI_QUESTION"   )     == 0 ) resourceID = 32514                     ;
        else if ( StrCmpI(keyword, "IDI_EXCLAMATION")     == 0 ) resourceID = 32515                     ;
        else if ( StrCmpI(keyword, "IDI_ASTERISK"   )     == 0 ) resourceID = 32516                     ;
        else if ( StrCmpI(keyword, "IDI_WINLOGO"    )     == 0 ) resourceID = 32517                     ;
        else if ( StrCmpI(keyword, "IDI_SHEILD"     )     == 0 ) resourceID = 32518                     ;
        else if ( StrCmpI(keyword, "OCR_NORMAL"     )     == 0 ) resourceID = OCR_NORMAL                ;
        else if ( StrCmpI(keyword, "OCR_IBEAM"      )     == 0 ) resourceID = OCR_IBEAM                 ;
        else if ( StrCmpI(keyword, "OCR_WAIT"       )     == 0 ) resourceID = OCR_WAIT                  ;
        else if ( StrCmpI(keyword, "OCR_CROSS"      )     == 0 ) resourceID = OCR_CROSS                 ;
        else if ( StrCmpI(keyword, "OCR_UP"         )     == 0 ) resourceID = OCR_UP                    ;
        else if ( StrCmpI(keyword, "OCR_SIZENWSE"   )     == 0 ) resourceID = OCR_SIZENWSE              ;
        else if ( StrCmpI(keyword, "OCR_SIZENESW"   )     == 0 ) resourceID = OCR_SIZENESW              ;
        else if ( StrCmpI(keyword, "OCR_SIZEWE"     )     == 0 ) resourceID = OCR_SIZEWE                ;
        else if ( StrCmpI(keyword, "OCR_SIZENS"     )     == 0 ) resourceID = OCR_SIZENS                ;
        else if ( StrCmpI(keyword, "OCR_SIZEALL"    )     == 0 ) resourceID = OCR_SIZEALL               ;
        else if ( StrCmpI(keyword, "OCR_NO"         )     == 0 ) resourceID = OCR_NO                    ;
        else if ( StrCmpI(keyword, "OCR_HAND"       )     == 0 ) resourceID = OCR_HAND                  ;
        else if ( StrCmpI(keyword, "OCR_APPSTARTING")     == 0 ) resourceID = OCR_APPSTARTING           ;
        else if ( StrCmpI(keyword, "OBM_CLOSE"      )     == 0 ) resourceID = OBM_CLOSE                 ;
        else if ( StrCmpI(keyword, "OBM_UPARROW"    )     == 0 ) resourceID = OBM_UPARROW               ;
        else if ( StrCmpI(keyword, "OBM_DNARROW"    )     == 0 ) resourceID = OBM_DNARROW               ;
        else if ( StrCmpI(keyword, "OBM_RGARROW"    )     == 0 ) resourceID = OBM_RGARROW               ;
        else if ( StrCmpI(keyword, "OBM_LFARROW"    )     == 0 ) resourceID = OBM_LFARROW               ;
        else if ( StrCmpI(keyword, "OBM_REDUCE"     )     == 0 ) resourceID = OBM_REDUCE                ;
        else if ( StrCmpI(keyword, "OBM_ZOOM"       )     == 0 ) resourceID = OBM_ZOOM                  ;
        else if ( StrCmpI(keyword, "OBM_RESTORE"    )     == 0 ) resourceID = OBM_RESTORE               ;
        else if ( StrCmpI(keyword, "OBM_REDUCED"    )     == 0 ) resourceID = OBM_REDUCED               ;
        else if ( StrCmpI(keyword, "OBM_ZOOMD"      )     == 0 ) resourceID = OBM_ZOOMD                 ;
        else if ( StrCmpI(keyword, "OBM_RESTORED"   )     == 0 ) resourceID = OBM_RESTORED              ;
        else if ( StrCmpI(keyword, "OBM_UPARROWD"   )     == 0 ) resourceID = OBM_UPARROWD              ;
        else if ( StrCmpI(keyword, "OBM_DNARROWD"   )     == 0 ) resourceID = OBM_DNARROWD              ;
        else if ( StrCmpI(keyword, "OBM_RGARROWD"   )     == 0 ) resourceID = OBM_RGARROWD              ;
        else if ( StrCmpI(keyword, "OBM_LFARROWD"   )     == 0 ) resourceID = OBM_LFARROWD              ;
        else if ( StrCmpI(keyword, "OBM_MNARROW"    )     == 0 ) resourceID = OBM_MNARROW               ;
        else if ( StrCmpI(keyword, "OBM_COMBO"      )     == 0 ) resourceID = OBM_COMBO                 ;
        else if ( StrCmpI(keyword, "OBM_UPARROWI"   )     == 0 ) resourceID = OBM_UPARROWI              ;
        else if ( StrCmpI(keyword, "OBM_DNARROWI"   )     == 0 ) resourceID = OBM_DNARROWI              ;
        else if ( StrCmpI(keyword, "OBM_RGARROWI"   )     == 0 ) resourceID = OBM_RGARROWI              ;
        else if ( StrCmpI(keyword, "OBM_LFARROWI"   )     == 0 ) resourceID = OBM_LFARROWI              ;
        else if ( StrCmpI(keyword, "OBM_SIZE"       )     == 0 ) resourceID = OBM_SIZE                  ;
        else if ( StrCmpI(keyword, "OBM_BTSIZE"     )     == 0 ) resourceID = OBM_BTSIZE                ;
        else if ( StrCmpI(keyword, "OBM_CHECK"      )     == 0 ) resourceID = OBM_CHECK                 ;
        else if ( StrCmpI(keyword, "OBM_CHECKBOXES" )     == 0 ) resourceID = OBM_CHECKBOXES            ;
        else if ( StrCmpI(keyword, "OBM_BTNCORNERS" )     == 0 ) resourceID = OBM_BTNCORNERS            ;
        else
        {
            resourceID = OOD_NO_VALUE;
        }
    }

    return resourceID;
}

/**
 * Methods for the .ImageList class.
 */
#define IMAGELIST_CLASS "ImageList"


#define IL_DEFAULT_FLAGS           ILC_COLOR32 | ILC_MASK
#define IL_DEFAULT_COUNT           6
#define IL_DEFAULT_GROW            0

/**
 * Gets the image list create flags the from specified argument object, where
 * the object could be the numeric value, a string keyword, or omitted
 * altogether.
 *
 * @param context
 * @param _flags
 * @param defFlags   The value to use if the argument is omitted.
 * @param argPos
 *
 * @return The image flags.
 *
 */
static uint32_t getImageListCreateFlagsArg(RexxMethodContext *context, RexxObjectPtr _flags,
                                           uint32_t defFlags, size_t argPos)
{
    uint32_t flags = defFlags;

    if ( argumentExists(argPos) )
    {
        if ( ! context->UnsignedInt32(_flags, &flags) )
        {
            CSTRING ilc = context->ObjectToStringValue(_flags);

            char *dup = strdupupr(ilc);
            if ( dup == NULL )
            {
                outOfMemoryException(context->threadContext);
                return OOD_NO_VALUE;
            }

            flags = 0;
            char *token = strtok(dup, " ");
            while ( token != NULL )
            {
                if (      strcmp(token, "MASK")             == 0 ) flags |= ILC_MASK                  ;
                else if ( strcmp(token, "COLOR")            == 0 ) flags |= ILC_COLOR                 ;
                else if ( strcmp(token, "COLORDDB")         == 0 ) flags |= ILC_COLORDDB              ;
                else if ( strcmp(token, "COLOR4")           == 0 ) flags |= ILC_COLOR4                ;
                else if ( strcmp(token, "COLOR8")           == 0 ) flags |= ILC_COLOR8                ;
                else if ( strcmp(token, "COLOR16")          == 0 ) flags |= ILC_COLOR16               ;
                else if ( strcmp(token, "COLOR24")          == 0 ) flags |= ILC_COLOR24               ;
                else if ( strcmp(token, "COLOR32")          == 0 ) flags |= ILC_COLOR32               ;
                else if ( strcmp(token, "PALETTE")          == 0 ) flags |= ILC_PALETTE               ;
                else if ( strcmp(token, "MIRROR")           == 0 ) flags |= ILC_MIRROR                ;
                else if ( strcmp(token, "PERITEMMIRROR")    == 0 ) flags |= ILC_PERITEMMIRROR         ;
                else if ( strcmp(token, "ORIGINALSIZE")     == 0 ) flags |= ILC_ORIGINALSIZE          ;
                else if ( strcmp(token, "HIGHQUALITYSCALE") == 0 ) flags |= ILC_HIGHQUALITYSCALE      ;
                else
                {
                    wrongArgKeywordsException(context->threadContext, argPos, IMAGELIST_CREATE_LIST, _flags);
                    flags = OOD_NO_VALUE;
                    break;
                }

                token = strtok(NULL, " ");
            }
            free(dup);
        }
    }
    return flags;
}


/**
 * Map a keyword string to the proper ILD_* flags.  By default ILD_NORMAL is
 * used when the argument is omitted (flags will be null.)  ILD_NORMAL is 0
 * anyway.
 *
 * @param flags
 *
 * @return uint32_t
 */
uint32_t keyword2ild(RexxMethodContext *c, CSTRING flags, size_t argPos)
{
    uint32_t val = ILD_NORMAL;

    if ( flags != NULL )
    {
        char *dup = strdupupr(flags);
        if ( dup == NULL )
        {
            outOfMemoryException(c->threadContext);
            return OOD_NO_VALUE;
        }

        char *token = strtok(dup, " ");
        while ( token != NULL )
        {
            if (      strcmp(flags, "BLEND"       ) == 0 ) val |= ILD_BLEND          ;
            else if ( strcmp(flags, "BLEND25"     ) == 0 ) val |= ILD_BLEND25        ;
            else if ( strcmp(flags, "BLEND50"     ) == 0 ) val |= ILD_BLEND50        ;
            else if ( strcmp(flags, "FOCUS"       ) == 0 ) val |= ILD_FOCUS          ;
            else if ( strcmp(flags, "MASK"        ) == 0 ) val |= ILD_MASK           ;
            else if ( strcmp(flags, "NORMAL"      ) == 0 ) val |= ILD_NORMAL         ;
            else if ( strcmp(flags, "SELECTED"    ) == 0 ) val |= ILD_SELECTED       ;
            else if ( strcmp(flags, "TRANSPARENT" ) == 0 ) val |= ILD_TRANSPARENT    ;
            else
            {
                wrongArgValueException(c->threadContext, argPos, LOAD_RESOURCE_LIST, flags);
                val = OOD_NO_VALUE;
                break;
            }

            token = strtok(NULL, " ");
        }
        free(dup);
    }

    return val;
}

HIMAGELIST rxGetImageList(RexxMethodContext *context, RexxObjectPtr il, size_t argPos)
{
    HIMAGELIST himl = NULL;
    if ( requiredClass(context->threadContext, il, "ImageList", argPos) )
    {
        // Make sure we don't use a null ImageList.
        himl = (HIMAGELIST)context->ObjectToCSelf(il);
        if ( himl == NULL )
        {
            nullObjectException(context->threadContext, IMAGELIST_CLASS, argPos);
        }
    }
    return himl;
}

RexxObjectPtr rxNewImageList(RexxMethodContext *c, HIMAGELIST himl)
{
    RexxObjectPtr imageList = NULL;

    RexxClassObject theClass = rxGetContextClass(c, "IMAGELIST");
    if ( theClass != NULL )
    {
        imageList = c->SendMessage1(theClass, "NEW", c->NewPointer(himl));
    }
    return imageList;
}

/**
 * Creates a Windows ImageList and the corresponding ooDialog .ImageList object
 * from a single bitmap.
 *
 * The Windows ImageList supports adding any number of images from a single
 * bitmap.  The individual images are assumed to be side-by-side in the bitmap.
 * The number of images is determined by the width of a single image.
 *
 * At this time, this function is used to allow the ooDialog programmer to
 * assign an image list to a dialog control by just passing in a bitmap, rather
 * than first creating an .ImageList object.  This is much less flexible, but
 * allows the programmer to write fewer lines of code.  In addition, it mimics
 * the behavior of pre-4.0 code allowing that code to be removed.
 *
 * @param c       The method context we are operating in.
 * @param himl    [in / out] The created handle of the ImageList is returned.
 * @param ilSrc   The bitmap.
 * @param width   [optional]  The width of a single image.  When omitted, the
 *                height of the actual bitmap is used for the width.
 * @param height  [optional]  The height of a single image.  If omitted the
 *                height of the actual bitmap is used.
 * @param hwnd    The window handle of the control.  Used to create a device
 *                context if needed.
 *
 * @return An instantiated .ImageList object on success, or NULLOBJECT on
 *         failure.
 *
 * @note These objects are accepted for the image list source (ilSrc): .Image
 *       object, a bitmap file name, a bitmap handle.  A bitmap handle can be
 *       either a pointer string, or a .Pointer object.  The bitmap handle is
 *       needed to provide backward compatibility, but its use is discouraged.
 *
 * @note This function needs to support the original ooDialog design where
 *       bitmaps were loaded as DIBs.  If the image list source is a handle,
 *       GetObject() is used to test if the bitmap is a compatible bitmap (DDB.)
 *       If GetObject() returns 0, it is still a device independent (DIB) and
 *       needs to be converted to a device dependent bitmap.
 */
RexxObjectPtr oodILFromBMP(RexxMethodContext *c, HIMAGELIST *himl, RexxObjectPtr ilSrc,
                          int width, int height, HWND hwnd)
{
    HBITMAP hDDB = NULL;
    RexxObjectPtr imageList = NULLOBJECT;
    bool canRelease = false;
    BITMAP bmpInfo;

    if ( c->IsOfType(ilSrc, "Image") )
    {
        POODIMAGE oi = rxGetImageBitmap(c, ilSrc, 1);
        if ( oi == NULLOBJECT )
        {
            goto done_out;
        }
        hDDB = (HBITMAP)oi->hImage;
    }
    else if ( c->IsString(ilSrc) || c->IsPointer(ilSrc) )
    {
        CSTRING bitmap = c->ObjectToStringValue(ilSrc);

        // See if the user passed in the handle to an already loaded bitmap.
        hDDB = (HBITMAP)string2pointer(bitmap);
        if ( hDDB != NULL )
        {
            if ( GetObject(hDDB, sizeof(BITMAP), &bmpInfo) == 0 )
            {
                HDC dc = GetDC(hwnd);
                hDDB = CreateDIBitmap(dc, (BITMAPINFOHEADER*)hDDB, CBM_INIT, dibPBits(hDDB),
                                      dibPBI(hDDB), DIB_RGB_COLORS);
                if ( hDDB == NULL )
                {
                    oodSetSysErrCode(c->threadContext);
                    ReleaseDC(hwnd, dc);
                    goto done_out;
                }
                ReleaseDC(hwnd, dc);
                canRelease = true;
            }
        }
        else
        {
            hDDB = (HBITMAP)LoadImage(NULL, bitmap, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
            if ( hDDB == NULL )
            {
                oodSetSysErrCode(c->threadContext);
                goto done_out;
            }
            canRelease = true;
        }
    }
    else
    {
        wrongArgValueException(c->threadContext, 1, "ImageList, Image, bitmap file name, bitmap handle", ilSrc);
        goto done_out;
    }

    if ( GetObject(hDDB, sizeof(BITMAP), &bmpInfo) == sizeof(BITMAP) )
    {
        if ( width == 0 )
        {
            width = bmpInfo.bmHeight;
        }
        if ( height == 0 )
        {
            height = bmpInfo.bmHeight;
        }
        int count = bmpInfo.bmWidth / width;

        HIMAGELIST il = ImageList_Create(width, height, ILC_COLOR8, count, 0);
        if ( il != NULL )
        {
            if ( ImageList_Add(il, hDDB, NULL) == -1 )
            {
                ImageList_Destroy(il);
                goto done_out;
            }

            imageList = rxNewImageList(c, il);
            *himl = il;
        }
    }

done_out:
    if ( hDDB && canRelease )
    {
        DeleteObject(hDDB);
    }
    return imageList;
}

/** ImageList::create()    [class method]
 *
 *
 *
 */
RexxMethod4(RexxObjectPtr, il_create_cls, OPTIONAL_RexxObjectPtr, size,  OPTIONAL_RexxObjectPtr, _flags,
            OPTIONAL_int32_t, count, OPTIONAL_int32_t, grow)
{
    RexxMethodContext *c = context;
    RexxObjectPtr result = TheNilObj;

    SIZE s = {0};
    if ( argumentExists(1) )
    {
        SIZE *p = (PSIZE)rxGetSize(c, size, 3);
        if ( p == NULL )
        {
            goto out;
        }
        s.cx = p->cx;
        s.cy = p->cy;
    }
    else
    {
        s.cx = GetSystemMetrics(SM_CXICON);
        s.cy = GetSystemMetrics(SM_CYICON);
    }

    uint32_t flags = getImageListCreateFlagsArg(context, _flags, IL_DEFAULT_FLAGS, 2);
    if ( (flags & (ILC_ORIGINALSIZE | ILC_HIGHQUALITYSCALE)) && (! requiredComCtl32Version(c, "init", COMCTL32_6_0)) )
    {
        goto out;
    }

    if ( argumentOmitted(3) )
    {
        count = IL_DEFAULT_COUNT;
    }
    if ( argumentOmitted(4) )
    {
        grow = IL_DEFAULT_GROW;
    }

    HIMAGELIST himl = ImageList_Create(s.cx, s.cy, flags, count, grow);
    result = rxNewImageList(c, himl);

out:
    return result;
}

/** ImageList::init()
 *
 *
 *  @note  As far as I can see, all of the ImageList_xxx functions do not blow
 *         up if an invalid handle is used, even if it is null.  We could just
 *         set CSELF to the pointer value unconditionally and not have to worry
 *         about an interpreter crash.  The ooRexx programmer would just have a
 *         .ImageList object that didn't work.
 *
 *         A valid image list can be released and then becomes invalid (isNull
 *         returns true.)  Since this is the same behavior as .ResourceImage and
 *         .Image objects, both of which allow a null object to be instantiated
 *         (isNull returns true,) we allow a null ImageList to be created.
 *
 *         However, if p is not null, we test that p is actually a valid
 *         ImageList and raise an exception if it is not. All image lists have a
 *         size, if ImageListGetIconSize() fails, then p is not an image list
 *         handle.
 */
RexxMethod1(RexxObjectPtr, il_init, POINTER, p)
{
    if ( p == NULL )
    {
        context->SetObjectVariable("CSELF", context->NewPointer(NULL));
        goto out;
    }
    HIMAGELIST himl = (HIMAGELIST)p;

    // Test that the pointer is really a valid handle to an image list.
    int cx = 2, cy = 2;
    if ( ! ImageList_GetIconSize(himl, &cx, &cy) )
    {
        invalidTypeException(context->threadContext, 1, "ImageList handle");
        goto out;
    }
    context->SetObjectVariable("CSELF", context->NewPointer(himl));

out:
    return NULLOBJECT;
}

/** ImageList::add()
 *
 *
 *
 */
RexxMethod3(int, il_add, RexxObjectPtr, image, OPTIONAL_RexxObjectPtr, optMask, CSELF, il)
{
    RexxMethodContext *c = context;
    int result = -1;

    HIMAGELIST himl = (HIMAGELIST)il;
    if ( himl == NULL )
    {
        nullObjectException(c->threadContext, IMAGELIST_CLASS);
        goto out;
    }

    POODIMAGE oi = rxGetImageBitmap(c, image, 1);
    if ( oi == NULL )
    {
        goto out;
    }

    HBITMAP mask = NULL;
    if ( argumentExists(2) )
    {
        POODIMAGE tmp = rxGetImageBitmap(c, optMask, 2);
        if ( tmp == NULL )
        {
            goto out;
        }
        mask =  (HBITMAP)tmp->hImage;
    }

    result = ImageList_Add(himl, (HBITMAP)oi->hImage, mask);

out:
    return result;
}

/** ImageList::addIcon()
 *
 *
 *
 */
RexxMethod2(int, il_addIcon, RexxObjectPtr, image, CSELF, il)
{
    int result = -1;

    HIMAGELIST himl = (HIMAGELIST)il;
    if ( himl == NULL )
    {
        nullObjectException(context->threadContext, IMAGELIST_CLASS);
        goto out;
    }
    POODIMAGE oi = rxGetImageIcon(context, image, 1);
    if ( oi == NULL )
    {
        goto out;
    }
    result = ImageList_AddIcon(himl, (HICON)oi->hImage);

out:
    return result;
}

/** ImageList::addImages()
 *
 *
 *
 */
RexxMethod3(int, il_addImages, RexxArrayObject, images, OPTIONAL_uint32_t, cRef, CSELF, il)
{
    RexxMethodContext *c = context;
    int result = -1;
    int tmpResult = -1;

    HIMAGELIST himl = (HIMAGELIST)il;
    if ( himl == NULL )
    {
        nullObjectException(context->threadContext, IMAGELIST_CLASS);
        goto out;
    }
    size_t count = c->ArraySize(images);
    if ( count < 1 )
    {
        emptyArrayException(c->threadContext, 1);
        goto out;
    }

    uint8_t imageType = 0;
    bool doMasked = false;

    for ( size_t i = 1; i <= count; i++)
    {
        RexxObjectPtr image = c->ArrayAt(images, i);
        if ( image == NULLOBJECT || ! c->IsOfType(image, "Image") )
        {
            wrongObjInArrayException(c->threadContext, 1, i, "an Image object");
            goto out;
        }
        POODIMAGE oi = (POODIMAGE)context->ObjectToCSelf(image);
        if ( oi->hImage == NULL )
        {
            wrongObjInArrayException(c->threadContext, 1, i, "a non-null Image object");
            goto out;
        }

        if ( imageType == 0 )
        {
            imageType = -1;
            if ( oi->type == IMAGE_CURSOR || oi->type == IMAGE_ICON )
            {
                imageType = IMAGE_ICON;
            }
            else if ( oi->type == IMAGE_BITMAP )
            {
                imageType = IMAGE_BITMAP;
                doMasked = argumentExists(2) ? true : false;
            }

            if ( imageType == -1 )
            {
                wrongObjInArrayException(c->threadContext, 1, i, "a bitmap, icon, or cursor Image");
                goto out;
            }
        }

        switch ( oi->type )
        {
            case IMAGE_ICON :
            case IMAGE_CURSOR :
                if ( imageType != IMAGE_ICON )
                {
                    wrongObjInArrayException(c->threadContext, 1, i, "a cursor or icon Image");
                    goto out;
                }
                tmpResult = ImageList_AddIcon(himl, (HICON)oi->hImage);
                break;

            case IMAGE_BITMAP :
                if ( imageType != IMAGE_BITMAP )
                {
                    wrongObjInArrayException(c->threadContext, 1, i, "a bitmap Image");
                    goto out;
                }
                if ( doMasked )
                {
                    tmpResult = ImageList_AddMasked(himl, (HBITMAP)oi->hImage, cRef);
                }
                else
                {
                    tmpResult = ImageList_Add(himl, (HBITMAP)oi->hImage, NULL);
                }
                break;

            default :
                wrongObjInArrayException(c->threadContext, 1, i, "a bitmap, icon, or cursor Image");
                goto out;

        }

        if ( tmpResult == -1 )
        {
            break;
        }
        result = tmpResult;
    }

out:
    return result;
}

/** ImageList::addMasked()
 *
 *
 *
 */
RexxMethod3(int, il_addMasked, RexxObjectPtr, image, uint32_t, cRef, CSELF, il)
{
    int result = -1;

    HIMAGELIST himl = (HIMAGELIST)il;
    if ( himl == NULL )
    {
        nullObjectException(context->threadContext, IMAGELIST_CLASS);
        goto out;
    }

    POODIMAGE oi = rxGetImageBitmap(context, image, 1);
    if ( oi == NULL )
    {
        goto out;
    }
    result = ImageList_AddMasked(himl, (HBITMAP)oi->hImage, cRef);

out:
    return result;
}

/** ImageList::duplicate()
 *
 *
 */
RexxMethod1(RexxObjectPtr, il_duplicate, CSELF, il)
{
    HIMAGELIST himl = (HIMAGELIST)il;
    if ( himl != NULL )
    {
        return rxNewImageList(context, ImageList_Duplicate(himl));
    }
    nullObjectException(context->threadContext, IMAGELIST_CLASS);
    return NULL;
}

/** ImageList::getCount()
 *
 *
 *  @return  The count of images in the image list.
 */
RexxMethod1(int, il_getCount, CSELF, il)
{
    HIMAGELIST himl = (HIMAGELIST)il;
    if ( himl != NULL )
    {
        return ImageList_GetImageCount(himl);
    }
    nullObjectException(context->threadContext, IMAGELIST_CLASS);
    return NULL;
}



/** ImageList::getIcon()
 *
 *  @param index  [required] The one-based index of the icon to get.
 *
 *  @param style  [optional] The drawing style of the icon.  If this argument is
 *                omitted, the NORMAL style is used
 *
 *  @param overlayIndex  [optional]  The one-based index of an overlay mask.
 *
 *  @return  The specified icon in this image list or .nil on error.
 *
 *  @remarks  For an Image_List, the overlay index is already one-based.
 */
RexxMethod4(RexxObjectPtr, il_getIcon, uint32_t, index, OPTIONAL_CSTRING, _style,
            OPTIONAL_uint32_t, overlayIndex, CSELF, il)
{
    HIMAGELIST himl = (HIMAGELIST)il;
    if ( himl != NULL )
    {
        uint32_t style = keyword2ild(context, _style, 2);
        if ( style == OOD_NO_VALUE )
        {
            return NULLOBJECT;
        }

        if ( argumentExists(3) )
        {
            style |= INDEXTOOVERLAYMASK(overlayIndex);
        }
        index--;
        HICON icon = ImageList_GetIcon(himl, index, style);

        if ( icon != NULL )
        {
            SIZE s;
            if ( ImageList_GetIconSize(himl, (int *)&s.cx, (int *)&s.cy) )
            {
                // LR_DEFAULTCOLOR means nothing, but I think that is correct
                // for the flags so we use true for the last arg.
                return rxNewValidImage(context, icon, IMAGE_ICON, &s, LR_DEFAULTCOLOR, true);
            }
        }
        return TheNilObj;
    }
    nullObjectException(context->threadContext, IMAGELIST_CLASS);
    return NULLOBJECT;
}

/** ImageList::getImageSize()
 *
 *
 * @return  A .Size object containing the size of an image on success, or .nil
 *          on failure.
 */
RexxMethod1(RexxObjectPtr, il_getImageSize, CSELF, il)
{
    HIMAGELIST himl = (HIMAGELIST)il;
    if ( himl != NULL )
    {
        SIZE s;
        if ( ImageList_GetIconSize(himl, (int *)&s.cx, (int *)&s.cy) == 0 )
        {
            return TheNilObj;
        }
        else
        {
            return rxNewSize(context, s.cx, s.cy);
        }
    }
    nullObjectException(context->threadContext, IMAGELIST_CLASS);
    return NULL;
}

/** ImageList::handle()
 *
 *
 *
 */
RexxMethod1(POINTER, il_handle, CSELF, il)
{
    if ( il == NULL )
    {
        nullObjectException(context->threadContext, IMAGELIST_CLASS);
    }
    return il;
}

/** ImageList::isNull()
 *
 *
 *
 */
RexxMethod1(logical_t, il_isNull, CSELF, il) { return ( il == NULL);  }

/** ImageList::release()
 *
 *
 *
 */
RexxMethod1(uint32_t, il_release, CSELF, il)
{
    if ( il != NULL )
    {
        ImageList_Destroy((HIMAGELIST)il);
        context->SetObjectVariable("CSELF", context->NewPointer(NULL));
    }
    return 0;
}

/** ImageList::remove()
 *
 *
 *
 */
RexxMethod2(logical_t, il_remove, int32_t, index, CSELF, il)
{
    HIMAGELIST himl = (HIMAGELIST)il;
    if ( himl != NULL )
    {
        index--;
        return ImageList_Remove(himl, index);
    }
    nullObjectException(context->threadContext, IMAGELIST_CLASS);
    return FALSE;
}

/** ImageList::removeAll()
 *
 *
 *
 */
RexxMethod1(logical_t, il_removeAll, CSELF, il)
{
    HIMAGELIST himl = (HIMAGELIST)il;
    if ( himl != NULL )
    {
        return ImageList_RemoveAll(himl);
    }
    nullObjectException(context->threadContext, IMAGELIST_CLASS);
    return NULL;
}


/**
 * Methods for the .Image class.
 */
#define IMAGE_CLASS "Image"


RexxObjectPtr rxNewImageObject(RexxMethodContext *c, RexxBufferObject bufferObj)
{
    RexxObjectPtr image = NULLOBJECT;

    RexxClassObject ImageClass = rxGetContextClass(c, "Image");
    if ( ImageClass != NULL )
    {
        image = c->SendMessage1(ImageClass, "NEW", bufferObj);
    }
    return image;
}

/**
 * Instantiates a new, non-null, .Image object.
 *
 * @param context
 * @param hImage
 * @param type
 * @param s
 * @param flags
 * @param src       True, ooDialog created using LoadImage(). False created from
 *                  a handle (so type, size, flags may not be correct.)
 *
 * @return  A new .Image object.
 */
RexxObjectPtr rxNewValidImage(RexxMethodContext *c, HANDLE hImage, uint8_t type, PSIZE s, uint32_t flags, bool src)
{
    RexxBufferObject bufferObj = c->NewBuffer(sizeof(OODIMAGE));
    POODIMAGE cself = (POODIMAGE)c->BufferData(bufferObj);

    cself->hImage = hImage;
    cself->type = type;
    cself->size.cx = s->cx;
    cself->size.cy = s->cy;
    cself->flags = flags;
    cself->isValid = true;
    cself->srcOOD = src;
    cself->canRelease = ! (flags & LR_SHARED);
    cself->typeName = getImageTypeName(type);
    cself->lastError = 0;
    cself->fileName = "";

    return rxNewImageObject(c, bufferObj);
}

/**
 * Creates an .Image object from an image handle retrieved from a dialog
 * control.
 *
 * If the image had been set from ooDialog code, the .Image object would be
 * known.  Therefore, this an image assigned to the control, loaded from a
 * resource DLL.  The assumption then is, that the OS loaded the image as
 * LR_SHARED.  (Is this true?)
 *
 * We need to create an .Image object. If the image type is not passed in,
 * (type=-1,) we can deduce the type (possibly) from the control style, but not
 * the size or all the flags.  However, we do use the LR_SHARED flag based on
 * the above assumption.
 *
 * When the process that loaded the image ends, the OS will clean up the image
 * resource (MSDN.)  Using LR_SHARED will prevent the user from releasing an
 * image that shouldn't be.
 *
 * @param c       Method context we are operating in.
 * @param hwnd    Window handle of the dialog control.
 * @param hImage  Handle to the image.
 * @param type    Image type.
 * @param ctrl    Type of dialog control.
 *
 * @return   A new .Image object.
 */
RexxObjectPtr rxNewImageFromControl(RexxMethodContext *c, HWND hwnd, HANDLE hImage, uint8_t type,
                                    oodControl_t ctrl)
{
    SIZE s = {0};

    // If the caller did not know the type, try to deduce it.
    if ( type == -1 )
    {
        LONG style = GetWindowLong(hwnd, GWL_STYLE);
        switch ( ctrl )
        {
            case winStatic :
                // If it is a cursor image, the control type is SS_ICON.
                switch ( style & SS_TYPEMASK )
                {
                    case SS_BITMAP :
                        type = IMAGE_BITMAP;
                        break;
                    case SS_ENHMETAFILE :
                        type = IMAGE_ENHMETAFILE;
                        break;
                    case SS_ICON :
                    default :
                        type = IMAGE_ICON;
                        break;
                }
                break;

            case winPushButton :
                switch ( style & BS_IMAGEMASK )
                {
                    case BS_BITMAP :
                        type = IMAGE_BITMAP;
                        break;
                    case BS_ICON :
                    default :
                        type = IMAGE_ICON;
                        break;
                }
                break;

            default :
                // Shouldn't happen
                type = IMAGE_BITMAP;
                break;

        }
    }
    return rxNewValidImage(c, hImage, type, &s, LR_SHARED, false);
}


RexxObjectPtr oodSetImageAttribute(RexxMethodContext *c, CSTRING varName, RexxObjectPtr image, HWND hwnd,
                                   HANDLE hOldImage, uint8_t type, oodControl_t ctrl)
{
    RexxObjectPtr result = c->GetObjectVariable(varName);
    if ( result == NULLOBJECT )
    {
        result = TheNilObj;
    }
    c->SetObjectVariable(varName, image);

    // It could be that the existing image was set from a resource DLL.  In
    // which case we need to create an .Image object.
    if ( result == TheNilObj && hOldImage != NULL )
    {
        result = rxNewImageFromControl(c, hwnd, hOldImage, type, ctrl);
    }
    return result;
}

RexxObjectPtr oodGetImageAttribute(RexxMethodContext *c, RexxObjectPtr self, CSTRING varName,
                                   UINT msg, WPARAM wParam, uint8_t type, oodControl_t ctrl)
{
    // If we already have an image in the object variable, just use it.
    RexxObjectPtr result = c->GetObjectVariable(varName);
    if ( result == NULLOBJECT )
    {
        HWND hwnd = controlToHCtrl(c, self);
        HANDLE hImage = (HANDLE)SendMessage(hwnd, msg, wParam, 0);

        if ( hImage == NULL )
        {
            result = TheNilObj;
        }
        else
        {
            // Create a new .Image object from the image handle.
            result = rxNewImageFromControl(c, hwnd, hImage, type, ctrl);
        }

        // Set the result in the object variable.  If there is a next time we
        // can retrieve it easily.
        c->SetObjectVariable(varName, result);
    }
    return result;
}



CSTRING getImageTypeName(uint8_t type)
{
    switch ( type )
    {
        case IMAGE_ICON :
            return "Icon";
        case IMAGE_BITMAP :
            return "Bitmap";
        case IMAGE_CURSOR :
            return "Cursor";
        case IMAGE_ENHMETAFILE :
            return "Enhanced Metafile";
        default :
            return "Unknown";
    }
}

POODIMAGE rxGetOodImage(RexxMethodContext *context, RexxObjectPtr o, size_t argPos)
{
    if ( requiredClass(context->threadContext, o, "Image", argPos) )
    {
        POODIMAGE oi = (POODIMAGE)context->ObjectToCSelf(o);
        if ( oi->isValid )
        {
            return oi;
        }
        nullObjectException(context->threadContext, IMAGE_CLASS, argPos);
    }
    return NULL;
}

/**
 * Extracts a valid oodImage pointer from a RexxObjectPtr, ensuring that the
 * image is either an icon or a cursor.  (Cursors are icons.)
 *
 * @param c    The method context we are executing in.
 * @param o    The, assumed, .Image object.
 * @param pos  The argument position in the invocation from ooRexx.  Used for
 *             exception messages.
 *
 * @return A pointer to an OODIMAGE struct on success, othewise NULL.
 */
POODIMAGE rxGetImageIcon(RexxMethodContext *c, RexxObjectPtr o, size_t pos)
{
    POODIMAGE oi = rxGetOodImage(c, o, pos);
    if ( oi != NULL )
    {
        if ( oi->type == IMAGE_ICON || oi->type == IMAGE_CURSOR )
        {
            return oi;
        }
        wrongArgValueException(c->threadContext, pos, "Icon, Cursor", oi->typeName);
    }
    return NULL;
}

/**
 * Extracts a valid oodImage pointer from a RexxObjectPtr, ensuring that the
 * image is a cursor.  (Cursors are icons, but this function strictly enforces
 * that the image is a cursor.)
 *
 * @param c    The method context we are executing in.
 * @param o    The, assumed, .Image object.
 * @param pos  The argument position in the invocation from ooRexx.  Used for
 *             exception messages.
 *
 * @return A pointer to an OODIMAGE struct on success, othewise NULL.
 */
POODIMAGE rxGetImageCursor(RexxMethodContext *c, RexxObjectPtr o, size_t pos)
{
    POODIMAGE oi = rxGetOodImage(c, o, pos);
    if ( oi != NULL )
    {
        if ( oi->type == IMAGE_CURSOR )
        {
            return oi;
        }
        wrongArgValueException(c->threadContext, pos, "Cursor", oi->typeName);
    }
    return NULL;
}

POODIMAGE rxGetImageBitmap(RexxMethodContext *c, RexxObjectPtr o, size_t pos)
{
    POODIMAGE oi = rxGetOodImage(c, o, pos);
    if ( oi != NULL )
    {
        if ( oi->type == IMAGE_BITMAP )
        {
            return oi;
        }
        invalidImageException(c->threadContext, pos, "Bitmap", oi->typeName);
    }
    return NULL;
}

RexxObjectPtr rxNewEmptyImage(RexxMethodContext *c, DWORD rc)
{
    RexxBufferObject bufferObj = c->NewBuffer(sizeof(OODIMAGE));
    POODIMAGE cself = (POODIMAGE)c->BufferData(bufferObj);

    // Set everything to invalid.
    memset(cself, 0, sizeof(OODIMAGE));
    cself->type = -1;
    cself->size.cx = -1;
    cself->size.cy = -1;
    cself->lastError = rc;

    return rxNewImageObject(c, bufferObj);
}

/**
 * Removes and releases all .Image objects in an .Array object.  This is an
 * internal method assuming the args are correct.  The array must contain *only*
 * .Image objects, but can be a sparse array.
 *
 *
 * @param c
 * @param a
 * @param last
 */
void rxReleaseAllImages(RexxMethodContext *c, RexxArrayObject a, size_t last)
{
    for ( size_t i = 1; i <= last; i++)
    {
        RexxObjectPtr image = c->ArrayAt(a, i);
        if ( image != NULLOBJECT )
        {
            c->SendMessage0(image, "RELEASE");
        }
    }
}

RexxArrayObject rxImagesFromArrayOfIDs(RexxMethodContext *c, RexxArrayObject ids, HINSTANCE hModule,
                                        uint8_t type, PSIZE s, uint32_t flags)
{
    size_t i = 0;
    size_t count = c->ArraySize(ids);
    RexxArrayObject result = c->NewArray(count);

    for ( i = 1; i <= count; i++ )
    {
        RexxObjectPtr id = c->ArrayAt(ids, i);
        if ( id == NULLOBJECT )
        {
            sparseArrayException(c->threadContext, 1, i);
            goto err_out;
        }

        int32_t nID = oodGlobalID(c->threadContext, id, 1, true);
        if ( nID == OOD_ID_EXCEPTION )
        {
            // We want to use our own exception, to be more clear.
            c->ClearCondition();
            wrongObjInArrayException(c->threadContext, 1, i, "valid resource ID", id);
            goto err_out;
        }

        HANDLE hImage = LoadImage(hModule, MAKEINTRESOURCE(nID), type, s->cx, s->cy, flags);
        if ( hImage == NULL )
        {
            // Set the system error code and leave this slot in the array blank.
            oodSetSysErrCode(c->threadContext);
        }
        else
        {
            // Theoretically, image could come back null, but this seems very
            // unlikely.  Still, we'll check for it.  If it is null, an
            // exception has already been raised.
            RexxObjectPtr image = rxNewValidImage(c, hImage, type, s, flags, true);
            if ( image == NULLOBJECT )
            {
                goto err_out;
            }
            c->ArrayPut(result, image, i);
        }
    }
    goto out;

err_out:
    // Shared images should not be released.
    if ( (flags & LR_SHARED) == 0 )
    {
        rxReleaseAllImages(c, result, i - 1);
    }
    result = NULLOBJECT;

out:
    return result;
}

RexxMethod1(uint32_t, image_toID_cls, CSTRING, symbol)
{
    return mapSymbol(context, symbol, 1, true);
}


/** Image::getImage()  [class method]
 *
 *  Instantiate an .Image object from one of the system images, or loaded from
 *  an image file (.bmp, .ico, etc..)
 *
 *  @param   id  Either the numeric resource id of a system image, a system
 *               image keyword, or the file name of a stand-alone image file.
 *
 *               The programmer should use one of the .OEM constants to load a
 *               system image, or the raw number if she knows it.  If id is not
 *               a number, or a system image keyword, it is assumed to be a file
 *               name.
 *
 *               Note that many of the .OEM constants have the same numeric
 *               value. The type argument distinguishes whether a bitmap or an
 *               icon is loaded
 *
 *  @note  This method is designed to always return an .Image object, or raise
 *         an exception.  The user would need to test the returned .Image object
 *         for null to be sure it is good.  I.e.:
 *
 *        image = .Image~getImage(...)
 *        if image~isNull then do
 *          -- error
 *        end
 */
RexxMethod4(RexxObjectPtr, image_getImage_cls, RexxObjectPtr, id, OPTIONAL_RexxObjectPtr, _type,
            OPTIONAL_RexxObjectPtr, size, OPTIONAL_RexxObjectPtr, _flags)
{
    RexxObjectPtr result = NULLOBJECT;
    SIZE s = {0};

    bool fromFile = true;
    LPCTSTR name = NULL;

    int32_t resourceID = getSystemImageID(context, id, 1);
    if ( resourceID != OOD_NO_VALUE )
    {
        name = MAKEINTRESOURCE(resourceID);
        fromFile = false;
    }

    if ( fromFile )
    {
        if ( ! context->IsString(id) )
        {
            wrongArgValueException(context->threadContext, 1, "either an image file name, or an OEM image ID", id);
            goto out;
        }
        name = context->ObjectToStringValue(id);
    }

    uint32_t type = getImageTypeArg(context, _type, IMAGE_BITMAP, 2);
    if ( type == OOD_NO_VALUE )
    {
        goto out;
    }

    if ( ! getImageSizeArg(context, size, &s, 3) )
    {
        goto out;
    }

    uint32_t flags = getImageFlagsArg(context, _flags, fromFile ? LR_LOADFROMFILE : LR_SHARED | LR_DEFAULTSIZE, 4);
    if ( flags == OOD_NO_VALUE )
    {
        goto out;
    }

    HANDLE hImage = LoadImage(NULL, name, type, s.cx, s.cy, flags);
    if ( hImage == NULL )
    {
        DWORD rc = GetLastError();
        oodSetSysErrCode(context->threadContext, rc);
        result = rxNewEmptyImage(context, rc);
        goto out;
    }

    result = rxNewValidImage(context, hImage, type, &s, flags, true);

out:
    return result;
}

#if 0
// Place holder, future enhancement.
RexxMethod1(RexxObjectPtr, image_getGDIImage_cls, CSTRING, file)
{
    RexxObjectPtr result = NULLOBJECT;

    WCHAR *fileName = ansi2unicode(file);

    Bitmap bitmap(fileName, FALSE);

    Gdiplus::Color colorBackground = Gdiplus::Color(255, 255, 255);

    HBITMAP hImage = NULL;
    bitmap.GetHBITMAP(colorBackground, &hImage);
    if ( hImage == NULL )
    {
        DWORD rc = GetLastError();
        oodSetSysErrCode(context->threadContext, rc);
        result = rxNewEmptyImage(context, rc);
        goto out;
    }

    SIZE s;
    s.cx = bitmap.GetHeight();
    s.cy = bitmap.GetWidth();

    result = rxNewValidImage(context, hImage, IMAGE_BITMAP, &s, 0, true);

out:
    return result;
}
#endif

/** Image::userIcon()  [class method]
 *
 *  Retrieves a user icon.  A user icon is added through
 *  DynamicDialog::addIconResource(), either by the programmer, or when a *.rc
 *  script file is parsed, if the file has icon resources.
 *
 *  @param  dlg    The UserDialog object that had the icon added.
 *  @param  rxID   The resource ID of the icon, numeric or symbolic
 *  @param  size   [optional]  A size object containing the desired size for the
 *                 loaded icon.  The default if omitted is 0 x 0, which tells
 *                 the OS to load the icon ...
 *  @param  flags  [optional]
 *
 *  @return An image object, which may be a null image on error.
 */
RexxMethod4(RexxObjectPtr, image_userIcon_cls, RexxObjectPtr, dlg, RexxObjectPtr, rxID,
            OPTIONAL_RexxObjectPtr, size, OPTIONAL_RexxObjectPtr, _flags)
{
    RexxObjectPtr result = NULLOBJECT;
    SIZE s = {0};

    if ( ! requiredClass(context->threadContext, dlg, "PlainBaseDialog", 1) )
    {
        goto out;
    }

    int32_t id = oodResolveSymbolicID(context, dlg, rxID, -1, 2, true);
    if ( id == OOD_ID_EXCEPTION )
    {
        goto out;
    }

    pCPlainBaseDialog pcpbd = dlgToCSelf(context, dlg);

    const char *fileName = NULL;
    for ( size_t i = 0; i < pcpbd->IT_nextIndex; i++ )
    {
        if ( pcpbd->IconTab[i].iconID == id )
        {
            fileName = pcpbd->IconTab[i].fileName;
            break;
        }
    }
    if ( fileName == NULL )
    {
        invalidTypeException(context->threadContext, 2, "resource ID for a user icon");
        goto out;
    }

    if ( argumentExists(3) )
    {
        SIZE *p = (PSIZE)rxGetSize(context, size, 3);
        if ( p == NULL )
        {
            goto out;
        }
        s.cx = p->cx;
        s.cy = p->cy;
    }

    uint32_t defFlags = getImageFlagsArg(context, _flags, LR_LOADFROMFILE, 4);
    if ( defFlags == OOD_NO_VALUE )
    {
        goto out;
    }

    HANDLE hImage = LoadImage(NULL, fileName, IMAGE_ICON, s.cx, s.cy, defFlags);
    if ( hImage == NULL )
    {
        DWORD rc = GetLastError();
        oodSetSysErrCode(context->threadContext, rc);
        result = rxNewEmptyImage(context, rc);
        goto out;
    }

    result = rxNewValidImage(context, hImage, IMAGE_ICON, &s, defFlags, true);

out:
    return result;
}

RexxMethod4(RexxObjectPtr, image_fromFiles_cls, RexxArrayObject, files, OPTIONAL_RexxObjectPtr, _type,
            OPTIONAL_RexxObjectPtr, size, OPTIONAL_RexxObjectPtr, _flags)
{
    RexxMethodContext *c = context;
    RexxArrayObject result = NULLOBJECT;
    SIZE s = {0};

    uint32_t type = getImageTypeArg(context, _type, IMAGE_BITMAP, 2);
    if ( type == OOD_NO_VALUE )
    {
        goto out;
    }

    if ( ! getImageSizeArg(context, size, &s, 3) )
    {
        goto out;
    }

    uint32_t flags = getImageFlagsArg(context, _flags, LR_LOADFROMFILE, 4);
    if ( flags == OOD_NO_VALUE )
    {
        goto out;
    }

    size_t count = c->ArraySize(files);
    result = c->NewArray(count);

    for ( size_t i = 1; i <= count; i++ )
    {
        RexxObjectPtr f = c->ArrayAt(files, i);
        if ( f == NULLOBJECT || ! context->IsString(f) )
        {
            rxReleaseAllImages(c, result, i - 1);
            c->RaiseException1(Rexx_Error_Incorrect_method_nostring_inarray , c->WholeNumber(1));
            result = NULLOBJECT;
            goto out;
        }

        const char *file = context->ObjectToStringValue(f);
        HANDLE hImage = LoadImage(NULL, file, type, s.cx, s.cy, flags);
        if ( hImage == NULL )
        {
            // Set the system error code and leave this slot in the array blank.
            oodSetSysErrCode(context->threadContext);
        }
        else
        {
            // Theoretically, image could come back null, but this seems very
            // unlikely.  Still, we'll check for it.  If it is null, an
            // exception has already been raised.
            RexxObjectPtr image = rxNewValidImage(context, hImage, type, &s, flags, true);
            if ( image == NULLOBJECT )
            {
                rxReleaseAllImages(c, result, i - 1);
                result = NULLOBJECT;
                goto out;
            }
            c->ArrayPut(result, image, i);
        }
    }

out:
    return result;
}

/** Image::fromIDs()   [class]
 *
 *  Return an array of .Image objects using an array of resource IDs.  Need to
 *  doc that the IDs can be numeric or symbolic.
 *
 */
RexxMethod4(RexxObjectPtr, image_fromIDs_cls, RexxArrayObject, ids, OPTIONAL_RexxObjectPtr, _type,
            OPTIONAL_RexxObjectPtr, size, OPTIONAL_RexxObjectPtr, _flags)
{
    RexxMethodContext *c = context;
    RexxArrayObject result = NULLOBJECT;
    SIZE s = {0};

    uint32_t type = getImageTypeArg(context, _type, IMAGE_ICON, 2);
    if ( type == OOD_NO_VALUE )
    {
        goto out;
    }

    if ( ! getImageSizeArg(context, size, &s, 3) )
    {
        goto out;
    }

    uint32_t flags = getImageFlagsArg(context, _flags, LR_SHARED | LR_DEFAULTSIZE, 4);
    if ( flags == OOD_NO_VALUE )
    {
        goto out;
    }

    result = rxImagesFromArrayOfIDs(context, ids, NULL, type, &s, flags);

out:
    return result;
}

/** Image::colorRef()  [class]
 *
 *  Returns a COLORREF composed from the specified RGB valuses.
 *
 *  @param r  The red component, or special case CLR_DEFAULT / CLR_NONE /
 *            CLR_INVALID.
 *  @param g  The green component
 *  @param b  The blue component
 *
 *  @return The COLORREF.
 *
 *  @note  The first argument can also be the string keyword for one of the
 *         special COLORREF values.  I.e., CLR_DEFAULT, CLR_NONE, or
 *         CLR_INVALID.  Case is insignificant for the keyword.
 *
 *  @remarks  For any omitted arg, the value of the arg will be 0.  Since 0 is
 *            the default value for the g and b args, we do not need to check
 *            for ommitted args.
 */
RexxMethod3(uint32_t, image_colorRef_cls, OPTIONAL_RexxObjectPtr, rVal,
            OPTIONAL_uint8_t, g, OPTIONAL_uint8_t, b)
{
    uint8_t r = 0;
    if ( argumentExists(1) )
    {
        CSTRING tmp = context->ObjectToStringValue(rVal);
        if ( *tmp && toupper(*tmp) == 'C' )
        {
            if ( stricmp(tmp, "CLR_DEFAULT") == 0 )
            {
                return CLR_DEFAULT;
            }
            else if ( stricmp(tmp, "CLR_NONE") == 0 )
            {
                return CLR_NONE;
            }
            else if ( stricmp(tmp, "CLR_INVALID") == 0 )
            {
                return CLR_INVALID;
            }
            else
            {
                goto error_out;
            }
        }

        uint32_t tmpR;
        if ( ! context->ObjectToUnsignedInt32(rVal, &tmpR) || tmpR > 255 )
        {
            goto error_out;
        }
        r = (uint8_t)tmpR;
    }
    return RGB(r, g, b);

error_out:
    wrongArgValueException(context->threadContext, 1,
                           "CLR_DEFAULT, CLR_NONE, CLR_INVALID, or a number from 0 through 255", rVal);
    return 0;
}

RexxMethod1(uint8_t, image_getRValue_cls, uint32_t, colorRef) { return GetRValue(colorRef); }
RexxMethod1(uint8_t, image_getGValue_cls, uint32_t, colorRef) { return GetGValue(colorRef); }
RexxMethod1(uint8_t, image_getBValue_cls, uint32_t, colorRef) { return GetBValue(colorRef); }


/** Image::init()
 *
 *
 */
RexxMethod1(RexxObjectPtr, image_init, RexxObjectPtr, cselfObj)
{
    if ( requiredClass(context->threadContext, cselfObj, "Buffer", 1) )
    {
        context->SetObjectVariable("CSELF", cselfObj);
    }
    return NULLOBJECT;
}

RexxMethod1(uint32_t, image_release, CSELF, oi)
{
    uint32_t rc = 0;
    POODIMAGE pOI = (POODIMAGE)oi;

    if ( pOI->canRelease )
    {
        switch ( pOI->type )
        {
            case IMAGE_ICON :
                if ( DestroyIcon((HICON)pOI->hImage) == 0 )
                {
                    rc = GetLastError();
                }
                break;

            case IMAGE_BITMAP :
                if ( DeleteObject((HGDIOBJ)pOI->hImage) == 0 )
                {
                    rc = GetLastError();
                }
                break;

            case IMAGE_CURSOR :
                if ( DestroyCursor((HCURSOR)pOI->hImage) == 0 )
                {
                    rc = GetLastError();
                }
                break;

            case IMAGE_ENHMETAFILE :
                // Currently no way in ooDialog to have this type of image.
                // Left for future enhancement.
                break;

            default :
                // Should be impossible.
                break;
        }
    }

    pOI->hImage = NULL;
    pOI->type = -1;
    pOI->size.cx = -1;
    pOI->size.cy = -1;
    pOI->flags = 0;
    pOI->isValid = false;
    pOI->srcOOD = false;
    pOI->canRelease = false;
    pOI->lastError = rc;

    oodSetSysErrCode(context->threadContext, pOI->lastError);

    return rc;
}

RexxMethod1(POINTER, image_handle, CSELF, oi)
{
    if ( ! ((POODIMAGE)oi)->isValid )
    {
        nullObjectException(context->threadContext, IMAGE_CLASS);
    }
    return ((POODIMAGE)oi)->hImage;
}

RexxMethod1(logical_t, image_isNull, CSELF, oi) { return ( ! ((POODIMAGE)oi)->isValid ); }
RexxMethod1(uint32_t, image_systemErrorCode, CSELF, oi) { return ((POODIMAGE)oi)->lastError; }


/**
 * Methods for the ooDialog .ResourceImage class.
 */
#define RESOURCE_IMAGE_CLASS  "ResourceImage"


PRESOURCEIMAGE rxGetResourceImage(RexxMethodContext *context, RexxObjectPtr r, size_t argPos)
{
    if ( requiredClass(context->threadContext, r, "ResourceImage", argPos) )
    {
        return (PRESOURCEIMAGE)context->ObjectToCSelf(r);
    }
    return NULL;
}


/** ResouceImage::init()
 *
 *  Instantiates a new ResourceImage from an executable module.
 *
 *  @param fileOrDlg  [required]  Either a dialog object, or a file name of an
 *                    executable module.  (DLLs are executable modules.)
 *
 *                    When the arg is a dialog object:  If the dialog object is
 *                    a ResDialog, the DLL used to instantiate the dialog is
 *                    used. Otherwise, ooDialog.dll is used.  ooDialog.dll has
 *                    some common resources bound to it that are always
 *                    available for use.
 *
 *                    When the arg is not a dialog object, it must be the string
 *                    file name of an executable file.
 *
 *  @param dlg        [optional] A dialog object.  This usage is deprecated, but
 *                    is maintained for backwards compatibility.  Do not use
 *                    this in new code.
 *
 *  @note  When this method was first introduced, the first arg had to be the
 *         string file name of an excutable file.  Arg 2 was an optional dialog
 *         object and could be used to optionally instantiate the resource image
 *         from an already loaded DLL as outlined above for the fileOrDlg
 *         argument.  This was silly, it forced the programmer to use a string
 *         for the first arg, when only the dialog object was needed.  This
 *         oversight is now fixed.
 *
 *  @remarks  Originally arg 1 was a CSTRING and named a file.  Arg 2 was an
 *            optional dialog object.  If arg 2 existed, then we got the HMODULE
 *            from the already loaded instance - either ooDialog.dll or the
 *            hInstance if a ResDialog.
 *
 *            That was done when the new native API was new, and silly. Now we
 *            just check the first arg for a dialog object or a string and do
 *            the right thing.  But, we still need to preserve the old for
 *            compatibility.
 */
RexxMethod2(RexxObjectPtr, ri_init, RexxObjectPtr, fileOrDlg, OPTIONAL_RexxObjectPtr, dlg)
{
    oodResetSysErrCode(context->threadContext);

    RexxBufferObject cself = context->NewBuffer(sizeof(RESOURCEIMAGE));
    context->SetObjectVariable("CSELF", cself);

    PRESOURCEIMAGE ri = (PRESOURCEIMAGE)context->BufferData(cself);
    memset(ri, 0, sizeof(RESOURCEIMAGE));

    RexxMethodContext *c = context;
    if ( c->IsOfType(fileOrDlg, "ResDialog") )
    {
        pCPlainBaseDialog pcpbd = dlgToCSelf(context, fileOrDlg);
        ri->hMod = pcpbd->hInstance;
        ri->isValid = true;
    }
    else if ( c->IsOfType(fileOrDlg, "PlainBaseDialog") )
    {
        ri->hMod = MyInstance;
        ri->isValid = true;
    }
    else
    {
        CSTRING file = c->ObjectToStringValue(fileOrDlg);

        if ( argumentOmitted(2) )
        {
            ri->hMod = LoadLibraryEx(file, NULL, LOAD_LIBRARY_AS_DATAFILE);
            if ( ri->hMod == NULL )
            {
                ri->lastError = GetLastError();
                oodSetSysErrCode(context->threadContext, ri->lastError);
            }
            else
            {
                ri->canRelease = true;
                ri->isValid = true;
            }
        }
        else
        {
            if ( ! requiredClass(context->threadContext, dlg, "PlainBaseDialog", 2) )
            {
                goto err_out;
            }

            if ( stricmp(OODDLL, file) == 0 )
            {
                ri->hMod = MyInstance;
                ri->isValid = true;
            }
            else
            {
                if ( ! requiredClass(context->threadContext, dlg, "ResDialog", 2) )
                {
                    goto err_out;
                }

                pCPlainBaseDialog pcpbd = dlgToCSelf(context, dlg);
                ri->hMod = pcpbd->hInstance;
                ri->isValid = true;
            }
        }
    }


    return NULLOBJECT;

err_out:

    // 1812 ERROR_RESOURCE_DATA_NOT_FOUND
    // The specified image file did not contain a resource section.

    ri->lastError = 1812;
    oodSetSysErrCode(context->threadContext, ri->lastError);
    return NULLOBJECT;
}

/** ResourceImage::getImage()
 *
 * Loads an image resource from this resource binary.
 *
 * @param   id     Resource ID of the image in the resource binary, may be
 *                 symbolic or numeric.  If symbolic, the programmer must use
 *                 the global .constDir.
 *
 * @param   type   Image type, IMAGE_BITMAP is the default if omitted.
 *
 * @param   size
 * @param   flags
 *
 * @return  An instantiated .Image object, which may be a null Image if an error
 *          occurred.
 *
 * @note  This method is designed to always return an .Image object, or raise an
 *        exception.  The user would need to test the returned .Image object for
 *        null to be sure it is good.  I.e.:
 *
 *        mod = .ResourceImage~new(...)
 *        ...
 *        image = mod~getImage(...)
 *        if image~isNull then do
 *          -- error
 *        end
 */
RexxMethod5(RexxObjectPtr, ri_getImage, RexxObjectPtr, _id, OPTIONAL_RexxObjectPtr, _type,
            OPTIONAL_RexxObjectPtr, size, OPTIONAL_RexxObjectPtr, _flags, CSELF, cself)
{
    RexxObjectPtr result = NULLOBJECT;
    SIZE s = {0};

    int32_t id = oodGlobalID(context->threadContext, _id, 1, true);
    if ( id == OOD_ID_EXCEPTION )
    {
        goto out;
    }

    PRESOURCEIMAGE ri = (PRESOURCEIMAGE)cself;
    if ( ! ri->isValid )
    {
        nullObjectException(context->threadContext, RESOURCE_IMAGE_CLASS);
        goto out;
    }
    ri->lastError = 0;

    uint32_t type = getImageTypeArg(context, _type, IMAGE_BITMAP, 2);
    if ( type == OOD_NO_VALUE )
    {
        goto out;
    }

    if ( ! getImageSizeArg(context, size, &s, 3) )
    {
        goto out;
    }

    uint32_t flags = getImageFlagsArg(context, _flags, LR_SHARED, 4);
    if ( flags == OOD_NO_VALUE )
    {
        goto out;
    }

    HANDLE hImage = LoadImage(ri->hMod, MAKEINTRESOURCE(id), type, s.cx, s.cy, flags);
    if ( hImage == NULL )
    {
        ri->lastError = GetLastError();
        oodSetSysErrCode(context->threadContext, ri->lastError);
        result = rxNewEmptyImage(context, ri->lastError);
        goto out;
    }

    result = rxNewValidImage(context, hImage, type, &s, flags, true);

out:
    return result;
}

RexxMethod5(RexxObjectPtr, ri_getImages, RexxArrayObject, ids, OPTIONAL_RexxObjectPtr, _type,
            OPTIONAL_RexxObjectPtr, size, OPTIONAL_RexxObjectPtr, _flags, CSELF, cself)
{
    RexxObjectPtr result = NULLOBJECT;
    SIZE s = {0};

    PRESOURCEIMAGE ri = (PRESOURCEIMAGE)cself;
    if ( ! ri->isValid )
    {
        nullObjectException(context->threadContext, RESOURCE_IMAGE_CLASS);
        goto out;
    }
    ri->lastError = 0;

    uint32_t type = getImageTypeArg(context, _type, IMAGE_BITMAP, 2);
    if ( type == OOD_NO_VALUE )
    {
        goto out;
    }

    if ( ! getImageSizeArg(context, size, &s, 3) )
    {
        goto out;
    }

    uint32_t flags = getImageFlagsArg(context, _flags, LR_SHARED, 4);
    if ( flags == OOD_NO_VALUE )
    {
        goto out;
    }

    result = rxImagesFromArrayOfIDs(context, ids, ri->hMod, type, &s, flags);
    if ( result == NULLOBJECT )
    {
        ri->lastError = oodGetSysErrCode(context->threadContext);
    }

out:
    return result;
}

RexxMethod1(uint32_t, ri_release, CSELF, r)
{
    uint32_t rc = 0;
    PRESOURCEIMAGE ri = (PRESOURCEIMAGE)r;

    if ( ri->canRelease )
    {
        if ( ! FreeLibrary((HMODULE)ri->hMod) )
        {
            rc = GetLastError();
        }
    }

    ri->canRelease = false;
    ri->isValid = false;
    ri->hMod = NULL;
    ri->lastError = rc;
    oodSetSysErrCode(context->threadContext, ri->lastError);

    return rc;
}

RexxMethod1(POINTER, ri_handle, CSELF, ri)
{
    if ( ! ((PRESOURCEIMAGE)ri)->isValid )
    {
        nullObjectException(context->threadContext, RESOURCE_IMAGE_CLASS);
    }
    return ((PRESOURCEIMAGE)ri)->hMod;
}

RexxMethod1(logical_t, ri_isNull, CSELF, ri) { return ( ! ((PRESOURCEIMAGE)ri)->isValid); }
RexxMethod1(uint32_t, ri_systemErrorCode, CSELF, ri) { return ((PRESOURCEIMAGE)ri)->lastError; }


