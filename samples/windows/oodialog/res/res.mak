#/*----------------------------------------------------------------------------*/
#/*                                                                            */
#/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
#/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
#/*                                                                            */
#/* This program and the accompanying materials are made available under       */
#/* the terms of the Common Public License v1.0 which accompanies this         */
#/* distribution. A copy is also available at the following address:           */
#/* https://www.oorexx.org/license.html                                        */
#/*                                                                            */
#/* Redistribution and use in source and binary forms, with or                 */
#/* without modification, are permitted provided that the following            */
#/* conditions are met:                                                        */
#/*                                                                            */
#/* Redistributions of source code must retain the above copyright             */
#/* notice, this list of conditions and the following disclaimer.              */
#/* Redistributions in binary form must reproduce the above copyright          */
#/* notice, this list of conditions and the following disclaimer in            */
#/* the documentation and/or other materials provided with the distribution.   */
#/*                                                                            */
#/* Neither the name of Rexx Language Association nor the names                */
#/* of its contributors may be used to endorse or promote products             */
#/* derived from this software without specific prior written permission.      */
#/*                                                                            */
#/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS        */
#/* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT          */
#/* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS          */
#/* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   */
#/* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,      */
#/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
#/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,        */
#/* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY     */
#/* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING    */
#/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         */
#/* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               */
#/*                                                                            */
#/*----------------------------------------------------------------------------*/
# NMAKE-compatible MAKE file for ooDialog resource DLLs

rcflags_common = /DWIN32 /v

all:  oowalk2.dll AnimalGame.dll ..\propertySheet.tabControls\rc\PropertySheetDemo.dll \
      ..\propertySheet.tabControls\rc\TabOwnerDemo.dll ..\resizableDialogs\ResizingAdmin\rc\PropertySheetDemo.dll

oowalk2.dll: oowalk2.res
    link /NOLOGO  $(@B).res /NOENTRY /DLL /MACHINE:$(MACHINE) -out:$(@B).dll

AnimalGame.dll: AnimalGame.res
    link /NOLOGO $(@B).res /NOENTRY /DLL /MACHINE:$(MACHINE) -out:$(@B).dll

..\propertySheet.tabControls\rc\PropertySheetDemo.dll: PropertySheetDemo.res
    link /NOLOGO $(@B).res /NOENTRY /DLL /MACHINE:$(MACHINE) -out:..\propertySheet.tabControls\rc\$(@B).dll

..\propertySheet.tabControls\rc\TabOwnerDemo.dll: TabOwnerDemo.res
    link /NOLOGO $(@B).res /NOENTRY /DLL /MACHINE:$(MACHINE) -out:..\propertySheet.tabControls\rc\$(@B).dll

..\resizableDialogs\ResizingAdmin\rc\PropertySheetDemo.dll: PropertySheetDemo.res
    link /NOLOGO $(@B).res /NOENTRY /DLL /MACHINE:$(MACHINE) -out:..\resizableDialogs\ResizingAdmin\rc\$(@B).dll

# Create .res from .rc
oowalk2.res: ..\rc\walker.rc
        rc $(rcflags_common) -r -fo$(@B).res ..\rc\walker.rc

AnimalGame.res: ..\rc\AnimalGame.rc
        rc $(rcflags_common) -r -fo$(@B).res ..\rc\AnimalGame.rc

PropertySheetDemo.res: ..\propertySheet.tabControls\rc\PropertySheetDemo.rc
        rc $(rcflags_common) -r -fo$(@B).res ..\propertySheet.tabControls\rc\PropertySheetDemo.rc

TabOwnerDemo.res: ..\propertySheet.tabControls\rc\TabOwnerDemo.rc
        rc $(rcflags_common) -r -fo$(@B).res ..\propertySheet.tabControls\rc\TabOwnerDemo.rc
