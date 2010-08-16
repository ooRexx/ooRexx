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

# NOTE: If for some reason there is a problem with setting the compiler version
# in this section, simply comment out everything except the setting of the
# proper VCPPx macro for your system.
#
# These compiler defines allow the support of different versions of Mircrosoft's
# Visual C++ compiler.  The build has not necessarily been tested on all of the
# following versions, so some of the !IFDEF statements may need to be adjusted.
# VCPP9 == Visual C++ 2008
# VCPP8 == Visual C++ 2005
# VCPP7 == Visual C++ 2003
# VCPP6 == Visual C++ 6.0
#
!IF "$(MSVCVER)" == "9.0"
VCPP9 = 1
!ELSEIF "$(MSVCVER)" == "8.0"
VCPP8 = 1
!ELSEIF "$(MSVCVER)" == "7.0"
VCPP7 = 1
!ELSEIF "$(MSVCVER)" == "6.0"
VCPP6 = 1
!ELSE
!ERROR MSVCVER does not appear to be set. Check windows-build.txt for details
!ENDIF

# include the version information
!include "$(OR_MAINSRC)\oorexx.ver.incl"

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

# The ooRexx version definition for the compile flags.
VER_DEF = -DORX_VER=$(ORX_MAJOR) -DORX_REL=$(ORX_MINOR) -DORX_MOD=$(ORX_MOD_LVL) -DOOREXX_BLD=$(ORX_BLD_LVL) -DOOREXX_COPY_YEAR=\"$(ORX_COPY_YEAR)\"

# The start of the warning flags for the compile flags.
WARNING_FLAGS = /D_CRT_SECURE_NO_DEPRECATE /D_CRT_NONSTDC_NO_DEPRECATE

# Visual C++ 6.0 chokes on /Wp64 and in Visual C++ 9.0 the flag is deprecated
!IFDEF VCPP7
WARNING_FLAGS = /Wp64 $(WARNING_FLAGS)
!ELSE IFDEF VCPP8
WARNING_FLAGS = /Wp64 $(WARNING_FLAGS)
!ENDIF

# Turn on extra warnings by defining EXTRAWARNINGS to 1.
#
# /W4 gives a lint-like level of warnings.
# /wd<number> turns off warning 'number'  /wd4100 then turns off warning C4100.
#
# Uncomment the following, or alternatively set EXTRAWARNINGS as an environment
# variable.
# EXTRAWARNINGS = 1
!IF "$(EXTRAWARNINGS)" == "1"
WARNING_FLAGS = /W4 /wd4100 /wd4706 /wd4701 $(WARNING_FLAGS)
!ELSE
WARNING_FLAGS = /W3 $(WARNING_FLAGS)
!ENDIF

!IFDEF VCPP9
Z_FLAGS =
!ELSE IFDEF VCPP8
Z_FLAGS =
!ELSE
Z_FLAGS = -Zd
!ENDIF

#
# set up the compile flags used in addition to the $(cflags) windows sets
#
!IF "$(NODEBUG)" == "1"
my_cdebug = $(Z_FLAGS) -O2 /Gr /DNDEBUG /Gs #Gs added by IH
#added by IH for the NT queue pull problem
cflags_noopt=/nologo /D:_X86_ /DWIN32 $(WARNING_FLAGS) -c /Ox /Gf /Gr /DNDEBUG /Gs /DNULL=0
!ELSE
my_cdebug = -Zi /Od /Gr /D_DEBUG /DEBUGTYPE:CV
#added by IH for the NT queue pull problem
cflags_noopt=/nologo /D:_X86_ /DWIN32 $(WARNING_FLAGS) -c $(my_cdebug) /DNULL=0
!ENDIF

cflags_common=/EHsc /nologo /D:_X86_ /DWIN32 $(VER_DEF) $(WARNING_FLAGS) -c $(my_cdebug) $(MK_ASM) $(RXDBG) /DNULL=0

# ooRexx has always been using a statically linked CRT.
!IFDEF NOCRTDLL
# statically linked rexx
!IF "$(NODEBUG)" == "1"
cflags_dll=/MT
!ELSE
cflags_dll=/MTd
!ENDIF
!ELSE
!IF "$(NODEBUG)" == "1"
cflags_dll=/MD
!ELSE
cflags_dll=/MDd
!ENDIF
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

libs_common=user32.lib gdi32.lib winspool.lib comdlg32.lib comctl32.lib advapi32.lib delayimp.lib ole32.lib oleaut32.lib uuid.lib shell32.lib kernel32.lib

lflags_common= /MAP /NOLOGO $(my_ldebug) /SUBSYSTEM:Windows $(lflags_lib) $(libs_common)
lflags_common_console= /MAP /NOLOGO $(my_ldebug) /SUBSYSTEM:Console $(lflags_lib) user32.lib comdlg32.lib gdi32.lib kernel32.lib
lflags_dll = /DLL
lflags_exe =

#
# set up the rc flags used
#
!IF "$(CPU)" == "X64"
M_FILE = "rexx64.exe.manifest"
!ELSE
M_FILE = "rexx32.exe.manifest"
!ENDIF
rcflags_common=rc /DWIN32 -dOOREXX_VER=$(ORX_MAJOR) -dOOREXX_REL=$(ORX_MINOR) -dOOREXX_SUB=$(ORX_MOD_LVL) -dOOREXX_BLD=$(ORX_BLD_LVL) -dOOREXX_VER_STR=\"$(ORX_VER_STR)\" -dOOREXX_COPY_YEAR=\"$(ORX_COPY_YEAR)\" -dMANIFEST_FILE=$(M_FILE)

#
# *** Inference Rule for CPP->OBJ
# *** For .CPP files in OR_LIBSRC directory
#
{$(OR_COMMONPLATFORMSRC)}.cpp{$(OR_OUTDIR)}.obj:
    @ECHO .
    @ECHO Compiling $(**)
    $(OR_CC) $(cflags_common) $(cflags_dll)  /Fo$(@) $(OR_ORYXINCL) $(Tp)$(**)

#
# *** Inference Rule for CPP->OBJ
# *** For .CPP files in OR_LIBSRC directory
#
{$(OR_COMMONSRC)}.cpp{$(OR_OUTDIR)}.obj:
    @ECHO .
    @ECHO Compiling $(**)
    $(OR_CC) $(cflags_common) $(cflags_dll)  /Fo$(@) $(OR_ORYXINCL) $(Tp)$(**)

