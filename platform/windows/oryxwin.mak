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
# ORYXWIN.MAK make file
#------------------------
# What we want to build...
all: $(OR_OUTDIR)\rexximage.exe $(OR_OUTDIR)\rexx.exe $(OR_OUTDIR)\orx.exe $(OR_OUTDIR)\rexxhide.exe $(OR_OUTDIR)\rexxpaws.exe COPYICOFILES
     @ECHO ...
     @ECHO All done ....

!include "$(OR_ORYXLSRC)\ORXWIN32.MAK"

!IFNDEF OR_ORYXWSRC
!ERROR Build error, OR_ORYXWSRC not set
!ENDIF

# define component-specific dependencies
.SUFFIXES: .ico
#orxw
REXXCOBJ = $(OR_OUTDIR)\rexx.obj
REXXIOBJ = $(OR_OUTDIR)\rexximage.obj
REXXCHOBJ = $(OR_OUTDIR)\rexxhide.obj
REXXPOBJ = $(OR_OUTDIR)\rexxpaws.obj
KERNELOBJ =

ICOFILES=$(OR_OUTDIR)\orxw.ico

#
# *** ORX.EXE
#
# We build this module as ORX.EXE, then copy it to rexx.exe so we can avoid problems
# with debug information for rexx.dll and rexx.exe.
$(OR_OUTDIR)\ORX.exe : $(REXXCOBJ) $(KERNELOBJ) $(OR_OUTDIR)\rexx.res
    $(OR_LINK) $(REXXCOBJ) $(KERNELOBJ) $(OR_OUTDIR)\rexx.res $(lflags_common_console) /STACK:524288 \
    $(OR_OUTDIR)\rexxapi.lib \
    $(OR_OUTDIR)\rexx.lib \
    $(libs_dll) \
    -out:$(OR_OUTDIR)\$(@B).exe

$(OR_OUTDIR)\rexx.exe : $(OR_OUTDIR)\orx.exe
    @ECHO .
    @ECHO Copying $(@B).exe
    COPY $(OR_OUTDIR)\orx.exe $(OR_OUTDIR)\$(@B).exe

$(OR_OUTDIR)\rexxhide.exe : $(REXXCHOBJ) $(KERNELOBJ) $(OR_OUTDIR)\rexx.res
    $(OR_LINK) $(REXXCHOBJ) $(KERNELOBJ) $(OR_OUTDIR)\rexx.res $(lflags_common) /STACK:524288 \
    $(OR_OUTDIR)\rexxapi.lib \
    $(OR_OUTDIR)\rexx.lib \
    $(libs_dll) \
    -out:$(OR_OUTDIR)\$(@B).exe

$(OR_OUTDIR)\rexxpaws.exe : $(REXXPOBJ) $(KERNELOBJ) $(OR_OUTDIR)\rexx.res
    $(OR_LINK) $(REXXPOBJ) $(KERNELOBJ) $(OR_OUTDIR)\rexx.res $(lflags_common_console) /STACK:524288 \
    $(OR_OUTDIR)\rexxapi.lib \
    $(OR_OUTDIR)\rexx.lib \
    $(libs_dll) \
    -out:$(OR_OUTDIR)\$(@B).exe
#
# *** rexximage.exe
#
# oryxi link is for setimagename only
# build orx.exe so there is no problem with debug infor for rexx.exe and rexx.dll
$(OR_OUTDIR)\rexximage.exe : $(REXXIOBJ) $(KERNELOBJ) $(OR_OUTDIR)\rexx.res
    $(OR_LINK) $(REXXIOBJ) $(KERNELOBJ) $(OR_OUTDIR)\rexx.res $(lflags_common_console) /STACK:524288 \
    $(OR_OUTDIR)\rexxapi.lib \
    $(OR_OUTDIR)\rexx.lib \
    $(libs_dll) \
    -out:$(OR_OUTDIR)\$(@B).exe

# *** Inference Rule for C->OBJ
#
{$(OR_ORYXWSRC)}.c{$(OR_OUTDIR)}.obj:
    @ECHO .
    @ECHO Compiling $(@B).c
    $(OR_CC) $(cflags_common) $(cflags_dll) /Fo$(OR_OUTDIR)\$(@B).obj $(OR_ORYXWSRC)\$(@B).c $(OR_ORYXINCL)

#
# *** Inference Rule for local C->OBJ
#
{$(OR_OUTDIR)}.c{$(OR_OUTDIR)}.obj:
    @ECHO .
    @ECHO Compiling $(@B).c
    $(OR_CC)  $(cflags_common) $(cflags_dll) /Fo$(OR_OUTDIR)\$(@B).obj $(OR_OUTDIR)\$(@B).c $(OR_ORYXINCL)

# *** Inference Rule for REXX->OBJ
#
$(OR_OUTDIR)\rexx.obj: $(OR_ORYXWSRC)\rexx.c
    @ECHO .
    @ECHO Compiling $(@B).c
    $(OR_CC) $(cflags_common) $(cflags_dll) /Fo$(OR_OUTDIR)\$(@B).obj $(OR_ORYXINCL) $(Tp)$(OR_ORYXWSRC)\$(@B).c

$(OR_OUTDIR)\rexxhide.obj: $(OR_ORYXWSRC)\rexxhide.c
    @ECHO .
    @ECHO Compiling $(@B).c
    $(OR_CC) $(cflags_common) $(cflags_dll) /Fo$(OR_OUTDIR)\$(@B).obj $(OR_ORYXINCL) $(Tp)$(OR_ORYXWSRC)\$(@B).c

$(OR_OUTDIR)\rexxpaws.obj: $(OR_ORYXWSRC)\rexxpaws.c
    @ECHO .
    @ECHO Compiling $(@B).c
    $(OR_CC) $(cflags_common) $(cflags_dll) /Fo$(OR_OUTDIR)\$(@B).obj $(OR_ORYXINCL) $(Tp)$(OR_ORYXWSRC)\$(@B).c

$(OR_OUTDIR)\rexximage.obj: $(OR_ORYXWSRC)\rexximage.cpp
    @ECHO .
    @ECHO Compiling $(@B).c
    $(OR_CC) $(cflags_common) $(cflags_dll) /Fo$(OR_OUTDIR)\$(@B).obj $(OR_ORYXINCL) $(Tp)$(OR_ORYXWSRC)\$(@B).cpp

#
# *** Copy ICO files to target dir...
#
COPYICOFILES: $(ICOFILES)

#
# *** Inference Rule for ICOFILES
#
{$(OR_ORYXWSRC)}.ico{$(OR_OUTDIR)}.ico:
    @ECHO .
    @ECHO Copying $(@B).ico
    COPY $(OR_ORYXWSRC)\$(@B).ico $(OR_OUTDIR)\$(@B).ico
#
# NEED individual dependencies placed here eventually
#


# Update the version information block
$(OR_OUTDIR)\rexx.res: $(OR_ORYXWSRC)\rexx.rc
    @ECHO .
    @ECHO ResourceCompiling $(@B).res
        $(rc) $(rcflags_common) -r -fo$(OR_OUTDIR)\$(@B).res $(OR_ORYXWSRC)\$(@B).rc
