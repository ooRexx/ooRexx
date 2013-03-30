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

/**
 * Windows Dialog Interface for Open Object Rexx (ooRexx.)
 *
 * ooDialog classes that are more Shell orientated than the original ooDialog
 * implementation.
 *
 * BrowseForFolder     Main 'browse for folder' class, allows customization.
 *
 * SimpleFolderBrowse  Simplified version of BrowseForFolder with ???
 *
 */


::class 'BrowseForFolder' public

::attribute banner get external "LIBRARY oodialog bff_banner"
::attribute banner set external "LIBRARY oodialog bff_setBanner"
::attribute dlgTitle get external "LIBRARY oodialog bff_dlgTitle"
::attribute dlgTitle set external "LIBRARY oodialog bff_setDlgTitle"
::attribute hint get external "LIBRARY oodialog bff_hint"
::attribute hint set external "LIBRARY oodialog bff_setHint"
::attribute initialThread get external "LIBRARY oodialog bff_initialThread"
::attribute options get external "LIBRARY oodialog bff_options"
::attribute options set external "LIBRARY oodialog bff_setOptions"
::attribute owner get external "LIBRARY oodialog bff_Owner"
::attribute owner set external "LIBRARY oodialog bff_setOwner"
::attribute root get external "LIBRARY oodialog bff_root"
::attribute root set external "LIBRARY oodialog bff_setRoot"
::attribute startDir get external "LIBRARY oodialog bff_startDir"
::attribute startDir set external "LIBRARY oodialog bff_setStartDir"
::attribute usePathForHint get external "LIBRARY oodialog bff_usePathForHint"
::attribute usePathForHint set external "LIBRARY oodialog bff_setUsePathForHint"

::method init external "LIBRARY oodialog bff_init"
::method uninit external "LIBRARY oodialog bff_uninit"

::method getDisplayName external "LIBRARY oodialog bff_getDisplayName"
::method getFolder external "LIBRARY oodialog bff_getFolder"
::method getItemIDList external "LIBRARY oodialog bff_getItemIDList"
::method initCOM external "LIBRARY oodialog bff_initCOM"
::method releaseCOM external "LIBRARY oodialog bff_releaseCOM"
::method releaseItemIDList external "LIBRARY oodialog bff_releaseItemIDList"
::method test external "LIBRARY oodialog bff_test"


::class 'SimpleFolderBrowse' public

::method getFolder class external "LIBRARY oodialog sfb_getFolder"

