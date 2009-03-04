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
# RXSOCK.MAK make file
#------------------------
all: $(OR_OUTDIR)\rxsock.dll $(OR_OUTDIR)\socket.cls
    @ECHO .
    @ECHO All done rxsock.dll and socket.cls
    @ECHO .

!include "$(OR_LIBSRC)\ORXWIN32.MAK"

SOURCE_DIR = $(OR_EXTENSIONS)\rxsock

OBJS = $(OR_OUTDIR)\rxsock.obj $(OR_OUTDIR)\rxsockfn.obj

# Following for rxsock.DLL
#
# *** rxsock.LIB  : Creates .lib import library
#                            .exp export library for use with this link
#
# Generate import library (.lib) and export library (.exp) from
# module-definition (.dfw) file for a DLL
$(OR_OUTDIR)\rxsock.lib : $(OBJS) $(OR_WINKERNELSRC)\rxsock.def
        $(OR_IMPLIB) -machine:$(CPU) \
        -def:$(OR_WINKERNELSRC)\rxsock.def               \
        $(OBJS)               \
        -out:$(OR_OUTDIR)\rxsock.lib

#
# *** rxsock.DLL
#
# need import libraries and def files still
$(OR_OUTDIR)\rxsock.dll : $(OBJS) $(OR_OUTDIR)\rxsock.lib \
                          $(OR_WINKERNELSRC)\rxsock.def $(OR_OUTDIR)\rxsock.exp
    $(OR_LINK) $(lflags_common) $(lflags_dll)  -out:$(OR_OUTDIR)\$(@B).dll \
             $(OBJS) \
             $(OR_OUTDIR)\verinfo.res \
             $(OR_OUTDIR)\$(@B).exp \
             $(OR_OUTDIR)\rexx.lib \
             wsock32.lib \
             $(OR_OUTDIR)\rexxapi.lib


# *** Inference Rule for CPP->OBJ
# *** For .CPP files in OR_LIBSRC directory
#
$(OBJS):  $(SOURCE_DIR)\$(@B).cpp
    @ECHO .
    @ECHO Compiling $(@B).cpp
    $(OR_CC) $(cflags_common) $(cflags_dll) /Fo$(OR_OUTDIR)\$(@B).obj $(OR_ORYXINCL) -I$(SOURCE_DIR)\ $(SOURCE_DIR)\$(@B).cpp

#
# Copy socket.cls to the build directory so the test suite can be run directly
# from that location without doing an install.
#
$(OR_OUTDIR)\socket.cls : $(SOURCE_DIR)\socket.cls
    @ECHO .
    @ECHO Copying $(SOURCEDIR)\socket.cls
    copy $(SOURCE_DIR)\socket.cls $(OR_OUTDIR)

