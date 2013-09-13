#/*----------------------------------------------------------------------------*/
#/*                                                                            */
#/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
#/* Copyright (c) 2005-2013 Rexx Language Association. All rights reserved.    */
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

# NOTE:  /OPT:REF in linker flags eliminates unreferenced functions and data.
#        Need to use /Gy when compiling to use /OPT:REF.

# NMAKE-compatible MAKE file for ooDialog

# If OOD_INDEPENDENT is defined we are operating outside of the interpreter
# build.  In that case, OOD_OUTDIR, OOD_OODIALOGSRC, and OOD_INCLUDE_File are
# defined in the parent make file that calls us.  When called in the interpreter
# build process, we set those macros using the OR_xx values set by the build
# batch files.

!ifdef OOD_INDEPENDENT

REXXAPI_LIBS = $(REXX_LIBS)

# Generate the version information.  Quit if there is an error.  We only do this
# when we are building outside of the interpreter build.  Within the interpreter
# build, the ooDialog source will be the latest released version of ooDialog.
# That released version will have a static ooDialog.ver.incl file that reflects
# the exact version at the time of release.
!IF [generateVersionFile.bat] != 0
!  ERROR Build error: could not gerate version file, ooDialog.ver.incl
!ENDIF

!else

OOD_OUTDIR=$(OR_OUTDIR)
OOD_OODIALOGSRC=$(OR_OODIALOGSRC)
REXXAPI_LIBS = $(OR_OUTDIR)\rexx.lib $(OR_OUTDIR)\rexxapi.lib
OOD_INCLUDE_FILE = "$(OR_LIBSRC)\ORXWIN32.MAK"

!endif

!include ooDialog.ver.incl
!include $(OOD_INCLUDE_FILE)

# The ooDialog specific version definition
ood_ver_def = -DOOD_VER=$(OOD_MAJOR) -DOOD_REL=$(OOD_MINOR) -DOOD_MOD=$(OOD_MOD_LVL) -DOOD_BLD=$(OOD_BLD_LVL) -DOOD_COPY_YEAR=\"$(OOD_COPY_YEAR)\"

# We use our own rc flags version.
rcflags_oodialog = rc /DWIN32 -dOODIALOG_VER=$(OOD_MAJOR) -dOODIALOG_REL=$(OOD_MINOR) -dOODIALOG_SUB=$(OOD_MOD_LVL) -dOODIALOG_BLD=$(OOD_BLD_LVL) -dOODIALOG_VER_STR=\"$(OOD_VER_STR)\" -dOODIALOG_COPY_YEAR=\"$(OOD_COPY_YEAR)\" -dMANIFEST_FILE=$(M_FILE)

C=cl
OPTIONS= $(cflags_common) $(ood_ver_def) $(cflags_dll) $(OR_ORYXINCL)

all:  $(OOD_OUTDIR)\oodialog.dll

# All Source Files
SOURCEF= $(OOD_OUTDIR)\APICommon.obj               $(OOD_OUTDIR)\oodBarControls.obj      $(OOD_OUTDIR)\oodBaseDialog.obj    \
         $(OOD_OUTDIR)\oodBasicControls.obj        $(OOD_OUTDIR)\oodCommon.obj           $(OOD_OUTDIR)\oodControl.obj       \
         $(OOD_OUTDIR)\oodData.obj                 $(OOD_OUTDIR)\oodDeviceGraphics.obj   $(OOD_OUTDIR)\ooDialog.obj         \
         $(OOD_OUTDIR)\oodKeyboard.obj             $(OOD_OUTDIR)\oodListView.obj         $(OOD_OUTDIR)\oodMenu.obj          \
         $(OOD_OUTDIR)\oodMessaging.obj            $(OOD_OUTDIR)\oodMouse.obj            $(OOD_OUTDIR)\oodPackageEntry.obj  \
         $(OOD_OUTDIR)\oodPropertySheetDialog.obj  $(OOD_OUTDIR)\oodResizableDialog.obj  $(OOD_OUTDIR)\oodResources.obj     \
         $(OOD_OUTDIR)\oodRoutines.obj             $(OOD_OUTDIR)\oodShellObjects.obj     $(OOD_OUTDIR)\oodToolTip.obj       \
         $(OOD_OUTDIR)\oodTreeView.obj             $(OOD_OUTDIR)\oodUser.obj             $(OOD_OUTDIR)\oodUtilities.obj     \
         $(OOD_OUTDIR)\oodViewControls.obj         $(OOD_OUTDIR)\oodialog.res

# All Source files that include APICommon.hpp
APICOMMON_SOURCEF = $(OOD_OUTDIR)\APICommon.obj              $(OOD_OUTDIR)\oodBarControls.obj     $(OOD_OUTDIR)\oodBaseDialog.obj    \
                    $(OOD_OUTDIR)\oodBasicControls.obj       $(OOD_OUTDIR)\oodCommon.obj          $(OOD_OUTDIR)\oodControl.obj       \
                    $(OOD_OUTDIR)\oodData.obj                $(OOD_OUTDIR)\oodDeviceGraphics.obj  $(OOD_OUTDIR)\ooDialog.obj         \
                    $(OOD_OUTDIR)\oodKeyboard.obj            $(OOD_OUTDIR)\oodListView.obj        $(OOD_OUTDIR)\oodMenu.obj          \
                    $(OOD_OUTDIR)\oodMessaging.obj           $(OOD_OUTDIR)\oodMouse.obj           $(OOD_OUTDIR)\oodPackageEntry.obj  \
                    $(OOD_OUTDIR)\oodPropertySheetDialog.obj $(OOD_OUTDIR)\oodResizableDialog.obj $(OOD_OUTDIR)\oodResources.obj     \
                    $(OOD_OUTDIR)\oodRoutines.obj            $(OOD_OUTDIR)\oodShellObjects.obj    $(OOD_OUTDIR)\oodToolTip.obj       \
                    $(OOD_OUTDIR)\oodTreeView.obj            $(OOD_OUTDIR)\oodUser.obj            $(OOD_OUTDIR)\oodUtilities.obj     \
                    $(OOD_OUTDIR)\oodViewControls.obj

# All Source files that include oodCommon.hpp
COMMON_SOURCEF = $(OOD_OUTDIR)\oodBarControls.obj        $(OOD_OUTDIR)\oodBaseDialog.obj   $(OOD_OUTDIR)\oodBasicControls.obj       \
                 $(OOD_OUTDIR)\oodCommon.obj             $(OOD_OUTDIR)\oodData.obj         $(OOD_OUTDIR)\oodDeviceGraphics.obj      \
                 $(OOD_OUTDIR)\oodMenu.obj               $(OOD_OUTDIR)\oodMessaging.obj    $(OOD_OUTDIR)\oodPropertySheetDialog.obj \
                 $(OOD_OUTDIR)\oodResizableDialog.obj    $(OOD_OUTDIR)\oodRoutines.obj     $(OOD_OUTDIR)\oodShellObjects.obj        \
                 $(OOD_OUTDIR)\oodUser.obj               $(OOD_OUTDIR)\oodUtilities.obj    $(OOD_OUTDIR)\oodViewControls.obj

# All Source files that include oodDeviceGraphics.hpp
OODDEVICEGRAPHICS_SOURCEF = \
                 $(OOD_OUTDIR)\oodBaseDialog.obj         $(OOD_OUTDIR)\oodControl.cpp   $(OOD_OUTDIR)\ooDeviceGraphics.cpp       \
                 $(OOD_OUTDIR)\ooDialog.cpp              $(OOD_OUTDIR)\oodMessaging.obj $(OOD_OUTDIR)\oodPropertySheetDialog.obj \
                 $(OOD_OUTDIR)\oodResizableDialog.obj    $(OOD_OUTDIR)\oodRoutines.obj  $(OOD_OUTDIR)\oodUser.obj                \
                 $(OOD_OUTDIR)\oodUtilities.obj

# All Source files that include oodData.hpp
OODDATA_SOURCEF = $(OOD_OUTDIR)\oodBaseDialog.obj  $(OOD_OUTDIR)\ooDialog.cpp               $(OOD_OUTDIR)\oodData.obj            \
                  $(OOD_OUTDIR)\oodMessaging.obj   $(OOD_OUTDIR)\oodPropertySheetDialog.obj $(OOD_OUTDIR)\oodUser.obj

# All Source files that include oodControl.hpp
OODCONTROL_SOURCEF = $(OOD_OUTDIR)\oodBarControls.obj $(OOD_OUTDIR)\oodBaseDialog.obj          $(OOD_OUTDIR)\oodBasicControls.obj   \
                     $(OOD_OUTDIR)\oodControl.obj     $(OOD_OUTDIR)\oodData.obj                $(OOD_OUTDIR)\oodDevicGraphics.obj   \
                     $(OOD_OUTDIR)\ooDialog.cpp       $(OOD_OUTDIR)\oodPropertySheetDialog.obj $(OOD_OUTDIR)\oodResizableDialog.obj \
                     $(OOD_OUTDIR)\oodUser.obj        $(OOD_OUTDIR)\oodViewControls.obj

# All Source files that include oodMessaging.hpp
OODMESSAGING_SOURCEF = \
                 $(OOD_OUTDIR)\oodBaseDialog.obj     $(OOD_OUTDIR)\oodBasicControls.obj       $(OOD_OUTDIR)\oodControl.obj \
                 $(OOD_OUTDIR)\oodDeviceGraphics.obj $(OOD_OUTDIR)\ooDialog.obj               $(OOD_OUTDIR)\oodMenu.obj    \
                 $(OOD_OUTDIR)\oodMessaging.obj      $(OOD_OUTDIR)\oodPropertySheetDialog.obj $(OOD_OUTDIR)\oodResizableDialog.obj \
                 $(OOD_OUTDIR)\oodUser.obj           $(OOD_OUTDIR)\oodViewControls.obj

# All Source files that include oodResizableDialog.hpp
OODRESIZABLEDIALOG_SOURCEF = \
                 $(OOD_OUTDIR)\oodBaseDialog.obj     $(OOD_OUTDIR)\ooDialog.cpp               $(OOD_OUTDIR)\oodResizable.obj \
                 $(OOD_OUTDIR)\oodUser.obj

# All Source files that include oodResources.hpp
OODRESOURCES_SOURCEF = \
                 $(OOD_OUTDIR)\oodBasicControls.obj  $(OOD_OUTDIR)\oodPropertySheetDialog.obj $(OOD_OUTDIR)\oodResources.obj \
                 $(OOD_OUTDIR)\oodViewControls.obj

# All Source files that include oodUser.hpp
OODUSER_SOURCEF = $(OOD_OUTDIR)\oodPropertySheetDialog.obj $(OOD_OUTDIR)\oodUser.obj



.c{$(OOD_OUTDIR)}.obj:
    $(C) $(OPTIONS)  /DINCL_32  -c $(@B).c /Fo$(OOD_OUTDIR)\$(@B).obj

#
# *** .cpp -> .obj rules
#
{$(OOD_OODIALOGSRC)}.cpp{$(OOD_OUTDIR)}.obj:
    @ECHO .
    @ECHO Compiling $(@B).cpp
    $(OR_CC) $(cflags_common) $(ood_ver_def) $(cflags_dll) /Fo$(OOD_OUTDIR)\$(@B).obj $(OR_ORYXINCL)  $(OOD_OODIALOGSRC)\$(@B).cpp


{$(OOD_OODIALOGSRC)}.c{$(OOD_OUTDIR)}.obj:
    @ECHO .
    @ECHO Compiling $(@B).c
    $(OR_CC) $(cflags_common) $(ood_ver_def)  $(cflags_dll) /Fo$(OOD_OUTDIR)\$(@B).obj $(OR_ORYXINCL)  $(OOD_OODIALOGSRC)\$(@B).c


$(OOD_OUTDIR)\oodialog.dll: $(SOURCEF)
    $(OR_LINK) \
    $(SOURCEF)  \
    $(lflags_common) $(lflags_dll) \
    $(REXXAPI_LIBS) \
    WINMM.LIB \
    COMDLG32.LIB \
    COMCTL32.LIB \
    shlwapi.lib \
    Oleacc.lib \
    UxTheme.lib \
    -def:$(OOD_OODIALOGSRC)\ooDialog.def \
    -out:$(OOD_OUTDIR)\$(@B).dll


# Update the version information block
$(OOD_OUTDIR)\oodialog.res: $(OOD_OODIALOGSRC)\oodialog.rc
    @ECHO .
    @ECHO ResourceCompiling $(@B).res
        $(rc) $(rcflags_oodialog) /i $(OOD_OODIALOGSRC) /i $(OR_WINKERNELSRC) -r -fo$(OOD_OUTDIR)\$(@B).res $(OOD_OODIALOGSRC)\$(@B).rc

# Recompile everything if the make file changes.
$(SOURCEF) : oodialog.mak

# Source .obj files that should be recompiled when header file(s) change.
$(SOURCEF) : ooDialog.hpp
$(COMMON_SOURCEF) : oodCommon.hpp
$(APICOMMON_SOURCEF) : APICommon.hpp
$(OODDEVICEGRAPHICS_SOURCEF) : oodDeviceGraphics.hpp
$(OODDATA_SOURCEF) : oodData.hpp
$(OODCONTROL_SOURCEF) : oodControl.hpp
$(OODMESSAGING_SOURCEF) : oodMessaging.hpp
$(OODRESIZABLEDIALOG_SOURCEF) : oodResizableDialog.hpp
$(OODRESOURCES_SOURCEF) : oodResources.hpp
$(OODUSER_SOURCEF) : oodUser.hpp
$(OOD_OUTDIR)\oodMenu.obj : oodMenu.hpp
$(OOD_OUTDIR)\oodShellObjects.obj : oodShellObjects.hpp
$(OOD_OUTDIR)\oodUtilities.obj : oodKeyNames.hpp
