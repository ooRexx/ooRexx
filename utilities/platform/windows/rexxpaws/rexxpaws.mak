#/*----------------------------------------------------------------------------*/
#/*                                                                            */
#/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
#/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
#/*                                                                            */
#/* This program and the accompanying materials are made available under       */
#/* the terms of the Common Public License v1.0 which accompanies this         */
#/* distribution. A copy is also available at the following address:           */
#/* https://www.oorexx.org/license.html                         */
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
# rexxpaws.mak make file
#------------------------
# What we want to build...
all: $(OR_OUTDIR)\rexxpaws.exe COPYICOFILES
     @ECHO ...
     @ECHO All done ....

!include "$(OR_LIBSRC)\ORXWIN32.MAK"

!IFNDEF OR_WINKERNELSRC
!ERROR Build error, OR_WINKERNELSRC not set
!ENDIF

# define component-specific dependencies
.SUFFIXES: .ico

REXXPOBJ = $(OR_OUTDIR)\rexxpaws.obj

ICOFILES=$(OR_OUTDIR)\orxw.ico

$(OR_OUTDIR)\rexxpaws.exe : $(REXXPOBJ) $(OR_OUTDIR)\rexx.res
    $(OR_LINK) $(REXXPOBJ) $(OR_OUTDIR)\rexx.res $(lflags_common_console) /STACK:524288 \
    $(OR_OUTDIR)\rexxapi.lib \
    $(OR_OUTDIR)\rexx.lib \
    -out:$(OR_OUTDIR)\$(@B).exe

#
# *** Copy ICO files to target dir...
#
COPYICOFILES: $(ICOFILES)

#
# *** Inference Rule for ICOFILES
#
{$(OR_WINKERNELSRC)}.ico{$(OR_OUTDIR)}.ico:
    @ECHO .
    @ECHO Copying $(@B).ico
    COPY $(OR_WINKERNELSRC)\$(@B).ico $(OR_OUTDIR)\$(@B).ico
#
# NEED individual dependencies placed here eventually
#


# Update the version information block
$(OR_OUTDIR)\rexx.res: $(OR_WINKERNELSRC)\rexx.rc
    @ECHO .
    @ECHO ResourceCompiling $(@B).res
        $(rc) $(rcflags_common) -r -fo$(OR_OUTDIR)\$(@B).res $(OR_WINKERNELSRC)\$(@B).rc

#
# *** Inference Rule for C->OBJ
# *** For .C files in OR_LIBSRC directory
#
{$(OR_UTILITIES)\platform\windows\rexxpaws}.cpp{$(OR_OUTDIR)}.obj:
    @ECHO .
    @ECHO Compiling $(**)
    $(OR_CC) $(cflags_common) $(cflags_dll) /Fo$(@) $(OR_ORYXINCL) $(Tp)$(**)
