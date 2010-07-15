/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
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

/**
 * oodResources.cpp
 *
 * Contains the classes used for objects representing Windows resources and
 * "resource-like" things.  .Image, .ResourceImage, .ImageList, etc..
 */
#include "ooDialog.hpp"     // Must be first, includes windows.h and oorexxapi.h

#include <commctrl.h>
#include "APICommon.hpp"
#include "oodCommon.hpp"
#include "oodControl.hpp"
#include "oodDeviceGraphics.hpp"
#include "oodResources.hpp"

/**
 * Defines, structs, etc., for the .ImageList class.
 */

#define IMAGELISTCLASS             ".ImageList"


// ImageList helper functions.
HIMAGELIST rxGetImageList(RexxMethodContext *, RexxObjectPtr, int);
RexxObjectPtr rxNewImageList(RexxMethodContext *, HIMAGELIST);

#define IL_DEFAULT_FLAGS           ILC_COLOR32 | ILC_MASK
#define IL_DEFAULT_COUNT           6
#define IL_DEFAULT_GROW            0


/**
 * Defines, structs, etc., for the .Image class.
 */

#define IMAGECLASS                 ".Image"


// Helper functions.
CSTRING getImageTypeName(uint8_t);
RexxObjectPtr rxNewImageFromControl(RexxMethodContext *, HWND, HANDLE, uint8_t, oodControl_t);
RexxObjectPtr rxNewEmptyImage(RexxMethodContext *, DWORD);
RexxObjectPtr rxNewValidImage(RexxMethodContext *, HANDLE, uint8_t, PSIZE, uint32_t, bool);

POODIMAGE rxGetImageBitmap(RexxMethodContext *, RexxObjectPtr, int);

RexxObjectPtr oodILFromBMP(RexxMethodContext *, HIMAGELIST *, RexxObjectPtr, int, int, HWND);


/**
 * Defines and structs for the .ResourceImage class.
 */
#define RESOURCEIMAGECLASS  ".ResourceImage"

typedef struct _RESOURCEIMAGE
{
    HMODULE  hMod;
    DWORD    lastError;
    bool     canRelease;
    bool     isValid;
} RESOURCEIMAGE, *PRESOURCEIMAGE;


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


/**
 * Methods for the .ImageList class.
 */
#define IMAGELIST_CLASS "ImageList"


HIMAGELIST rxGetImageList(RexxMethodContext *context, RexxObjectPtr il, int argPos)
{
    HIMAGELIST himl = NULL;
    if ( requiredClass(context->threadContext, il, "ImageList", argPos) )
    {
        // Make sure we don't use a null ImageList.
        himl = (HIMAGELIST)context->ObjectToCSelf(il);
        if ( himl == NULL )
        {
            nullObjectException(context->threadContext, IMAGELISTCLASS, argPos);
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
        invalidTypeException(context->threadContext, 1, " ImageList handle");
        goto out;
    }
    context->SetObjectVariable("CSELF", context->NewPointer(himl));

out:
    return NULLOBJECT;
}

RexxMethod4(RexxObjectPtr, il_create_cls, OPTIONAL_RexxObjectPtr, size,  OPTIONAL_uint32_t, flags,
            OPTIONAL_int32_t, count, OPTIONAL_int32_t, grow)
{
    RexxMethodContext *c = context;
    RexxObjectPtr result = TheNilObj;

    SIZE s = {0};
    if ( argumentExists(1) )
    {
        SIZE *p = rxGetSize(c, size, 3);
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

    if ( argumentExists(2) )
    {
        if ( (flags & (ILC_MIRROR | ILC_PERITEMMIRROR)) && (! requiredComCtl32Version(c, "init", COMCTL32_6_0)) )
        {
            goto out;
        }
    }
    else
    {
        flags = IL_DEFAULT_FLAGS;
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

RexxMethod3(int, il_add, RexxObjectPtr, image, OPTIONAL_RexxObjectPtr, optMask, CSELF, il)
{
    RexxMethodContext *c = context;
    int result = -1;

    HIMAGELIST himl = (HIMAGELIST)il;
    if ( himl == NULL )
    {
        nullObjectException(c->threadContext, IMAGELISTCLASS);
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

RexxMethod3(int, il_addMasked, RexxObjectPtr, image, uint32_t, cRef, CSELF, il)
{
    int result = -1;

    HIMAGELIST himl = (HIMAGELIST)il;
    if ( himl == NULL )
    {
        nullObjectException(context->threadContext, IMAGELISTCLASS);
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

RexxMethod2(int, il_addIcon, RexxObjectPtr, image, CSELF, il)
{
    int result = -1;

    HIMAGELIST himl = (HIMAGELIST)il;
    if ( himl == NULL )
    {
        nullObjectException(context->threadContext, IMAGELISTCLASS);
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

RexxMethod3(int, il_addImages, RexxArrayObject, images, OPTIONAL_uint32_t, cRef, CSELF, il)
{
    RexxMethodContext *c = context;
    int result = -1;
    int tmpResult = -1;

    HIMAGELIST himl = (HIMAGELIST)il;
    if ( himl == NULL )
    {
        nullObjectException(context->threadContext, IMAGELISTCLASS);
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
    nullObjectException(context->threadContext, IMAGELISTCLASS);
    return NULL;
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
    nullObjectException(context->threadContext, IMAGELISTCLASS);
    return NULL;
}

RexxMethod1(RexxObjectPtr, il_duplicate, CSELF, il)
{
    HIMAGELIST himl = (HIMAGELIST)il;
    if ( himl != NULL )
    {
        return rxNewImageList(context, ImageList_Duplicate(himl));
    }
    nullObjectException(context->threadContext, IMAGELISTCLASS);
    return NULL;
}

RexxMethod2(logical_t, il_remove, int, index, CSELF, il)
{
    HIMAGELIST himl = (HIMAGELIST)il;
    if ( himl != NULL )
    {
        return ImageList_Remove(himl, index);
    }
    nullObjectException(context->threadContext, IMAGELISTCLASS);
    return NULL;
}

RexxMethod1(logical_t, il_removeAll, CSELF, il)
{
    HIMAGELIST himl = (HIMAGELIST)il;
    if ( himl != NULL )
    {
        return ImageList_RemoveAll(himl);
    }
    nullObjectException(context->threadContext, IMAGELISTCLASS);
    return NULL;
}

RexxMethod1(uint32_t, il_release, CSELF, il)
{
    if ( il != NULL )
    {
        ImageList_Destroy((HIMAGELIST)il);
        context->SetObjectVariable("CSELF", context->NewPointer(NULL));
    }
    return 0;
}

RexxMethod1(POINTER, il_handle, CSELF, il)
{
    if ( il == NULL )
    {
        nullObjectException(context->threadContext, IMAGELISTCLASS);
    }
    return il;
}

RexxMethod1(logical_t, il_isNull, CSELF, il) { return ( il == NULL);  }


/**
 * Methods for the .Image class.
 */
#define IMAGE_CLASS "Image"

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

POODIMAGE rxGetOodImage(RexxMethodContext *context, RexxObjectPtr o, int argPos)
{
    if ( requiredClass(context->threadContext, o, "Image", argPos) )
    {
        POODIMAGE oi = (POODIMAGE)context->ObjectToCSelf(o);
        if ( oi->isValid )
        {
            return oi;
        }
        nullObjectException(context->threadContext, IMAGECLASS, argPos);
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
POODIMAGE rxGetImageIcon(RexxMethodContext *c, RexxObjectPtr o, int pos)
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

POODIMAGE rxGetImageBitmap(RexxMethodContext *c, RexxObjectPtr o, int pos)
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

RexxArrayObject rxImagesFromArrayOfInts(RexxMethodContext *c, RexxArrayObject ids, HINSTANCE hModule,
                                        uint8_t type, PSIZE s, uint32_t flags)
{
    int resourceID;
    size_t count = c->ArraySize(ids);
    RexxArrayObject result = c->NewArray(count);

    for ( size_t i = 1; i <= count; i++ )
    {
        RexxObjectPtr id = c->ArrayAt(ids, i);
        if ( id == NULLOBJECT || ! c->Int32(id, &resourceID) )
        {
            // Shared images should not be released.
            if ( (flags & LR_SHARED) == 0 )
            {
                rxReleaseAllImages(c, result, i - 1);
            }
            wrongObjInArrayException(c->threadContext, 1, i, "number");
            result = NULLOBJECT;
            goto out;
        }

        HANDLE hImage = LoadImage(hModule, MAKEINTRESOURCE(resourceID), type, s->cx, s->cy, flags);
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
                if ( (flags & LR_SHARED) == 0 )
                {
                    rxReleaseAllImages(c, result, i - 1);
                }
                result = NULLOBJECT;
                goto out;
            }
            c->ArrayPut(result, image, i);
        }
    }
out:
    return result;
}

bool getStandardImageArgs(RexxMethodContext *context, uint8_t *type, uint8_t defType, RexxObjectPtr size,
                          SIZE *defSize, uint32_t *flags, uint32_t defFlags)
{
    oodResetSysErrCode(context->threadContext);

    if ( argumentOmitted(2) )
    {
        *type = defType;
    }

    if ( argumentExists(3) )
    {
        SIZE *p = rxGetSize(context, size, 3);
        if ( p == NULL )
        {
            return false;
        }
        defSize->cx = p->cx;
        defSize->cy = p->cy;
    }

    if ( argumentOmitted(4) )
    {
        *flags = defFlags;
    }
    else
    {
        // The user specified flags.  Use some safeguards, determined by the
        // value of the default flags.  In all other cases, assume the user
        // knows best.

        if ( defFlags == LR_LOADFROMFILE )
        {
            // Ensure the user did not use shared and did use load from file.
            *flags = (*flags &  ~LR_SHARED) | LR_LOADFROMFILE;
        }
        else if ( defFlags == (LR_SHARED | LR_DEFAULTSIZE) )
        {
            // Ensure the user did not use load from file and did use shared.
            *flags = (*flags &  ~LR_LOADFROMFILE) | LR_SHARED;
        }
    }
    return true;
}

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

RexxMethod1(uint32_t, image_toID_cls, CSTRING, symbol)
{
    static String2Int *imageConstantsMap = NULL;

    if ( imageConstantsMap == NULL )
    {
        imageConstantsMap = imageInitMap();
    }
    int idValue = getKeywordValue(imageConstantsMap, symbol);
    if ( idValue == -1 )
    {
        wrongArgValueException(context->threadContext, 1, "the Image class symbol IDs", symbol);
    }
    return (uint32_t)idValue;
}


/** Image::getImage()  [class method]
 *
 *  Load a stand alone image from a file or one of the system images.
 *
 *  @param   id  Either the numeric resource id of a system image, or the file
 *               name of a stand-alone image file.
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
RexxMethod4(RexxObjectPtr, image_getImage_cls, RexxObjectPtr, id, OPTIONAL_uint8_t, type,
            OPTIONAL_RexxObjectPtr, size, OPTIONAL_uint32_t, flags)
{
    RexxObjectPtr result = NULLOBJECT;
    SIZE s = {0};

    bool fromFile = true;
    LPCTSTR name = NULL;

    int resourceID;
    if ( context->Int32(id, &resourceID) )
    {
        name = MAKEINTRESOURCE(resourceID);
        fromFile = false;
    }

    if ( fromFile )
    {
        if ( ! context->IsString(id) )
        {
            wrongArgValueException(context->threadContext, 1, "either an image file name, or a numeric system image ID", id);
            goto out;
        }
        name = context->ObjectToStringValue(id);
    }

    if ( ! getStandardImageArgs(context, &type, IMAGE_BITMAP, size, &s, &flags,
                                fromFile ? LR_LOADFROMFILE : LR_SHARED | LR_DEFAULTSIZE) )
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
            OPTIONAL_RexxObjectPtr, size, OPTIONAL_uint32_t, flags)
{
    RexxObjectPtr result = NULLOBJECT;
    SIZE s = {0};
    uint32_t defFlags = LR_LOADFROMFILE;

    if ( ! requiredClass(context->threadContext, dlg, "PlainBaseDialog", 1) )
    {
        goto out;
    }

    int32_t id = oodResolveSymbolicID(context, dlg, rxID, -1, 2);
    if ( id == OOD_ID_EXCEPTION )
    {
        goto out;
    }

    pCPlainBaseDialog pcpbd = dlgToCSelf(context, dlg);

    const char *fileName = NULL;
    for ( size_t i = 0; i < pcpbd->IT_size; i++ )
    {
        if ( pcpbd->IconTab[i].iconID == id )
        {
            fileName = pcpbd->IconTab[i].fileName;
            break;
        }
    }
    if ( fileName == NULL )
    {
        invalidTypeException(context->threadContext, 2, " resource ID for a user icon");
        goto out;
    }

    if ( argumentExists(3) )
    {
        SIZE *p = rxGetSize(context, size, 3);
        if ( p == NULL )
        {
            goto out;
        }
        s.cx = p->cx;
        s.cy = p->cy;
    }

    if ( argumentExists(4) )
    {
        // Make sure the user has compatible flags for this operation.
        defFlags = (flags &  ~LR_SHARED) | LR_LOADFROMFILE;
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

RexxMethod4(RexxObjectPtr, image_fromFiles_cls, RexxArrayObject, files, OPTIONAL_uint8_t, type,
            OPTIONAL_RexxObjectPtr, size, OPTIONAL_uint32_t, flags)
{
    RexxMethodContext *c = context;
    RexxArrayObject result = NULLOBJECT;
    SIZE s = {0};

    if ( ! getStandardImageArgs(context, &type, IMAGE_BITMAP, size, &s, &flags, LR_LOADFROMFILE) )
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

RexxMethod4(RexxObjectPtr, image_fromIDs_cls, RexxArrayObject, ids, OPTIONAL_uint8_t, type,
            OPTIONAL_RexxObjectPtr, size, OPTIONAL_uint32_t, flags)
{
    RexxMethodContext *c = context;
    RexxArrayObject result = NULLOBJECT;
    SIZE s = {0};

    if ( ! getStandardImageArgs(context, &type, IMAGE_ICON, size, &s, &flags, LR_SHARED | LR_DEFAULTSIZE) )
    {
        goto out;
    }

    result = rxImagesFromArrayOfInts(context, ids, NULL, type, &s, flags);

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
        nullObjectException(context->threadContext, IMAGECLASS);
    }
    return ((POODIMAGE)oi)->hImage;
}

RexxMethod1(logical_t, image_isNull, CSELF, oi) { return ( ! ((POODIMAGE)oi)->isValid ); }
RexxMethod1(uint32_t, image_systemErrorCode, CSELF, oi) { return ((POODIMAGE)oi)->lastError; }


/**
 * Methods for the ooDialog .ResourceImage class.
 */
#define RESOURCE_IMAGE_CLASS  "ResourceImage"


/** ResouceImage::init()
 *
 *
 */
RexxMethod2(RexxObjectPtr, ri_init, CSTRING, file, OPTIONAL_RexxObjectPtr, dlg)
{
    oodResetSysErrCode(context->threadContext);

    RexxBufferObject cself = context->NewBuffer(sizeof(RESOURCEIMAGE));
    context->SetObjectVariable("CSELF", cself);

    PRESOURCEIMAGE ri = (PRESOURCEIMAGE)context->BufferData(cself);
    memset(ri, 0, sizeof(RESOURCEIMAGE));

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
 * Loads an image from this resource binary.
 *
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
RexxMethod5(RexxObjectPtr, ri_getImage, int, id, OPTIONAL_uint8_t, type,
            OPTIONAL_RexxObjectPtr, size, OPTIONAL_uint32_t, flags, CSELF, cself)
{

    RexxObjectPtr result = NULLOBJECT;
    SIZE s = {0};

    PRESOURCEIMAGE ri = (PRESOURCEIMAGE)cself;
    if ( ! ri->isValid )
    {
        nullObjectException(context->threadContext, RESOURCEIMAGECLASS);
        goto out;
    }
    ri->lastError = 0;

    if ( ! getStandardImageArgs(context, &type, IMAGE_BITMAP, size, &s, &flags, LR_SHARED) )
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

RexxMethod5(RexxObjectPtr, ri_getImages, RexxArrayObject, ids, OPTIONAL_uint8_t, type,
            OPTIONAL_RexxObjectPtr, size, OPTIONAL_uint32_t, flags, CSELF, cself)
{
    RexxObjectPtr result = NULLOBJECT;
    SIZE s = {0};

    PRESOURCEIMAGE ri = (PRESOURCEIMAGE)cself;
    if ( ! ri->isValid )
    {
        nullObjectException(context->threadContext, RESOURCEIMAGECLASS);
        goto out;
    }
    ri->lastError = 0;

    if ( ! getStandardImageArgs(context, &type, IMAGE_BITMAP, size, &s, &flags, LR_SHARED) )
    {
        goto out;
    }

    result = rxImagesFromArrayOfInts(context, ids, ri->hMod, type, &s, flags);
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
        nullObjectException(context->threadContext, RESOURCEIMAGECLASS);
    }
    return ((PRESOURCEIMAGE)ri)->hMod;
}

RexxMethod1(logical_t, ri_isNull, CSELF, ri) { return ( ! ((PRESOURCEIMAGE)ri)->isValid); }
RexxMethod1(uint32_t, ri_systemErrorCode, CSELF, ri) { return ((PRESOURCEIMAGE)ri)->lastError; }


