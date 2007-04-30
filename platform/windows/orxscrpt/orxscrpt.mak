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
# ORXSCRPT.MAK make file
#------------------------
#all: $(OR_OUTDIR)\ORXSCRPT.dll \
#     $(OR_OUTDIR)\ORXSCRPT.cmd
all: $(OR_OUTDIR)\ORXSCRPT.dll
    @ECHO .
    @ECHO All done ORXSCRPT.DLL
    @ECHO .

!include "$(OR_ORYXLSRC)\ORXWIN32.MAK"

!IFNDEF OR_ORYXAXSCRIPT
!ERROR Build error, OR_ORYXAXSCRIPT not set
!ENDIF


CPPOBJS = $(OR_OUTDIR)\dllfuncs.obj   \
          $(OR_OUTDIR)\ORXSCRPT.obj  \
          $(OR_OUTDIR)\OrxScrptError.obj  \
          $(OR_OUTDIR)\OrxEvents.obj  \
          $(OR_OUTDIR)\OrxDispID.obj  \
          $(OR_OUTDIR)\OrxIDispatch.obj  \
          $(OR_OUTDIR)\nameditem.obj  \
          $(OR_OUTDIR)\eng2rexx.obj   \
          $(OR_OUTDIR)\scrptdebug.obj \
          $(OR_OUTDIR)\engfact.obj    \
          $(OR_OUTDIR)\scriptutil.obj  \
          $(OR_OUTDIR)\classfactory.obj

# Following for ORXSCRPT.LIB
#
# *** ORXSCRPT.LIB  : Creates .lib import library
#                          .exp export library for use with this link
#
# Generate import library (.lib) and export library (.exp) from
# module-definition (.dfw) file for a DLL
$(OR_OUTDIR)\ORXSCRPT.lib : $(CPPOBJS) $(OR_ORYXAXSCRIPT)\ORXSCRPT.def
        $(OR_IMPLIB) -machine:$(CPU) \
        -def:$(OR_ORYXAXSCRIPT)\ORXSCRPT.def \
        $(OR_OUTDIR)\orexxole.lib \
        $(CPPOBJS)               \
        -out:$(OR_OUTDIR)\ORXSCRPT.lib

#
# *** ORXSCRPT.DLL
#
# need import libraries and def files still
$(OR_OUTDIR)\ORXSCRPT.dll : $(CPPOBJS) $(RXDBG_OBJ) $(OR_OUTDIR)\ORXSCRPT.lib \
                            $(OR_ORYXAXSCRIPT)\ORXSCRPT.def     \
                            $(OR_OUTDIR)\ORXSCRPT.exp
    $(OR_LINK) -map $(lflags_common) $(lflags_dll) -out:$(OR_OUTDIR)\$(@B).dll \
             $(CPPOBJS) $(RXDBG_OBJ) \
             $(OR_OUTDIR)\verinfo.res \
             $(OR_OUTDIR)\$(@B).exp \
             $(OR_OUTDIR)\orexxole.lib \
             $(OR_OUTDIR)\rexx.lib \
             $(OR_OUTDIR)\rexxapi.lib \
             $(libs_dll)

#
# *** .cpp -> .obj rules
#
$(CPPOBJS):  $(@B).cpp
    @ECHO .
    @ECHO Compiling $(@B).cpp
    $(OR_CC) $(cflags_common) /GX $(cflags_dll) /Fo$(OR_OUTDIR)\$(@B).obj $(OR_ORYXINCL) $(OR_ORYXAXSCRIPT)\$(@B).cpp
