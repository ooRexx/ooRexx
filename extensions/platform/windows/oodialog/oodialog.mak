#/*----------------------------------------------------------------------------*/
#/*                                                                            */
#/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
#/* Copyright (c) 2005-2008 Rexx Language Association. All rights reserved.    */
#/*                                                                            */
#/* This program and the accompanying materials are made available under       */
#/* the terms of the Common Public License v1.0 which accompanies this         */
#/* distribution. A copy is also available at the following address:           */
#/* http://www.oorexx.org/license.html                                         */
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
# NMAKE-compatible MAKE file for FNTEST*
all:  $(OR_OUTDIR)\oodialog.dll

!include "$(OR_LIBSRC)\ORXWIN32.MAK"
C=cl
OPTIONS= $(cflags_common) $(cflags_dll) $(OR_ORYXINCL)
OR_LIB=$(OR_OUTDIR)

SOURCEF= $(OR_OUTDIR)\oovutil.obj $(OR_OUTDIR)\oovdata.obj $(OR_OUTDIR)\oovtext.obj $(OR_OUTDIR)\oovtools.obj \
         $(OR_OUTDIR)\oovmsg.obj $(OR_OUTDIR)\oovscrll.obj $(OR_OUTDIR)\oovdeskt.obj $(OR_OUTDIR)\oovdraw.obj \
         $(OR_OUTDIR)\oovuser.obj $(OR_OUTDIR)\oovbmp.obj $(OR_OUTDIR)\oovother.obj $(OR_OUTDIR)\menu.obj \
         $(OR_OUTDIR)\oodialog.res

.c{$(OR_OUTDIR)}.obj:
    $(C) $(OPTIONS)  /DINCL_32  -c $(@B).c /DCREATEDLL /Fo$(OR_OUTDIR)\$(@B).obj

#
# *** .cpp -> .obj rules
#
{$(OR_OODIALOGSRC)}.cpp{$(OR_OUTDIR)}.obj:
    @ECHO .
    @ECHO Compiling $(@B).cpp
    $(OR_CC) $(cflags_common) $(cflags_dll) /DCREATEDLL /Fo$(OR_OUTDIR)\$(@B).obj $(OR_ORYXINCL)  $(OR_OODIALOGSRC)\$(@B).cpp


{$(OR_OODIALOGSRC)}.c{$(OR_OUTDIR)}.obj:
    @ECHO .
    @ECHO Compiling $(@B).c
    $(OR_CC) $(cflags_common) $(cflags_dll) /DCREATEDLL /Fo$(OR_OUTDIR)\$(@B).obj $(OR_ORYXINCL)  $(OR_OODIALOGSRC)\$(@B).c


$(OR_OUTDIR)\oodialog.dll:     $(SOURCEF)
    $(OR_LINK) \
        $(SOURCEF)  \
    $(lflags_common) $(lflags_dll) \
    $(OR_LIB)\rexx.lib \
    $(OR_LIB)\rexxapi.lib \
    WINMM.LIB \
    COMDLG32.LIB \
    COMCTL32.LIB \
    shlwapi.lib \
    -def:$(OR_OODIALOGSRC)\oovutil.def \
    -out:$(OR_OUTDIR)\$(@B).dll


# Update the version information block
$(OR_OUTDIR)\oodialog.res: $(OR_OODIALOGSRC)\oodialog.rc
    @ECHO .
    @ECHO ResourceCompiling $(@B).res
        $(rc) $(rcflags_common) /i $(OR_OODIALOGSRC) /i $(OR_WINKERNELSRC) -r -fo$(OR_OUTDIR)\$(@B).res $(OR_OODIALOGSRC)\$(@B).rc
