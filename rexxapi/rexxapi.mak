#/*----------------------------------------------------------------------------*/
#/*                                                                            */
#/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
#/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
#/*                                                                            */
#/* This program and the accompanying materials are made available under       */
#/* the terms of the Common Public License v1.0 which accompanies this         */
#/* distribution. A copy is also available at the following address:           */
#/* http://www.oorexx.org/license.html                          */
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
#------------------------
# REXXAPI.MAK make file
#------------------------
TARGET=rexxapi

all: $(OR_OUTDIR)\$(TARGET).dll $(OR_OUTDIR)\rxapi.exe
    @ECHO.
    @ECHO All done $(TARGET).dll  rxapi.exe
    @ECHO.

!include "$(OR_ORYXLSRC)\ORXWIN32.MAK"

!IFNDEF OR_ORYXASRC
!ERROR Build error, OR_ORYXASRC not set
!ENDIF

#lflags_common = $(lflags_common) kernel32.lib

OBJLIST = $(OR_OUTDIR)\RexxAPIManager.obj $(OR_OUTDIR)\QueuesAPI.obj \
          $(OR_OUTDIR)\MacroSpace.obj $(OR_OUTDIR)\APIUtilities.obj \
          $(OR_OUTDIR)\SubcommandAPI.obj \

OBJLST2 = $(OR_OUTDIR)\QueuesAPI.obj  $(OR_OUTDIR)\RexxAPIManager.obj \
          $(OR_OUTDIR)\MacroSpace.obj $(OR_OUTDIR)\APIUtilities.obj

# Following for REXXAPI.DLL
#
# *** REXXAPI.LIB  : Creates .lib import library
#                            .exp export library for use with this link
#
# Generate import library (.lib) and export library (.exp) from
# module-definition (.dfw) file for a DLL
$(OR_OUTDIR)\$(TARGET).lib : $(OBJLIST) $(APLATFORM)\rexxapi.def
        $(OR_IMPLIB) -machine:$(CPU) \
        -def:$(APLATFORM)\$(TARGET).def               \
        $(OBJLIST)               \
        -out:$(OR_OUTDIR)\$(TARGET).lib

#
# *** REXXAPI.DLL
#
# need import libraries and def files still
$(OR_OUTDIR)\$(TARGET).dll : $(OBJLIST) $(RXDBG_OBJ)      \
                             $(OR_OUTDIR)\$(TARGET).lib   \
                             $(APLATFORM)\$(TARGET).def \
                             $(OR_OUTDIR)\$(TARGET).exp   \
                             $(OR_OUTDIR)\verinfo.res
    $(OR_LINK) $(lflags_common) $(lflags_dll) /MAP -out:$(OR_OUTDIR)\$(@B).dll \
             $(OBJLIST) $(RXDBG_OBJ) \
             $(OR_OUTDIR)\verinfo.res \
             $(OR_OUTDIR)\$(@B).exp \
             $(libs_dll)

#
# *** rxapi.EXE
#
$(OR_OUTDIR)\rxapi.exe : $(OR_OUTDIR)\RexxAPIService.obj $(OR_OUTDIR)\rxapi.res
    $(OR_LINK) $(OR_OUTDIR)\RexxAPIService.obj $(OR_OUTDIR)\rxapi.res /MAP \
    $(lflags_common) $(lflags_exe) \
    $(OR_OUTDIR)\Rexxapi.lib  \
    $(libs_dll) \
    /DELAYLOAD:advapi32.dll \
    -out:$(OR_OUTDIR)\$(@B).exe


# Update the resource if necessary
$(OR_OUTDIR)\rxapi.res: $(APLATFORM)\rxapi.rc $(APLATFORM)\APIServiceMessages.h
    @ECHO.
    @ECHO ResourceCompiling $(@B).res
        $(rc) $(rcflags_common) -r -fo $(OR_OUTDIR)\rxapi.res $(APLATFORM)\rxapi.rc

#
# *** Inference Rule for APIMAIN.C->OBJ
#
$(OR_OUTDIR)\RexxAPIService.obj:$(APLATFORM)\RexxAPIService.c
    @ECHO.
    @ECHO Compiling $(**)
    $(OR_CC) $(cflags_common) $(cflags_exe)  /Fo$(@) $(OR_ORYXINCL) $(**)

#
# *** Inference Rule for rxapi.exe C->OBJ, if no Local C file
#
{$(OR_ORYXASRC)}.c{$(OR_OUTDIR)}.obj:
    @ECHO.
    @ECHO Compiling $(**)
    $(OR_CC) $(cflags_common) $(cflags_dll) /Fo$(@) $(OR_ORYXINCL) $(**)

#
# *** Inference Rule for rxapi.exe C->OBJ, if no Local C file
#
{$(APLATFORM)}.c{$(OR_OUTDIR)}.obj:
    @ECHO.
    @ECHO Compiling $(**)
    $(OR_CC) $(cflags_common) $(cflags_dll) /Fo$(@) $(OR_ORYXINCL) $(**)


#
# *** Inference Rule for local C->OBJ
#
{$(OR_OUTDIR)}.c{$(OR_OUTDIR)}.obj:
    @ECHO.
    @ECHO Compiling $(**)
    $(OR_CC) $(cflags_common) $(cflags_dll) /Fo$(@) $(OR_ORYXINCL) $(**)

#
# NEED individual dependencies placed here eventually
#

# Update the version information block
$(OR_OUTDIR)\verinfo.res: $(KWINDOWS)\verinfo.rc
    @ECHO.
    @ECHO ResourceCompiling $(@B).res
        $(rc) $(rcflags_common) -r -fo$(@) $(**)
