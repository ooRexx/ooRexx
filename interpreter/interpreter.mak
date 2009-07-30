# *******************************************************************************
#
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
#
# *******************************************************************************
#------------------------
# INTERPRETER.MAK make file
#------------------------


# -------------------------------------------------------------------------
# Main (default) target:
# -------------------------------------------------------------------------
# CHM moved target definition to top
all : ORXHEADERS $(OR_OUTDIR)\rexx.dll  \
      COPYORXFILES COPYAPIFILES
    @ECHO ...
    @ECHO All done ....

# Include compiler specific macro definitions
!include "$(OR_LIBSRC)\ORXWIN32.MAK"

# Check for oryxk source variable set
# This is where the source files are....
!IFNDEF OR_INTERPRETER_SRC
!ERROR Build error, OR_INTERPRETER_SRC not set
!ENDIF

# Check for lib source path
!IFNDEF OR_LIBSRC
!ERROR Build error, OR_LIBSRC not set
!ENDIF


# -------------------------------------------------------------------------
# Object list:
# -------------------------------------------------------------------------
# Add .orx file to list of possible inference rule targets
.SUFFIXES: .orx .rex .cmd .cls
OBJ = obj


# Following all part of rexx
OTSOBJ1=$(OR_OUTDIR)\BuiltinFunctions.$(OBJ)   $(OR_OUTDIR)\DoBlock.$(OBJ) $(OR_OUTDIR)\Clause.$(OBJ) \
        $(OR_OUTDIR)\RexxInstruction.$(OBJ) $(OR_OUTDIR)\CommonExternalFunctions.$(OBJ)
OTSOBJ2=$(OR_OUTDIR)\SourceFile.$(OBJ) $(OR_OUTDIR)\ExpressionStack.$(OBJ) $(OR_OUTDIR)\Token.$(OBJ)
OTIOBJ1=$(OR_OUTDIR)\AddressInstruction.$(OBJ)  $(OR_OUTDIR)\AssignmentInstruction.$(OBJ) $(OR_OUTDIR)\CallInstruction.$(OBJ) \
        $(OR_OUTDIR)\CommandInstruction.$(OBJ)
OTIOBJ2=$(OR_OUTDIR)\DoInstruction.$(OBJ)    $(OR_OUTDIR)\DropInstruction.$(OBJ)
OTIOBJ3=$(OR_OUTDIR)\ElseInstruction.$(OBJ)  $(OR_OUTDIR)\EndInstruction.$(OBJ)   $(OR_OUTDIR)\EndIf.$(OBJ)
OTIOBJ4=$(OR_OUTDIR)\ExitInstruction.$(OBJ)  $(OR_OUTDIR)\ExposeInstruction.$(OBJ) $(OR_OUTDIR)\ForwardInstruction.$(OBJ) \
        $(OR_OUTDIR)\GuardInstruction.$(OBJ)
OTIOBJ5=$(OR_OUTDIR)\IfInstruction.$(OBJ)    $(OR_OUTDIR)\InterpretInstruction.$(OBJ)   $(OR_OUTDIR)\LabelInstruction.$(OBJ)
OTIOBJ6=$(OR_OUTDIR)\LeaveInstruction.$(OBJ) $(OR_OUTDIR)\MessageInstruction.$(OBJ)   $(OR_OUTDIR)\NopInstruction.$(OBJ) \
        $(OR_OUTDIR)\NumericInstruction.$(OBJ)
OTIOBJ7=$(OR_OUTDIR)\OptionsInstruction.$(OBJ) $(OR_OUTDIR)\OtherwiseInstruction.$(OBJ) $(OR_OUTDIR)\ParseInstruction.$(OBJ) \
        $(OR_OUTDIR)\ProcedureInstruction.$(OBJ)

OTIOBJ8=$(OR_OUTDIR)\QueueInstruction.$(OBJ) $(OR_OUTDIR)\RaiseInstruction.$(OBJ) \
	$(OR_OUTDIR)\RequiresDirective.$(OBJ) $(OR_OUTDIR)\LibraryDirective.$(OBJ)  $(OR_OUTDIR)\ClassDirective.$(OBJ)
OTIOBJ9=$(OR_OUTDIR)\ReplyInstruction.$(OBJ) $(OR_OUTDIR)\ReturnInstruction.$(OBJ)   $(OR_OUTDIR)\SayInstruction.$(OBJ) \
        $(OR_OUTDIR)\SelectInstruction.$(OBJ)
OTIOBJ10=$(OR_OUTDIR)\SignalInstruction.$(OBJ) $(OR_OUTDIR)\ThenInstruction.$(OBJ) $(OR_OUTDIR)\TraceInstruction.$(OBJ) \
         $(OR_OUTDIR)\UseStrictInstruction.$(OBJ)
OTEOBJ1=$(OR_OUTDIR)\ExpressionCompoundVariable.$(OBJ)  $(OR_OUTDIR)\ExpressionDotVariable.$(OBJ)  $(OR_OUTDIR)\ExpressionFunction.$(OBJ) \
        $(OR_OUTDIR)\ExpressionMessage.$(OBJ) $(OR_OUTDIR)\ExpressionLogical.$(OBJ)
OTEOBJ2=$(OR_OUTDIR)\ExpressionStem.$(OBJ)  $(OR_OUTDIR)\ExpressionVariable.$(OBJ)   $(OR_OUTDIR)\IndirectVariableReference.$(OBJ) \
        $(OR_OUTDIR)\ExpressionOperator.$(OBJ)
OTEOBJ3=$(OR_OUTDIR)\ParseTarget.$(OBJ) $(OR_OUTDIR)\ParseTrigger.$(OBJ) $(OR_OUTDIR)\RexxInternalStack.$(OBJ) \
        $(OR_OUTDIR)\RexxLocalVariables.$(OBJ) $(OR_OUTDIR)\RexxActivationStack.$(OBJ) $(OR_OUTDIR)\ProtectedObject.$(OBJ) \
	$(OR_OUTDIR)\ExitHandler.$(OBJ) $(OR_OUTDIR)\CommandHandler.$(OBJ)
OTPOBJS=$(OTSOBJ1)  $(OTSOBJ2) $(OTIOBJ1) $(OTIOBJ2) $(OTIOBJ3) \
        $(OTIOBJ4)  $(OTIOBJ5) $(OTIOBJ6) $(OTIOBJ7) $(OTIOBJ8) $(OTIOBJ9) \
        $(OTIOBJ10) $(OTEOBJ1) $(OTEOBJ2) $(OTEOBJ3)

# Following all part of rexx
OKCOBJ1=$(OR_OUTDIR)\Version.$(OBJ)
OKCOBJ2= $(OR_OUTDIR)\Utilities.$(OBJ)
OKAOBJS= $(OR_OUTDIR)\GlobalData.$(OBJ)  $(OR_OUTDIR)\GlobalNames.$(OBJ)
OKLOBJS=$(OR_OUTDIR)\Setup.$(OBJ) $(OR_OUTDIR)\InstructionParser.$(OBJ) \
        $(OR_OUTDIR)\Scanner.$(OBJ)
OKCOBJS=$(OKCOBJ1) $(OKCOBJ2) $(OKLOBJS)

# Following all part of rexx
OKPOBJ1=$(OR_OUTDIR)\RexxEnvelope.$(OBJ) $(OR_OUTDIR)\ArrayClass.$(OBJ) $(OR_OUTDIR)\RexxMisc.$(OBJ)   \
        $(OR_OUTDIR)\ClassClass.$(OBJ) $(OR_OUTDIR)\DeadObject.$(OBJ) $(OR_OUTDIR)\PointerClass.$(OBJ) \
	$(OR_OUTDIR)\WeakReferenceClass.$(OBJ)
OKPOBJ2=$(OR_OUTDIR)\DirectoryClass.$(OBJ) $(OR_OUTDIR)\MethodClass.$(OBJ)  $(OR_OUTDIR)\RoutineClass.$(OBJ) \
	 $(OR_OUTDIR)\PackageClass.$(OBJ) $(OR_OUTDIR)\ContextClass.$(OBJ)
OKPOBJ3=$(OR_OUTDIR)\ListClass.$(OBJ)   $(OR_OUTDIR)\RexxMemory.$(OBJ) $(OR_OUTDIR)\MemorySegment.$(OBJ) \
         $(OR_OUTDIR)\MemoryStats.$(OBJ) $(OR_OUTDIR)\MessageClass.$(OBJ)    \
        $(OR_OUTDIR)\StemClass.$(OBJ)   $(OR_OUTDIR)\ObjectClass.$(OBJ) $(OR_OUTDIR)\RexxCompoundTail.$(OBJ) \
        $(OR_OUTDIR)\RexxCompoundElement.$(OBJ) $(OR_OUTDIR)\RexxCompoundTable.$(OBJ)
OKPOBJ4=$(OR_OUTDIR)\QueueClass.$(OBJ)  $(OR_OUTDIR)\SupplierClass.$(OBJ) $(OR_OUTDIR)\RexxQueueMethods.$(OBJ) \
        $(OR_OUTDIR)\RelationClass.$(OBJ)  $(OR_OUTDIR)\TableClass.$(OBJ) $(OR_OUTDIR)\IdentityTableClass.$(OBJ)\
	$(OR_OUTDIR)\PrimitiveBehaviours.$(OBJ) $(OR_OUTDIR)\VirtualFunctionTable.$(OBJ)
OKPOBJ5=$(OR_OUTDIR)\IntegerClass.$(OBJ)    $(OR_OUTDIR)\NumberStringClass.$(OBJ)
OKIOBJ1=$(OR_OUTDIR)\RexxActivation.$(OBJ) $(OR_OUTDIR)\RexxActivity.$(OBJ) $(OR_OUTDIR)\KeywordConstants.$(OBJ)  \
        $(OR_OUTDIR)\RexxBehaviour.$(OBJ)  $(OR_OUTDIR)\BufferClass.$(OBJ) $(OR_OUTDIR)\ActivityManager.$(OBJ) \
	$(OR_OUTDIR)\Interpreter.$(OBJ) $(OR_OUTDIR)\SystemInterpreter.$(OBJ) $(OR_OUTDIR)\RexxStartDispatcher.$(OBJ) \
	$(OR_OUTDIR)\InterpreterInstance.$(OBJ) $(OR_OUTDIR)\ActivityDispatcher.$(OBJ) $(OR_OUTDIR)\TranslateDispatcher.$(OBJ) \
	$(OR_OUTDIR)\CallbackDispatcher.$(OBJ) $(OR_OUTDIR)\SecurityManager.$(OBJ) \
	$(OR_OUTDIR)\MessageDispatcher.$(OBJ) $(OR_OUTDIR)\SysInterpreterInstance.$(OBJ)
OKIOBJ2=$(OR_OUTDIR)\RexxHashTable.$(OBJ)  $(OR_OUTDIR)\RexxCode.$(OBJ) $(OR_OUTDIR)\PackageManager.$(OBJ) \
        $(OR_OUTDIR)\RexxListTable.$(OBJ) $(OR_OUTDIR)\RexxNativeActivation.$(OBJ) $(OR_OUTDIR)\RexxNativeCode.$(OBJ) \
	$(OR_OUTDIR)\CPPCode.$(OBJ) $(OR_OUTDIR)\LibraryPackage.$(OBJ) $(OR_OUTDIR)\InternalPackage.$(OBJ)
OKIOBJ3=$(OR_OUTDIR)\RexxCollection.$(OBJ)   $(OR_OUTDIR)\RexxSmartBuffer.$(OBJ) $(OR_OUTDIR)\StackClass.$(OBJ)  \
        $(OR_OUTDIR)\RexxVariable.$(OBJ)    $(OR_OUTDIR)\RexxVariableDictionary.$(OBJ) $(OR_OUTDIR)\RexxDateTime.$(OBJ) \
	$(OR_OUTDIR)\Numerics.$(OBJ) $(OR_OUTDIR)\CallContextStubs.$(OBJ) $(OR_OUTDIR)\InterpreterAPI.$(OBJ) \
	$(OR_OUTDIR)\InterpreterInstanceStubs.$(OBJ) $(OR_OUTDIR)\MethodContextStubs.$(OBJ) $(OR_OUTDIR)\ThreadContextStubs.$(OBJ)

OKPOBJS=$(OKPOBJ1) $(OKPOBJ2) $(OKPOBJ3) $(OKPOBJ4) $(OKPOBJ5)
OKIOBJS=$(OKIOBJ1) $(OKIOBJ2) $(OKIOBJ3)
#part of rexx

SYSOBJ1=$(OR_OUTDIR)\TimeSupport.$(OBJ)  \
        $(OR_OUTDIR)\FileSystem.$(OBJ)  $(OR_OUTDIR)\ValueFunction.$(OBJ) $(OR_OUTDIR)\UseridFunction.$(OBJ)

SYSOBJ2=$(OR_OUTDIR)\ExternalFunctions.$(OBJ) $(OR_OUTDIR)\SystemCommands.$(OBJ)   \
        $(OR_OUTDIR)\StreamNative.$(OBJ)   $(OR_OUTDIR)\StreamCommandParser.$(OBJ)    $(OR_OUTDIR)\ProgramMetaData.$(OBJ) \
	$(OR_OUTDIR)\SysFile.$(OBJ) $(OR_OUTDIR)\SysFileSystem.$(OBJ) $(OR_OUTDIR)\SysLibrary.$(OBJ) $(OR_OUTDIR)\SysActivity.$(OBJ) \
        $(OR_OUTDIR)\SysSemaphore.$(OBJ) $(OR_OUTDIR)\FileNative.$(OBJ)

SYSOBJ3=$(OR_OUTDIR)\MemorySupport.$(OBJ)   $(OR_OUTDIR)\MiscSystem.$(OBJ)  $(OR_OUTDIR)\SystemInitialization.$(OBJ)

SYSOBJS=$(SYSOBJ1) $(SYSOBJ2) $(SYSOBJ3)


#part of rexx
OEPOBJS=$(OR_OUTDIR)\NumberStringMath.$(OBJ)   $(OR_OUTDIR)\NumberStringMath2.$(OBJ)

#part of rexx
OKSOBJS=$(OR_OUTDIR)\StringClass.$(OBJ) $(OR_OUTDIR)\StringClassUtil.$(OBJ) $(OR_OUTDIR)\StringClassSub.$(OBJ)   \
        $(OR_OUTDIR)\StringClassWord.$(OBJ) $(OR_OUTDIR)\StringClassMisc.$(OBJ) $(OR_OUTDIR)\StringClassBit.$(OBJ)    \
        $(OR_OUTDIR)\StringClassConversion.$(OBJ) $(OR_OUTDIR)\MutableBufferClass.$(OBJ) $(OR_OUTDIR)\StringUtil.$(OBJ)

SYSERR= $(OR_OUTDIR)\ErrorMessages.$(OBJ)

# rexx
ORYXKOBJ= $(OKCOBJS) $(OKAOBJS) $(OKPOBJS) $(OKIOBJS) $(OKSOBJS) $(OEPOBJS)\
          $(OTPOBJS) $(SYSOBJS) $(SYSERR)

#    Windows Universal Thunk 32-bit stub
#    16-bit side must be built with 16-bit compiler
#    Also, you will need to get some components from the \win32s\ut
#    directory from your VC++32 CD-Rom.  In particular the W32sUT.h needs
#    to go in msvc20\h (and also in a directory in the INCLUDE path for
#    our Win16 development environment.)  W32sUT32.Lib needs to go in
#    msvc20\h and W32sUT16.Lib needs to go in a directory in the LIB
#    path for our Win16 development environment.
#
# SYSUT32OBJ = $(OR_OUTDIR)\rxcmd32.$(OBJ)

# define files copied by the make to the test directory
ORXFILES=$(OR_OUTDIR)\CoreClasses.orx  $(OR_OUTDIR)\StreamClasses.orx \
	 $(OR_OUTDIR)\PlatformObjects.orx $(OR_OUTDIR)\orexxole.cls

#define critical header files for forcing recomp
ORXHEADERS=$(OR_APISRC)\oorexxerrors.h $(INTERPRETER_MESSAGES)\RexxErrorCodes.h $(INTERPRETER_MESSAGES)\RexxMessageNumbers.h $(INTERPRETER_MESSAGES)\RexxMessageTable.h $(INTERPRETER_RUNTIME)\RexxCore.h \
    $(BEHAVIOUR)\PrimitiveBehaviourNames.h $(BEHAVIOUR)\ClassTypeCodes.h

# All the files needed to compile and link an external program using the native
# API are copied to one directory to make building an external program easier.
APIFILES = $(OR_OUTDIR_API)\oorexxapi.h $(OR_OUTDIR_API)\oorexxerrors.h $(OR_OUTDIR_API)\rexx.h \
           $(OR_OUTDIR_API)\rexxapidefs.h $(OR_OUTDIR_API)\rexxapitypes.h $(OR_OUTDIR_API)\rexxplatformapis.h \
           $(OR_OUTDIR_API)\rexxplatformdefs.h $(OR_OUTDIR_API)\rexx.lib $(OR_OUTDIR_API)\rexxapi.lib


#
# *** rexx.LIB  : Creates .lib import library
#                          .exp export library for use with this link
#
# the type command creates a file of all objects as input to the lib
#
$(OR_OUTDIR)\rexx.lib : $(ORYXKOBJ)  \
                  $(INT_PLATFORM)\wrexx.def
   type <<$(OR_OUTDIR)\oryxk.lst
   $(ORYXKOBJ) $(ORYXLOBJ)
<<
        $(OR_IMPLIB)    \
        -machine:$(CPU) \
        -def:$(INT_PLATFORM)\wrexx.def \
        @$(OR_OUTDIR)\oryxk.lst \
        -out:$(OR_OUTDIR)\$(@B).lib

#
# *** rexx.DLL
#
# need import libraries and def files still
#
$(OR_OUTDIR)\rexx.dll : $(ORXHEADERS) $(ORYXKOBJ) $(ORYXLOBJ) \
                         $(OR_OUTDIR)\$(@B).lib $(INT_PLATFORM)\wrexx.def    \
                         $(OR_OUTDIR)\winmsgtb.res $(OR_OUTDIR)\verinfo.res
 type <<$(OR_OUTDIR)\oryxk.lst
   $(ORYXKOBJ) $(ORYXLOBJ)
<<
    $(OR_LINK) $(lflags_common) $(lflags_dll)  -out:$(OR_OUTDIR)\$(@B).dll \
             @$(OR_OUTDIR)\oryxk.lst \
             $(OR_OUTDIR)\winmsgtb.res \
             $(OR_OUTDIR)\$(@B).exp  \
             $(OR_OUTDIR)\rexxapi.lib

#
# *** rxcmd32.LIB  : Creates .lib import library
#                          .exp export library for use with this link
#
# the type command creates a file of all objects as input to the lib
#
$(OR_OUTDIR)\rxcmd32.lib : $(SYSUT32OBJ)  $(OR_INTERPRETER_SRC)\$(@B).def
        $(OR_IMPLIB)    \
        -machine:$(CPU) \
        -def:$(OR_INTERPRETER_SRC)\$(@B).def \
        $(SYSUT32OBJ) \
        -out:$(OR_OUTDIR)\$(@B).lib

#
# *** rxcmd32.DLL
#
# need import libraries and def files still
# w32sut32.lib needed for this Universal Thunk DLL
#
$(OR_OUTDIR)\rxcmd32.dll : $(SYSUT32OBJ) $(OR_OUTDIR)\$(@B).lib \
                           $(OR_INTERPRETER_SRC)\$(@B).def
    $(OR_LINK) $(lflags_common) $(lflags_dll)  -out:$(OR_OUTDIR)\$(@B).dll \
             $(SYSUT32OBJ) \
             $(OR_OUTDIR)\$(@B).exp  \
             w32sut32.lib


# Update the Windows Message Table resource if necessary

$(INT_PLATFORM)\winmsgtb.rc: $(INT_PLATFORM)\WinMessageResource.xsl $(INTERPRETER_MESSAGES)\rexxmsg.xml
    @ECHO .
    @ECHO Generating $(@)
    xalan -o $(@) $(INTERPRETER_MESSAGES)\rexxmsg.xml $(INT_PLATFORM)\WinMessageResource.xsl

$(INTERPRETER_MESSAGES)\RexxErrorCodes.h: $(INTERPRETER_MESSAGES)\RexxErrorCodes.xsl $(INTERPRETER_MESSAGES)\rexxmsg.xml
    @ECHO.
    @ECHO Generating $(@)
    xalan -o $(@) $(INTERPRETER_MESSAGES)\rexxmsg.xml $(INTERPRETER_MESSAGES)\RexxErrorCodes.xsl

$(INTERPRETER_MESSAGES)\DocErrorMessages.sgml: $(INTERPRETER_MESSAGES)\DocBookErrors.xsl $(INTERPRETER_MESSAGES)\rexxmsg.xml
    @ECHO.
    @ECHO Generating $(@)
    xalan -o $(@) $(INTERPRETER_MESSAGES)\rexxmsg.xml $(INTERPRETER_MESSAGES)\DocBookErrors.xsl

$(INTERPRETER_MESSAGES)\RexxMessageNumbers.h: $(INTERPRETER_MESSAGES)\RexxMessageNumbers.xsl $(INTERPRETER_MESSAGES)\rexxmsg.xml
    @ECHO.
    @ECHO Generating $(@)
    xalan -o $(@) $(INTERPRETER_MESSAGES)\rexxmsg.xml $(INTERPRETER_MESSAGES)\RexxMessageNumbers.xsl

$(INTERPRETER_MESSAGES)\RexxMessageTable.h: $(INTERPRETER_MESSAGES)\RexxMessageTable.xsl $(INTERPRETER_MESSAGES)\rexxmsg.xml
    @ECHO.
    @ECHO Generating $(@)
    xalan -o $(@) $(INTERPRETER_MESSAGES)\rexxmsg.xml $(INTERPRETER_MESSAGES)\RexxMessageTable.xsl

$(OR_APISRC)\oorexxerrors.h: $(INTERPRETER_MESSAGES)\ApiErrorCodes.xsl $(INTERPRETER_MESSAGES)\rexxmsg.xml
    @ECHO .
    @ECHO Generating $(@)
    xalan -o $(@) $(INTERPRETER_MESSAGES)\rexxmsg.xml $(INTERPRETER_MESSAGES)\ApiErrorCodes.xsl

$(BEHAVIOUR)\PrimitiveBehaviourNames.h: $(BEHAVIOUR)\PrimitiveBehaviourNames.xsl $(BEHAVIOUR)\PrimitiveClasses.xml
    @ECHO.
    @ECHO Generating $(@)
    xalan -o $(@) $(BEHAVIOUR)\PrimitiveClasses.xml $(BEHAVIOUR)\PrimitiveBehaviourNames.xsl

$(BEHAVIOUR)\PrimitiveBehaviours.cpp: $(BEHAVIOUR)\PrimitiveBehaviours.xsl $(BEHAVIOUR)\PrimitiveClasses.xml
    @ECHO.
    @ECHO Generating $(@)
    xalan -o $(@) $(BEHAVIOUR)\PrimitiveClasses.xml $(BEHAVIOUR)\PrimitiveBehaviours.xsl

$(BEHAVIOUR)\VirtualFunctionTable.cpp: $(BEHAVIOUR)\VirtualFunctionTable.xsl $(BEHAVIOUR)\PrimitiveClasses.xml
    @ECHO.
    @ECHO Generating $(@)
    xalan -o $(@) $(BEHAVIOUR)\PrimitiveClasses.xml $(BEHAVIOUR)\VirtualFunctionTable.xsl

$(BEHAVIOUR)\ClassTypeCodes.h: $(BEHAVIOUR)\ClassTypeCodes.xsl $(BEHAVIOUR)\PrimitiveClasses.xml
    @ECHO.
    @ECHO Generating $(@)
    xalan -o $(@) $(BEHAVIOUR)\PrimitiveClasses.xml $(BEHAVIOUR)\ClassTypeCodes.xsl

$(OR_OUTDIR)\winmsgtb.res: $(INT_PLATFORM)\winmsgtb.rc $(INTERPRETER_MESSAGES)\DocErrorMessages.sgml
    @ECHO.
    @ECHO ResourceCompiling $(@)
        $(rc) $(rcflags_common) $(OR_ORYXRCINCL) -r -fo$(@) $(INT_PLATFORM)\winmsgtb.rc


# Update the version information block
$(OR_OUTDIR)\verinfo.res: $(INT_PLATFORM)\verinfo.rc
    @ECHO.
    @ECHO ResourceCompiling $(@B).res
        $(rc) $(rcflags_common) -r -fo$(OR_OUTDIR)\$(@B).res $(OR_INTERPRETER_SRC)\$(@B).rc

#
#
# *** Copy ORX files to target dir...
#
COPYORXFILES: $(ORXFILES)

#
#
# *** Copy API files to target dir...
#
COPYAPIFILES: $(APIFILES)

#
#
# *** Make sure headers are generated
#
ORXHEADERS: $(ORXHEADERS)

#
# *** Inference Rule for Rexx Class files
#
{$(REXX_CLASSES)}.orx{$(OR_OUTDIR)}.orx:
    @ECHO .
    @ECHO Copying $(**)
    COPY $(**) $(@)

#
# *** Inference Rule for Rexx Class files
#
{$(INT_PLATFORM)}.orx{$(OR_OUTDIR)}.orx:
    @ECHO .
    @ECHO Copying $(**)
    COPY $(**) $(@)

#
# *** Inference Rule for Rexx samples
#
{$(OR_SAMPLESRC)}.rex{$(OR_OUTDIR)}.rex:
    @ECHO .
    @ECHO Copying $(**)
    COPY $(**) $(@)

#
# *** Inference Rule for Rexx samples
#
{$(OR_OLEOBJECTSRC)}.cls{$(OR_OUTDIR)}.cls:
    @ECHO .
    @ECHO Copying $(**)
    COPY $(**) $(@)


#
# *** Inference Rule for C->OBJ
# *** For .C files in OR_LIBSRC directory
#
{$(OR_LIBSRC)}.c{$(OR_OUTDIR)}.obj:
    @ECHO .
    @ECHO Compiling $(@B).c
    $(OR_CC)  $(cflags_common) $(cflags_dll) /Fo$(OR_OUTDIR)\$(@B).obj $(Tp)$(OR_LIBSRC)\$(@B).c $(OR_ORYXINCL)

#
# *** Inference Rule for CPP->OBJ
# *** For .CPP files in OR_LIBSRC directory
#
{$(OR_LIBSRC)}.cpp{$(OR_OUTDIR)}.obj:
    @ECHO .
    @ECHO Compiling $(**)
    $(OR_CC)  $(cflags_common) $(cflags_dll) /Fo$(OR_OUTDIR)\$(@B).obj $(Tp)$(OR_LIBSRC)\$(@B).cpp $(OR_ORYXINCL)

#
# *** Inference Rule for CPP->OBJ
# *** For .CPP files in OR_LIBSRC directory
#
{$(INTERPRETER_RUNTIME)}.cpp{$(OR_OUTDIR)}.obj:
    @ECHO .
    @ECHO Compiling $(**)
    $(OR_CC)  $(cflags_common) $(cflags_dll) /Fo$(@) $(Tp)$(**) $(OR_ORYXINCL)


#
# *** Inference Rule for CPP->OBJ
# *** For .CPP files in OR_LIBSRC directory
#
{$(INTERPRETER_API)}.cpp{$(OR_OUTDIR)}.obj:
    @ECHO .
    @ECHO Compiling $(**)
    $(OR_CC)  $(cflags_common) $(cflags_dll) /Fo$(@) $(Tp)$(**) $(OR_ORYXINCL)

#
# *** Inference Rule for CPP->OBJ
# *** For .CPP files in OR_LIBSRC directory
#
{$(PARSER)}.cpp{$(OR_OUTDIR)}.obj:
    @ECHO .
    @ECHO Compiling $(**)
    $(OR_CC)  $(cflags_common) $(cflags_dll) /Fo$(@) $(Tp)$(**) $(OR_ORYXINCL)

#
# *** Inference Rule for CPP->OBJ
# *** For .CPP files in OR_LIBSRC directory
#
{$(EXPRESSIONS)}.cpp{$(OR_OUTDIR)}.obj:
    @ECHO .
    @ECHO Compiling $(**)
    $(OR_CC)  $(cflags_common) $(cflags_dll) /Fo$(@) $(Tp)$(**) $(OR_ORYXINCL)

#
# *** Inference Rule for CPP->OBJ
# *** For .CPP files in OR_LIBSRC directory
#
{$(CONCURRENCY)}.cpp{$(OR_OUTDIR)}.obj:
    @ECHO .
    @ECHO Compiling $(**)
    $(OR_CC)  $(cflags_common) $(cflags_dll) /Fo$(@) $(Tp)$(**) $(OR_ORYXINCL)

#
# *** Inference Rule for CPP->OBJ
# *** For .CPP files in OR_LIBSRC directory
#
{$(BEHAVIOUR)}.cpp{$(OR_OUTDIR)}.obj:
    @ECHO .
    @ECHO Compiling $(**)
    $(OR_CC)  $(cflags_common) $(cflags_dll) /Fo$(@) $(Tp)$(**) $(OR_ORYXINCL)

#
# *** Inference Rule for CPP->OBJ
# *** For .CPP files in OR_LIBSRC directory
#
{$(EXECUTION)}.cpp{$(OR_OUTDIR)}.obj:
    @ECHO .
    @ECHO Compiling $(**)
    $(OR_CC)  $(cflags_common) $(cflags_dll) /Fo$(@) $(Tp)$(**) $(OR_ORYXINCL)

#
# *** Inference Rule for CPP->OBJ
# *** For .CPP files in OR_LIBSRC directory
#
{$(MEMORY)}.cpp{$(OR_OUTDIR)}.obj:
    @ECHO .
    @ECHO Compiling $(**)
    $(OR_CC)  $(cflags_common) $(cflags_dll) /Fo$(@) $(Tp)$(**) $(OR_ORYXINCL)

#
# *** Inference Rule for CPP->OBJ
# *** For .CPP files in OR_LIBSRC directory
#
{$(PACKAGE)}.cpp{$(OR_OUTDIR)}.obj:
    @ECHO .
    @ECHO Compiling $(**)
    $(OR_CC)  $(cflags_common) $(cflags_dll) /Fo$(@) $(Tp)$(**) $(OR_ORYXINCL)

#
# *** Inference Rule for CPP->OBJ
# *** For .CPP files in OR_LIBSRC directory
#
{$(INSTRUCTIONS)}.cpp{$(OR_OUTDIR)}.obj:
    @ECHO .
    @ECHO Compiling $(**)
    $(OR_CC)  $(cflags_common) $(cflags_dll) /Fo$(@) $(Tp)$(**) $(OR_ORYXINCL)

#
# *** Inference Rule for CPP->OBJ
# *** For .CPP files in OR_LIBSRC directory
#
{$(INTERPRETER_CLASSES)}.cpp{$(OR_OUTDIR)}.obj:
    @ECHO .
    @ECHO Compiling $(**)
    $(OR_CC)  $(cflags_common) $(cflags_dll) /Fo$(@) $(Tp)$(**) $(OR_ORYXINCL)

#
# *** Inference Rule for CPP->OBJ
# *** For .CPP files in OR_LIBSRC directory
#
{$(CLASSES_SUPPORT)}.cpp{$(OR_OUTDIR)}.obj:
    @ECHO .
    @ECHO Compiling $(**)
    $(OR_CC)  $(cflags_common) $(cflags_dll) /Fo$(@) $(Tp)$(**) $(OR_ORYXINCL)

#
# *** Inference Rule for CPP->OBJ
# *** For .CPP files in OR_LIBSRC directory
#
{$(INT_PLATFORM)}.cpp{$(OR_OUTDIR)}.obj:
    @ECHO .
    @ECHO Compiling $(**)
    $(OR_CC)  $(cflags_common) $(cflags_dll) /Fo$(@) $(Tp)$(**) $(OR_ORYXINCL)

#
# *** Inference Rule for C->OBJ
# *** For .C files in OR_LIBSRC directory
#
{$(INTERPRETER_RUNTIME)}.c{$(OR_OUTDIR)}.obj:
    @ECHO .
    @ECHO Compiling $(**)
    $(OR_CC) $(cflags_common) $(cflags_dll) /Fo$(@) $(OR_ORYXINCL) $(Tp)$(**)

#
# *** Inference Rule for C->OBJ
# *** For .C files in OR_LIBSRC directory
#
{$(INT_PLATFORM)}.c{$(OR_OUTDIR)}.obj:
    @ECHO .
    @ECHO Compiling $(**)
    $(OR_CC) $(cflags_common) $(cflags_dll)  /Fo$(@) $(OR_ORYXINCL) $(Tp)$(**)

#
# *** Inference Rule for CPP->OBJ
# *** For .CPP files in OR_LIBSRC directory
#
{$(STREAM)}.cpp{$(OR_OUTDIR)}.obj:
    @ECHO .
    @ECHO Compiling $(**)
    $(OR_CC) $(cflags_common) $(cflags_dll)  /Fo$(@) $(OR_ORYXINCL) $(Tp)$(**)


# Copy any out-of-date API files to a single directory.
$(OR_OUTDIR_API)\rexx.lib : $(OR_OUTDIR)\rexx.lib
    @copy $? $(OR_OUTDIR_API) 1>nul 2>&1

$(OR_OUTDIR_API)\rexxapi.lib : $(OR_OUTDIR)\rexxapi.lib
    @copy $? $(OR_OUTDIR_API) 1>nul 2>&1

$(OR_OUTDIR_API)\oorexxapi.h : $(OR_APISRC)\oorexxapi.h
    @copy $? $(OR_OUTDIR_API) 1>nul 2>&1

$(OR_OUTDIR_API)\oorexxerrors.h : $(OR_APISRC)\oorexxerrors.h
    @copy $? $(OR_OUTDIR_API) 1>nul 2>&1

$(OR_OUTDIR_API)\rexx.h : $(OR_APISRC)\rexx.h
    @copy $? $(OR_OUTDIR_API) 1>nul 2>&1

$(OR_OUTDIR_API)\rexxapidefs.h : $(OR_APISRC)\rexxapidefs.h
    @copy $? $(OR_OUTDIR_API) 1>nul 2>&1

$(OR_OUTDIR_API)\rexxapitypes.h : $(OR_APIWINSRC)\rexxapitypes.h
    @copy $? $(OR_OUTDIR_API) 1>nul 2>&1

$(OR_OUTDIR_API)\rexxplatformapis.h : $(OR_APIWINSRC)\rexxplatformapis.h
    @copy $? $(OR_OUTDIR_API) 1>nul 2>&1

$(OR_OUTDIR_API)\rexxplatformdefs.h : $(OR_APIWINSRC)\rexxplatformdefs.h
    @copy $? $(OR_OUTDIR_API) 1>nul 2>&1
