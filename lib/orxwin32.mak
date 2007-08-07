#/*******************************************************************************
# Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.
# Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.
#
# This program and the accompanying materials are made available under
# the terms of the Common Public License v1.0 which accompanies this
# distribution. A copy is also available at the following address:
# http://www.oorexx.org/license.html
#
# Redistribution and use in source and binary forms, with or
# without modification, are permitted provided that the following
# conditions are met:
#
# Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
# Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in
# the documentation and/or other materials provided with the distribution.
#
# Neither the name of Rexx Language Association nor the names
# of its contributors may be used to endorse or promote products
# derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
# TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
# OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# ******************************************************************************/

# include the version information

!include "$(OR_ORYXSRC)\oorexx.ver.incl"

# Make file include for WIN32 stuff
#
# Include the WIN32 standard include file
# Back out if no source/output dirs set
!IFNDEF OR_OUTDIR
!ERROR Build error, OR_OUTDIR not set
!ENDIF
!IFNDEF OR_ORYXINCL
!ERROR Build error, OR_ORYXINCL not set
!ENDIF

!IF "$(REXXDEBUG)" == "1"
RXDBG=/DREXX_DEBUG
RXDBG_OBJ=$(OR_OUTDIR)\windbg.obj
!ELSE
RXDBG=
RXDBG_OBJ=
!ENDIF

# CHM - added assembly listings
!IF "$(MKASM)" == "1"
MK_ASM=/FAcs /Fa$(OR_OUTDIR)\ASM\$(@B).asm
!ELSE
MK_ASM=
!ENDIF

OR_CC=cl
#
# set up the link command
#
OR_LINK=link
#
# set up the lib command
#
OR_IMPLIB=lib

#
# set up the compile flags used in addition to the $(cflags) windows sets
#
!IF "$(NODEBUG)" == "1"
my_cdebug = -Zd -O2 /Gr /DNDEBUG /Gs #Gs added by IH
#added by IH for the NT queue pull problem
cflags_noopt=/nologo /D:_X86_ /DWIN32 /W3 -c /Ox /Gf /Gr /DNDEBUG /Gs /DNULL=0
!ELSE
my_cdebug = -Zi /Od /Gr /D_DEBUG /DEBUGTYPE:CV
#added by IH for the NT queue pull problem
cflags_noopt=/nologo /D:_X86_ /DWIN32 /W3 -c $(my_cdebug) /DNULL=0
!ENDIF

# CHM - added definition for RXDBG
cflags_common=/nologo -DORX_VER=$(ORX_MAJOR) -DORX_REL=$(ORX_MINOR) -DORX_MOD=$(ORX_MOD_LVL) -DOOREXX_BLD=$(ORX_BLD_LVL)  -DOOREXX_COPY_YEAR=\"$(ORX_COPY_YEAR)\" /D:_X86_ /DWIN32 /W3 -c $(my_cdebug) $(MK_ASM) $(RXDBG) /DNULL=0 /D_CRT_SECURE_NO_DEPRECATE /D_CRT_NONSTDC_NO_DEPRECATE

# ENG - added for feature 953
!IFDEF JAPANESE
cflags_common = $(cflags_common) /DJAPANESE
!ENDIF

# Toronto does not want to use MSVCRT20.DLL so we need a statically linked rexx
!IFDEF NOCRTDLL
cflags_dll=/MT   #MTd if runtime debug
!ELSE
cflags_dll=/MDd   #MDd if runtime debug
!ENDIF
cflags_exe=

!IFDEF CPLUSPLUS
Tp=/Tp
!ELSE
Tp=
!ENDIF

#
# set up the Link flags used in addition to the $(cflags) windows sets
#
!IF "$(NODEBUG)" == "1"
my_ldebug =
!ELSE
my_ldebug = /PROFILE /DEBUG -debugtype:cv
!ENDIF

lflags_common= /MAP /NOLOGO $(my_ldebug) /SUBSYSTEM:Windows $(lflags_lib) user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib delayimp.lib ole32.lib oleaut32.lib uuid.lib shell32.lib kernel32.lib
lflags_common_console= /MAP /NOLOGO $(my_ldebug) /SUBSYSTEM:Console $(lflags_lib) user32.lib comdlg32.lib gdi32.lib kernel32.lib
lflags_dll = /DLL -entry:_DllMainCRTStartup@12
lflags_exe =
#
# set up the Lib flags used
#
libs_dll=
libs_exe=

#
# set up the rc flags used
#
rcflags_common=rc /DWIN32 -dOOREXX_VER=$(ORX_MAJOR) -dOOREXX_REL=$(ORX_MINOR) -dOOREXX_SUB=$(ORX_MOD_LVL) -dOOREXX_BLD=$(ORX_BLD_LVL) -dOOREXX_VER_STR=\"$(ORX_VER_STR)\" -dOOREXX_COPY_YEAR=\"$(ORX_COPY_YEAR)\"

# CHM - define dependency for WINDBG.OBJ
!IF "$(REXXDEBUG)" == "1"
$(RXDBG_OBJ): $(OR_ORYXKSRC)\windbg.c
    @ECHO .
    @ECHO Compiling WINDBG.c
    $(OR_CC) $(cflags_common) $(cflags_dll) /Fo$(OR_OUTDIR)\$(@B).obj $(OR_ORYXINCL) $(Tp) $(OR_ORYXKSRC)\$(@B).c
!ENDIF
