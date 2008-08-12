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

all: $(OR_OUTDIR)\rexxapi.dll $(OR_OUTDIR)\rxapi.exe
    @ECHO.
    @ECHO All done rexxapi.dll  rxapi.exe
    @ECHO.

!include "$(OR_LIBSRC)\ORXWIN32.MAK"

!IFNDEF OR_REXXAPISRC
!ERROR Build error, OR_REXXAPISRC not set
!ENDIF

COMMONINC = -I$(OR_REXXAPISRC)\common -I$(OR_REXXAPISRC)\common\platform\windows
CLIENTINC = -I$(OR_REXXAPISRC)\client -I$(OR_REXXAPISRC)\client\platform\windows
SERVERINC = -I$(OR_REXXAPISRC)\server -I$(OR_REXXAPISRC)\server\platform\windows

CLIENTOBJS = $(OR_OUTDIR)\ClientMessage.obj $(OR_OUTDIR)\LocalAPIContext.obj \
          $(OR_OUTDIR)\LocalAPIManager.obj $(OR_OUTDIR)\LocalQueueManager.obj \
          $(OR_OUTDIR)\LocalMacroSpaceManager.obj $(OR_OUTDIR)\LocalRegistrationManager.obj \
          $(OR_OUTDIR)\MacroSpaceApi.obj $(OR_OUTDIR)\QueuesApi.obj  $(OR_OUTDIR)\RegistrationApi.obj \
          $(OR_OUTDIR)\ServiceMessage.obj $(OR_OUTDIR)\SysCSStream.obj $(OR_OUTDIR)\SysProcess.obj \
          $(OR_OUTDIR)\Utilities.obj $(OR_OUTDIR)\SysLegacyAPI.obj $(OR_OUTDIR)\SysFile.obj \
          $(OR_OUTDIR)\SysLocalAPIManager.obj $(OR_OUTDIR)\SysLibrary.obj $(OR_OUTDIR)\SysAPIManager.obj \
          $(OR_OUTDIR)\SysSemaphore.obj

SERVEROBJS = $(OR_OUTDIR)\APIServer.obj $(OR_OUTDIR)\APIServerInstance.obj \
          $(OR_OUTDIR)\MacroSpaceManager.obj $(OR_OUTDIR)\QueueManager.obj \
          $(OR_OUTDIR)\RegistrationManager.obj $(OR_OUTDIR)\ServiceMessage.obj \
          $(OR_OUTDIR)\APIService.obj $(OR_OUTDIR)\SysCSStream.obj $(OR_OUTDIR)\SysProcess.obj \
          $(OR_OUTDIR)\SysAPIManager.obj $(OR_OUTDIR)\SysThread.obj $(OR_OUTDIR)\SysSemaphore.obj \
          $(OR_OUTDIR)\Utilities.obj $(OR_OUTDIR)\APIServerThread.obj

# Following for REXXAPI.DLL
#
# *** REXXAPI.LIB  : Creates .lib import library
#                            .exp export library for use with this link
#
# Generate import library (.lib) and export library (.exp) from
# module-definition (.dfw) file for a DLL
$(OR_OUTDIR)\rexxapi.lib : $(CLIENTOBJS) $(OR_REXXAPISRC)\client\platform\windows\rexxapi.def
        $(OR_IMPLIB) -machine:$(CPU) \
        -def:$(OR_REXXAPISRC)\client\platform\windows\rexxapi.def \
        $(CLIENTOBJS) \
        -out:$(OR_OUTDIR)\rexxapi.lib

#
# *** REXXAPI.DLL
#
# need import libraries and def files still
$(OR_OUTDIR)\rexxapi.dll : $(CLIENTOBJS) \
                             $(OR_OUTDIR)\rexxapi.lib   \
                             $(OR_REXXAPISRC)\client\platform\windows\rexxapi.def \
                             $(OR_OUTDIR)\rexxapi.exp   \
                             $(OR_OUTDIR)\verinfo.res
    $(OR_LINK) $(lflags_common) $(lflags_dll) /MAP -out:$(OR_OUTDIR)\$(@B).dll \
             $(CLIENTOBJS) \
             $(OR_OUTDIR)\verinfo.res \
             $(OR_OUTDIR)\$(@B).exp \
             wsock32.lib

#
# *** rxapi.EXE
#
$(OR_OUTDIR)\rxapi.exe : $(SERVEROBJS) $(OR_OUTDIR)\rxapi.res
    $(OR_LINK) $(SERVEROBJS) $(OR_OUTDIR)\rxapi.res /MAP \
    $(lflags_common) $(lflags_exe) \
    -out:$(OR_OUTDIR)\$(@B).exe \
    wsock32.lib


# Update the resource if necessary
$(OR_OUTDIR)\rxapi.res: $(OR_REXXAPISRC)\server\platform\windows\rxapi.rc
    @ECHO.
    @ECHO ResourceCompiling $(@B).res
        $(rc) $(rcflags_common) -r -fo $(OR_OUTDIR)\rxapi.res $(OR_REXXAPISRC)\server\platform\windows\rxapi.rc

#
# *** Inference Rule for CPP->OBJ
# *** For .CPP files in OR_LIBSRC directory
#
{$(OR_REXXAPISRC)\client}.cpp{$(OR_OUTDIR)}.obj:
    @ECHO.
    @ECHO Compiling $(**)
    $(OR_CC) $(cflags_common) $(cflags_dll)  /Fo$(@) $(CLIENTINC) $(COMMONINC) $(OR_ORYXINCL) $(Tp)$(**)

#
# *** Inference Rule for CPP->OBJ
# *** For .CPP files in OR_LIBSRC directory
#
{$(OR_REXXAPISRC)\client\platform\windows}.cpp{$(OR_OUTDIR)}.obj:
    @ECHO.
    @ECHO Compiling $(**)
    $(OR_CC) $(cflags_common) $(cflags_dll)  /Fo$(@) $(CLIENTINC) $(COMMONINC) $(OR_ORYXINCL) $(Tp)$(**)

#
# *** Inference Rule for CPP->OBJ
# *** For .CPP files in OR_LIBSRC directory
#
{$(OR_REXXAPISRC)\server}.cpp{$(OR_OUTDIR)}.obj:
    @ECHO.
    @ECHO Compiling $(**)
    $(OR_CC) $(cflags_common) $(cflags_dll)  /Fo$(@) $(SERVERINC) $(COMMONINC) $(OR_ORYXINCL) $(Tp)$(**)

#
# *** Inference Rule for CPP->OBJ
# *** For .CPP files in OR_LIBSRC directory
#
{$(OR_REXXAPISRC)\server\platform\windows}.cpp{$(OR_OUTDIR)}.obj:
    @ECHO.
    @ECHO Compiling $(**)
    $(OR_CC) $(cflags_common) $(cflags_dll)  /Fo$(@) $(SERVERINC) $(COMMONINC) $(OR_ORYXINCL) $(Tp)$(**)

#
# *** Inference Rule for CPP->OBJ
# *** For .CPP files in OR_LIBSRC directory
#
{$(OR_REXXAPISRC)\common}.cpp{$(OR_OUTDIR)}.obj:
    @ECHO .
    @ECHO Compiling $(**)
    $(OR_CC) $(cflags_common) $(cflags_dll)  /Fo$(@) $(COMMONINC) $(OR_ORYXINCL) $(Tp)$(**)

#
# *** Inference Rule for CPP->OBJ
# *** For .CPP files in OR_LIBSRC directory
#
{$(OR_REXXAPISRC)\common\platform\windows}.cpp{$(OR_OUTDIR)}.obj:
    @ECHO .
    @ECHO Compiling $(**)
    $(OR_CC) $(cflags_common) $(cflags_dll)  /Fo$(@) $(COMMONINC) $(OR_ORYXINCL) $(Tp)$(**)

# Update the version information block
$(OR_OUTDIR)\verinfo.res: $(INT_PLATFORM)\verinfo.rc
    @ECHO.
    @ECHO ResourceCompiling $(@B).res
        $(rc) $(rcflags_common) -r -fo$(@) $(**)
