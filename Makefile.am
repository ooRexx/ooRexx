#/*----------------------------------------------------------------------------*/
#/*                                                                            */
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


AUTOMAKE_OPTIONS	= foreign dist-zip

MAINTAINERCLEANFILES 	= Makefile.in aclocal.m4 configure config.guess config.sub \
			depcomp install-sh ltmain.sh missing mkinstalldirs config.h.in \
			compile @REXXCAT@ rexx.img \
			stamp-h.in cscope.files cscope.out $(distdir).tar.gz

SUBDIRS			= samples
DIST_SUBDIRS 		= samples

########## start of new ##########

# get version details
include $(top_srcdir)/oorexx.ver

VERSION = $(ORX_MAJOR).$(ORX_MINOR).$(ORX_MOD_LVL)
#driver_name = orexx-$(ORX_MAJOR).$(ORX_MINOR).$(ORX_MOD_LVL).0-$(ORX_BLD_LVL)

# Please note that the version-info has nothing to do with the release version.
# You need to know exactly what you are doing in order to get this correct.
VERSION_INFO = -version-info ${ORX_CURRENT}:${ORX_REVISION}:${ORX_AGE}

platform_dir = $(top_srcdir)/platform
platform_unix_dir = $(platform_dir)/unix
platform_windows_dir = $(platform_dir)/windows
build_platform_unix_dir = platform/unix
rexxapi_dir = $(top_srcdir)/rexxapi
rexxapi_unix_dir = $(rexxapi_dir)/unix
rexxapi_windows_dir = $(rexxapi_dir)/windows
kernel_dir = $(top_srcdir)/kernel
kernel_platform_unix_dir = $(kernel_dir)/platform/unix
kernel_platform_windows_dir = $(kernel_dir)/platform/windows
kernel_main_dir = $(kernel_dir)/main
kernel_platform_common_dir = $(kernel_dir)/platform/common
classes_dir = $(kernel_dir)/classes
expression_dir = $(kernel_dir)/expression
instructions_dir = $(kernel_dir)/instructions
parser_dir = $(kernel_dir)/parser
runtime_dir = $(kernel_dir)/runtime
streamlibrary_dir = $(kernel_dir)/streamLibrary
messages_dir = $(kernel_dir)/messages
lib_dir = $(top_srcdir)/lib
api_dir = $(top_srcdir)/api
platform_api_dir = $(api_dir)/platform/unix

rexutils_dir = $(top_srcdir)/rexutils
rexutils_unix_dir = $(rexutils_dir)/unix
rexutils_windows_dir = $(rexutils_dir)/windows
rxregexp_dir = $(top_srcdir)/rxregexp

rexxclasses_dir = $(kernel_dir)/RexxClasses

samples_dir = $(top_srcdir)/samples
samples_windows_dir = $(top_srcdir)/samples/windows
samples_unix_dir = $(top_srcdir)/samples/unix

catdir = $(prefix)/bin

COMMON_CPPFLAGS = -DORX_VER=$(ORX_MAJOR) -DORX_REL=$(ORX_MINOR) -DORX_MOD=$(ORX_MOD_LVL) -DORX_FIX=0 \
                  -DORX_SYS_STR=\"@ORX_SYS_STR@\" \
                  -DORX_CATDIR=\"$(catdir)\" \
                  -DORX_SHARED_LIBRARY_EXT=\"@ORX_SHARED_LIBRARY_EXT@\" \
                  -DHIGHTID

messageXML = $(messages_dir)/rexxmsg.xml

classesXML = $(runtime_dir)/PrimitiveClasses.xml

#
# header files for source distribution
#
errorHeaders = $(messages_dir)/RexxMessageNumbers.h \
               $(messages_dir)/RexxErrorCodes.h \
               $(messages_dir)/RexxMessageTable.h
classesHeaders = $(classes_dir)/ArrayClass.hpp \
                 $(classes_dir)/BufferClass.hpp \
                 $(classes_dir)/ClassClass.hpp \
                 $(classes_dir)/DirectoryClass.hpp \
                 $(classes_dir)/IntegerClass.hpp \
                 $(classes_dir)/ListClass.hpp \
                 $(classes_dir)/MessageClass.hpp \
                 $(classes_dir)/MethodClass.hpp \
                 $(classes_dir)/MutableBufferClass.hpp \
                 $(classes_dir)/NumberStringClass.hpp \
                 $(classes_dir)/NumberStringMath.hpp \
                 $(classes_dir)/ObjectClass.hpp \
                 $(classes_dir)/PointerClass.hpp \
                 $(classes_dir)/QueueClass.hpp \
                 $(classes_dir)/RelationClass.hpp \
                 $(classes_dir)/StackClass.hpp \
                 $(classes_dir)/StemClass.hpp \
                 $(classes_dir)/StringClass.hpp \
                 $(classes_dir)/SupplierClass.hpp \
                 $(classes_dir)/TableClass.hpp \
                 $(classes_dir)/WeakReferenceClass.hpp
expressionHeaders = $(expression_dir)/BuiltinFunctions.hpp \
                    $(expression_dir)/ExpressionBaseVariable.hpp \
                    $(expression_dir)/ExpressionCompoundVariable.hpp \
                    $(expression_dir)/ExpressionDotVariable.hpp \
                    $(expression_dir)/ExpressionFunction.hpp \
                    $(expression_dir)/ExpressionMessage.hpp \
                    $(expression_dir)/ExpressionOperator.hpp \
                    $(expression_dir)/ExpressionStack.hpp \
                    $(expression_dir)/ExpressionStem.hpp \
                    $(expression_dir)/ExpressionVariable.hpp \
                    $(expression_dir)/IndirectVariableReference.hpp \
                    $(expression_dir)/ExpressionLogical.hpp
instructionsHeaders = $(instructions_dir)/AddressInstruction.hpp \
                      $(instructions_dir)/AssignmentInstruction.hpp \
                      $(instructions_dir)/CallInstruction.hpp \
                      $(instructions_dir)/CommandInstruction.hpp \
                      $(instructions_dir)/DoBlock.hpp \
                      $(instructions_dir)/DoInstruction.hpp \
                      $(instructions_dir)/DropInstruction.hpp \
                      $(instructions_dir)/ElseInstruction.hpp \
                      $(instructions_dir)/EndIf.hpp \
                      $(instructions_dir)/EndInstruction.hpp \
                      $(instructions_dir)/ExitInstruction.hpp \
                      $(instructions_dir)/ExposeInstruction.hpp \
                      $(instructions_dir)/ForwardInstruction.hpp \
                      $(instructions_dir)/GuardInstruction.hpp \
                      $(instructions_dir)/IfInstruction.hpp \
                      $(instructions_dir)/InterpretInstruction.hpp \
                      $(instructions_dir)/LabelInstruction.hpp \
                      $(instructions_dir)/LeaveInstruction.hpp \
                      $(instructions_dir)/MessageInstruction.hpp \
                      $(instructions_dir)/NopInstruction.hpp \
                      $(instructions_dir)/NumericInstruction.hpp \
                      $(instructions_dir)/OptionsInstruction.hpp \
                      $(instructions_dir)/OtherwiseInstruction.hpp \
                      $(instructions_dir)/ParseInstruction.hpp \
                      $(instructions_dir)/ParseTarget.hpp \
                      $(instructions_dir)/ParseTrigger.hpp \
                      $(instructions_dir)/ProcedureInstruction.hpp \
                      $(instructions_dir)/QueueInstruction.hpp \
                      $(instructions_dir)/RaiseInstruction.hpp \
                      $(instructions_dir)/ReplyInstruction.hpp \
                      $(instructions_dir)/ReturnInstruction.hpp \
                      $(instructions_dir)/RexxInstruction.hpp \
                      $(instructions_dir)/SayInstruction.hpp \
                      $(instructions_dir)/SelectInstruction.hpp \
                      $(instructions_dir)/SignalInstruction.hpp \
                      $(instructions_dir)/ThenInstruction.hpp \
                      $(instructions_dir)/TraceInstruction.hpp \
                      $(instructions_dir)/UseStrictInstruction.hpp
parserHeaders = $(parser_dir)/Clause.hpp \
                $(parser_dir)/SourceFile.hpp \
                $(parser_dir)/SourceLocation.hpp \
                $(parser_dir)/Token.hpp
platformHeaders = $(kernel_platform_unix_dir)/APIDefinitions.h \
                  $(kernel_platform_unix_dir)/PlatformDefinitions.h \
                  $(kernel_platform_unix_dir)/SharedMemorySupport.h \
                  $(kernel_platform_unix_dir)/SystemCommands.h \
                  $(kernel_platform_unix_dir)/SystemSemaphores.h \
                  $(kernel_platform_unix_dir)/ThreadSupport.hpp
platformcommonHeaders =
runtimeHeaders = $(runtime_dir)/DeadObject.hpp \
                 $(runtime_dir)/ClassTypeCodes.h \
                 $(runtime_dir)/GlobalNames.h \
                 $(runtime_dir)/MemorySegment.hpp \
                 $(runtime_dir)/MemoryStats.hpp \
                 $(runtime_dir)/NativeMethods.h \
                 $(runtime_dir)/Numerics.hpp \
                 $(runtime_dir)/PrimitiveBehaviourNames.h \
                 $(runtime_dir)/RexxActivation.hpp \
                 $(runtime_dir)/RexxActivationStack.hpp \
                 $(runtime_dir)/RexxActivity.hpp \
                 $(runtime_dir)/ActivityManager.hpp \
                 $(runtime_dir)/LibraryManager.hpp \
                 $(runtime_dir)/Interpreter.hpp \
                 $(runtime_dir)/ProtectedObject.hpp \
                 $(runtime_dir)/RexxBehaviour.hpp \
                 $(runtime_dir)/RexxBuiltinFunctions.h \
                 $(runtime_dir)/RexxCode.hpp \
                 $(runtime_dir)/RexxCollection.hpp \
                 $(runtime_dir)/RexxCompoundElement.hpp \
                 $(runtime_dir)/RexxCompoundTable.hpp \
                 $(runtime_dir)/RexxCompoundTail.hpp \
                 $(runtime_dir)/RexxConstants.hpp \
                 $(runtime_dir)/RexxCore.h \
                 $(runtime_dir)/RexxDateTime.hpp \
                 $(runtime_dir)/RexxEnvelope.hpp \
                 $(runtime_dir)/RexxHashTable.hpp \
                 $(runtime_dir)/RexxInternalStack.hpp \
                 $(runtime_dir)/RexxListTable.hpp \
                 $(runtime_dir)/RexxLocalVariables.hpp \
                 $(runtime_dir)/RexxMemory.hpp \
                 $(runtime_dir)/RexxMisc.hpp \
                 $(runtime_dir)/RexxNativeAPI.h \
                 $(runtime_dir)/RexxNativeActivation.hpp \
                 $(runtime_dir)/RexxNativeCode.hpp \
                 $(runtime_dir)/RexxSmartBuffer.hpp \
                 $(runtime_dir)/RexxVariable.hpp \
                 $(runtime_dir)/RexxVariableDictionary.hpp
streamlibraryHeaders = $(streamlibrary_dir)/StreamCommandParser.h \
                       $(streamlibrary_dir)/StreamNative.h
libHeaders = $(lib_dir)/RexxPlatformDefinitions.h \
             $(lib_dir)/RexxPlatformInterface.h
rexxapiHeaders = $(rexxapi_dir)/APIUtil.h \
                 $(rexxapi_dir)/ASCIISymbols.h \
                 $(rexxapi_dir)/Characters.h \
                 $(rexxapi_dir)/CodePageTables.h
rexxapiunixHeaders = $(rexxapi_unix_dir)/APIUtilities.h \
                     $(rexxapi_unix_dir)/RexxAPIManager.h \
                     $(rexxapi_unix_dir)/SubcommandAPI.h
rxregexpHeaders = $(rxregexp_dir)/automaton.hpp \
                  $(rxregexp_dir)/dblqueue.cpp \
                  $(rxregexp_dir)/dblqueue.hpp \
                  $(rxregexp_dir)/regexp.hpp
rexutilsHeaders = $(rexutils_dir)/rxsock.h
aixExportFiles = $(rexutils_unix_dir)/*.exp \
                 $(rexutils_dir)/*.exp \
                 $(rexxapi_dir)/*.exp \
                 $(rxregexp_dir)/*.exp
windowsDefFiles = $(kernel_platform_windows_dir)/*.def \
                  $(lib_dir)/*.def \
                  $(rexutils_dir)/*.def \
                  $(rxregexp_dir)/*.def
miscFiles = $(top_srcdir)/CPLv1.0.txt \
            $(top_srcdir)/CHANGES \
            $(top_srcdir)/CONTRIBUTORS \
            $(top_srcdir)/NOTICE \
            $(top_srcdir)/oorexx.ver \
            $(top_srcdir)/unix-like-build.txt \
            $(aixExportFiles) \
            $(platform_unix_dir)/oorexx.spec \
            $(platform_unix_dir)/makepkg.rex \
            $(rxregexp_dir)/*.cls \
            $(rexutils_dir)/*.cls \
            $(samples_dir)/*.rex \
            $(samples_windows_dir)/* \
            $(samples_dir)/readme
windowsFiles = \
            $(top_srcdir)/makeorx.bat \
            $(top_srcdir)/orxdb.bat \
            $(top_srcdir)/windows-build.txt \
            $(lib_dir)/*.mak \
            $(kernel_platform_windows_dir)/*.cpp \
            $(kernel_platform_windows_dir)/*.h \
            $(kernel_platform_windows_dir)/*.rc \
            $(kernel_platform_windows_dir)/*.xsl \
            $(kernel_platform_windows_dir)/*.orx \
            $(kernel_dir)/*.mak \
            $(rexxapi_dir)/*.mak \
            $(rexutils_dir)/*.mak \
            $(rxregexp_dir)/*.mak \
            $(rexxapi_windows_dir)/* \
            $(rexutils_windows_dir)/* \
            $(platform_windows_dir)/*
#
# These error message files are built BEFORE any other target
#
BUILT_SOURCES = $(errorHeaders) \
                rexx.img @REXXCAT@
#
# Binary targets
#
bin_PROGRAMS = rexx rexxc rxqueue rxsubcom rxdelipc

#
# Shared library targets
#
pkglib_LTLIBRARIES = librexxapi.la librexx.la librexxutil.la librxmath.la librxregexp.la librxsock.la

#
# Scripts to install
#
pkgdata_SCRIPTS = $(platform_unix_dir)/rexx.sh $(platform_unix_dir)/rexx.csh

#
# headers to install
#
include_HEADERS = $(api_dir)/rexx.h \
                  $(api_dir)/rexxapidefs.h \
                  $(platform_api_dir)/rexxapitypes.h \
                  $(platform_api_dir)/rexxplatformapis.h \
                  $(platform_api_dir)/rexxplatformdefs.h

#
# man pages to install
#
man_MANS = $(platform_unix_dir)/rexx.1 \
           $(platform_unix_dir)/rexxc.1 \
           $(platform_unix_dir)/rxsubcom.1 \
           $(platform_unix_dir)/rxqueue.1

#################### librexx.so ##########################
# Sources for librexxapi.so
#
librexxapi_la_SOURCES = $(rexxapi_unix_dir)/SubcommandAPI.cpp \
                        $(rexxapi_unix_dir)/RexxAPIManager.cpp \
                        $(kernel_platform_unix_dir)/SystemSemaphores.cpp \
                        $(kernel_platform_unix_dir)/SharedMemorySupport.cpp \
                        $(rexxapi_unix_dir)/MacroSpace.cpp \
                        $(rexxapi_unix_dir)/QueuesAPI.cpp

#
# Preprocessor flags for librexxapi.so
#
librexxapi_la_CXXFLAGS  = $(COMMON_CPPFLAGS) \
                          -I$(lib_dir) \
                          -I$(api_dir) \
                          -I$(platform_api_dir) \
                          -I$(kernel_dir) \
                          -I$(expression_dir) \
                          -I$(instructions_dir) \
                          -I$(classes_dir) \
                          -I$(runtime_dir) \
                          -I$(parser_dir) \
                          -I$(messages_dir) \
                          -I$(kernel_platform_common_dir) \
                          -I$(kernel_platform_unix_dir) \
                          -I$(rexxapi_dir) \
                          -I$(rexxapi_unix_dir)

librexxapi_la_LDFLAGS = $(VERSION_INFO)

#################### librexx.so ##########################
# Sources for librexx.so
#
classes_sources = $(classes_dir)/ArrayClass.cpp \
                  $(classes_dir)/BufferClass.cpp \
                  $(classes_dir)/ClassClass.cpp \
                  $(classes_dir)/DirectoryClass.cpp \
                  $(classes_dir)/IntegerClass.cpp \
                  $(classes_dir)/ListClass.cpp \
                  $(classes_dir)/MessageClass.cpp \
                  $(classes_dir)/MethodClass.cpp \
                  $(classes_dir)/MutableBufferClass.cpp \
                  $(classes_dir)/NumberStringClass.cpp \
                  $(classes_dir)/NumberStringMath.cpp \
                  $(classes_dir)/NumberStringMath2.cpp \
                  $(classes_dir)/ObjectClass.cpp \
                  $(classes_dir)/PointerClass.cpp \
                  $(classes_dir)/QueueClass.cpp \
                  $(classes_dir)/RelationClass.cpp \
                  $(classes_dir)/StackClass.cpp \
                  $(classes_dir)/StemClass.cpp \
                  $(classes_dir)/StringClass.cpp \
                  $(classes_dir)/StringClassBit.cpp \
                  $(classes_dir)/StringClassConversion.cpp \
                  $(classes_dir)/StringClassMisc.cpp \
                  $(classes_dir)/StringClassSub.cpp \
                  $(classes_dir)/StringClassUtil.cpp \
                  $(classes_dir)/StringClassWord.cpp \
                  $(classes_dir)/SupplierClass.cpp \
                  $(classes_dir)/TableClass.cpp \
                  $(classes_dir)/WeakReferenceClass.cpp
expression_sources = $(expression_dir)/BuiltinFunctions.cpp \
                     $(expression_dir)/ExpressionCompoundVariable.cpp \
                     $(expression_dir)/ExpressionDotVariable.cpp \
                     $(expression_dir)/ExpressionFunction.cpp \
                     $(expression_dir)/ExpressionMessage.cpp \
                     $(expression_dir)/ExpressionOperator.cpp \
                     $(expression_dir)/ExpressionStack.cpp \
                     $(expression_dir)/ExpressionStem.cpp \
                     $(expression_dir)/ExpressionVariable.cpp \
                     $(expression_dir)/IndirectVariableReference.cpp \
                     $(expression_dir)/ExpressionLogical.cpp
instructions_sources  = $(instructions_dir)/AddressInstruction.cpp \
                        $(instructions_dir)/AssignmentInstruction.cpp \
                        $(instructions_dir)/CallInstruction.cpp \
                        $(instructions_dir)/CommandInstruction.cpp \
                        $(instructions_dir)/DoBlock.cpp \
                        $(instructions_dir)/DoInstruction.cpp \
                        $(instructions_dir)/DropInstruction.cpp \
                        $(instructions_dir)/ElseInstruction.cpp \
                        $(instructions_dir)/EndIf.cpp \
                        $(instructions_dir)/EndInstruction.cpp \
                        $(instructions_dir)/ExitInstruction.cpp \
                        $(instructions_dir)/ExposeInstruction.cpp \
                        $(instructions_dir)/ForwardInstruction.cpp \
                        $(instructions_dir)/GuardInstruction.cpp \
                        $(instructions_dir)/IfInstruction.cpp \
                        $(instructions_dir)/InterpretInstruction.cpp \
                        $(instructions_dir)/LabelInstruction.cpp \
                        $(instructions_dir)/LeaveInstruction.cpp \
                        $(instructions_dir)/MessageInstruction.cpp \
                        $(instructions_dir)/NopInstruction.cpp \
                        $(instructions_dir)/NumericInstruction.cpp \
                        $(instructions_dir)/OptionsInstruction.cpp \
                        $(instructions_dir)/OtherwiseInstruction.cpp \
                        $(instructions_dir)/ParseInstruction.cpp \
                        $(instructions_dir)/ParseTarget.cpp \
                        $(instructions_dir)/ParseTrigger.cpp \
                        $(instructions_dir)/ProcedureInstruction.cpp \
                        $(instructions_dir)/QueueInstruction.cpp \
                        $(instructions_dir)/RaiseInstruction.cpp \
                        $(instructions_dir)/ReplyInstruction.cpp \
                        $(instructions_dir)/ReturnInstruction.cpp \
                        $(instructions_dir)/RexxInstruction.cpp \
                        $(instructions_dir)/SayInstruction.cpp \
                        $(instructions_dir)/SelectInstruction.cpp \
                        $(instructions_dir)/SignalInstruction.cpp \
                        $(instructions_dir)/ThenInstruction.cpp \
                        $(instructions_dir)/TraceInstruction.cpp \
                        $(instructions_dir)/UseStrictInstruction.cpp
parser_sources = $(parser_dir)/Clause.cpp \
                 $(parser_dir)/KeywordConstants.cpp \
                 $(parser_dir)/InstructionParser.cpp \
                 $(parser_dir)/Scanner.cpp \
                 $(parser_dir)/SourceFile.cpp \
                 $(parser_dir)/Token.cpp
kernel_platform_common_sources =
#                        SharedMemorySupport.cpp
#                        SystemSemaphores.cpp
platform_sources = $(kernel_platform_unix_dir)/ErrorMessages.cpp \
                   $(kernel_platform_unix_dir)/ExternalFunctions.cpp \
                   $(kernel_platform_unix_dir)/FileSystem.cpp \
                   $(kernel_platform_unix_dir)/MemorySupport.cpp \
                   $(kernel_platform_unix_dir)/MiscSystem.cpp \
                   $(kernel_platform_unix_dir)/RexxMain.cpp \
                   $(kernel_platform_unix_dir)/RexxQueues.cpp \
                   $(kernel_platform_unix_dir)/StoredMetaData.cpp \
                   $(kernel_platform_unix_dir)/SystemCommands.cpp \
                   $(kernel_platform_unix_dir)/ThreadSupport.cpp \
                   $(kernel_platform_unix_dir)/TimeSupport.cpp \
                   $(kernel_platform_unix_dir)/UseridFunction.cpp \
                   $(kernel_platform_unix_dir)/ValueFunction.cpp \
                   $(kernel_platform_unix_dir)/VariablePool.cpp
#                   $(runtime_dir)/RexxVariableDictionary2.cpp
runtime_sources  = $(runtime_dir)/DeadObject.cpp \
                   $(runtime_dir)/GlobalData.cpp \
                   $(runtime_dir)/GlobalNames.cpp \
                   $(runtime_dir)/MemorySegment.cpp \
                   $(runtime_dir)/MemoryStats.cpp \
                   $(runtime_dir)/NativeRoot.cpp \
                   $(runtime_dir)/Numerics.cpp \
                   $(runtime_dir)/PrimitiveBehaviours.cpp \
                   $(runtime_dir)/RexxActivation.cpp \
                   $(runtime_dir)/RexxActivationStack.cpp \
                   $(runtime_dir)/RexxActivity.cpp \
                   $(runtime_dir)/ActivityManager.cpp \
                   $(runtime_dir)/LibraryManager.cpp \
                   $(runtime_dir)/Interpreter.cpp \
                   $(runtime_dir)/RexxBehaviour.cpp \
                   $(runtime_dir)/RexxCode.cpp \
                   $(runtime_dir)/RexxCollection.cpp \
                   $(runtime_dir)/RexxCompoundElement.cpp \
                   $(runtime_dir)/RexxCompoundTable.cpp \
                   $(runtime_dir)/RexxCompoundTail.cpp \
                   $(runtime_dir)/RexxDateTime.cpp \
                   $(runtime_dir)/RexxEnvelope.cpp \
                   $(runtime_dir)/RexxHashTable.cpp \
                   $(runtime_dir)/RexxInternalStack.cpp \
                   $(runtime_dir)/RexxListTable.cpp \
                   $(runtime_dir)/RexxLocalVariables.cpp \
                   $(runtime_dir)/RexxMemory.cpp \
                   $(runtime_dir)/RexxMisc.cpp \
                   $(runtime_dir)/RexxNativeActivation.cpp \
                   $(runtime_dir)/RexxNativeCode.cpp \
                   $(runtime_dir)/RexxSmartBuffer.cpp \
                   $(runtime_dir)/RexxStartup.cpp \
                   $(runtime_dir)/RexxVariable.cpp \
                   $(runtime_dir)/RexxVariableDictionary.cpp \
                   $(runtime_dir)/Setup.cpp \
                   $(runtime_dir)/Utilities.cpp \
                   $(runtime_dir)/Version.cpp \
                   $(runtime_dir)/VirtualFunctionTable.cpp
streamlibrary_sources = $(streamlibrary_dir)/StreamCommandParser.cpp \
                        $(streamlibrary_dir)/StreamNative.cpp
lib_sources = $(lib_dir)/LocalProcessData.cpp
librexx_la_SOURCES = $(classes_sources) \
                     $(expression_sources) \
                     $(instructions_sources) \
                     $(parser_sources) \
                     $(kernel_platform_common_sources) \
                     $(platform_sources) \
                     $(runtime_sources) \
                     $(streamlibrary_sources) \
                     $(lib_sources)
core_classes = $(rexxclasses_dir)/CoreClasses.orx \
               $(rexxclasses_dir)/StreamClasses.orx \
	       $(rexxclasses_dir)/SystemMethods.orx
core_platform_classes = $(kernel_platform_unix_dir)/PlatformObjects.orx \
                        $(kernel_platform_unix_dir)/UnixMethods.orx
#
# Preprocessor flags for librexx.so
#
librexx_la_CXXFLAGS  = $(COMMON_CPPFLAGS) \
                       -I$(lib_dir) \
                       -I$(api_dir) \
                       -I$(platform_api_dir) \
                       -I$(kernel_dir) \
                       -I$(expression_dir) \
                       -I$(instructions_dir) \
                       -I$(classes_dir) \
                       -I$(runtime_dir) \
                       -I$(parser_dir) \
                       -I$(messages_dir) \
                       -I$(streamlibrary_dir) \
                       -I$(kernel_platform_common_dir) \
                       -I$(kernel_platform_unix_dir) \
                       -I$(rexxapi_unix_dir)

librexx_la_LDFLAGS = $(VERSION_INFO)
#################### rexx ##########################
# Sources for rexx
#
rexx_SOURCES = $(platform_unix_dir)/rexx.cpp
#
# Preprocessor flags for rexx
#
rexx_CXXFLAGS  = $(COMMON_CPPFLAGS) \
                 -I$(lib_dir) \
                 -I$(api_dir) \
                 -I$(platform_api_dir) \
                 -I$(messages_dir) \
                 -I$(kernel_platform_unix_dir) \
                 -I$(rexxapi_unix_dir)
#
# Extra libraries for rexx
#
rexx_LDADD   = librexx.la librexxapi.la @ORX_LDADD_EXECUTABLE@
rexx_LDFLAGS = @ORX_LDFLAGS_EXECUTABLE@

#################### rexxc #########################
# Sources for rexxc
#
rexxc_SOURCES = $(platform_unix_dir)/RexxCompiler.cpp
#
# Preprocessor flags for rexxc
#
rexxc_CXXFLAGS  = $(COMMON_CPPFLAGS) \
                  -I$(lib_dir) \
                  -I$(api_dir) \
                  -I$(platform_api_dir) \
                  -I$(messages_dir) \
                  -I$(kernel_platform_unix_dir) \
                  -I$(rexxapi_unix_dir)

#                  -I$(top_srcdir)/kernel/messages
#
# Extra libraries for rexxc
#
rexxc_LDADD   = librexx.la librexxapi.la @ORX_LDADD_EXECUTABLE@
rexxc_LDFLAGS = @ORX_LDFLAGS_EXECUTABLE@


#################### rxqueue #########################
# Sources for rxqueue
#
rxqueue_SOURCES =  $(rexxapi_unix_dir)/QueueCommand.cpp
#
# Preprocessor flags for rxqueue
#
rxqueue_CXXFLAGS  = $(COMMON_CPPFLAGS) \
                    -I$(lib_dir) \
                    -I$(api_dir) \
                    -I$(platform_api_dir) \
                    -I$(kernel_platform_unix_dir) \
                    -I$(messages_dir) \
                    -I$(rexxapi_unix_dir)
#
# Extra libraries for rxqueue
#
rxqueue_LDADD   = librexx.la librexxapi.la @ORX_LDADD_EXECUTABLE@
rxqueue_LDFLAGS = @ORX_LDFLAGS_EXECUTABLE@

#################### rxsubcom #########################
# Sources for rxsubcom
#
rxsubcom_SOURCES =  $(rexxapi_unix_dir)/SUBCOMCommand.cpp
#
# Preprocessor flags for rxsubcom
#
rxsubcom_CXXFLAGS  = $(COMMON_CPPFLAGS) \
                    -I$(lib_dir) \
                    -I$(api_dir) \
                    -I$(platform_api_dir) \
                    -I$(kernel_platform_unix_dir) \
                    -I$(messages_dir) \
                    -I$(rexxapi_unix_dir)
#
# Extra libraries for rxsubcom
#
rxsubcom_LDADD   = librexx.la librexxapi.la @ORX_LDADD_EXECUTABLE@
rxsubcom_LDFLAGS = @ORX_LDFLAGS_EXECUTABLE@

#################### rxdelipc #########################
# Sources for rxdelipc
#
rxdelipc_SOURCES =  $(rexxapi_unix_dir)/APIManagerShutdown.cpp
#
# Preprocessor flags for rxdelipc
#
rxdelipc_CXXFLAGS  = $(COMMON_CPPFLAGS) \
                     -I$(api_dir) \
                     -I$(platform_api_dir)
#
# Extra libraries for rxdelipc
#
rxdelipc_LDADD   = librexx.la librexxapi.la @ORX_LDADD_EXECUTABLE@
rxdelipc_LDFLAGS = @ORX_LDFLAGS_EXECUTABLE@

#################### librexxutil.so ################
# Preprocessor flags for librexxutil.so
#
librexxutil_la_CXXFLAGS = $(COMMON_CPPFLAGS) \
                          -I$(lib_dir) \
                          -I$(api_dir) \
                          -I$(platform_api_dir) \
                          -I$(kernel_dir) \
                          -I$(messages_dir) \
                          -I$(kernel_platform_common_dir) \
                          -I$(kernel_platform_unix_dir) \
                          -I$(rexxapi_unix_dir)
#
# Sources for librexxutil.so
#
librexxutil_la_SOURCES = $(rexutils_unix_dir)/rexxutil.cpp
#
# Extra libraries for librexxutil.so (maybe)
#
librexxutil_la_LIBADD  = @ORX_LIBADD_LIBRARY@
librexxutil_la_LDFLAGS = $(VERSION_INFO) @ORX_LDFLAGS_LIBRARY@

#################### librxmath.so ################
# Preprocessor flags for librxmath.so
#
librxmath_la_CXXFLAGS = $(COMMON_CPPFLAGS) \
                        -I$(top_srcdir)/lib \
                        -I$(api_dir) \
                        -I$(platform_api_dir) \
                        -I$(kernel_dir) \
                        -I$(messages_dir) \
                        -I$(kernel_platform_common_dir) \
                        -I$(runtime_dir) \
                        -I$(kernel_platform_unix_dir) \
                        -I$(rexxapi_unix_dir)
#
# Sources for librxmath.so
#
librxmath_la_SOURCES = $(rexutils_dir)/rxmath.cpp
#
# Extra libraries for librxmath.so (maybe)
#
librxmath_la_LIBADD  = @ORX_LIBADD_LIBRARY@
librxmath_la_LDFLAGS = $(VERSION_INFO) @ORX_LDFLAGS_LIBRARY@

#################### librxsock.so ################
# Preprocessor flags for librxsock.so
#
librxsock_la_CXXFLAGS = $(COMMON_CPPFLAGS) \
                      -I$(api_dir) \
                      -I$(platform_api_dir) \
                      -I$(rexxapi_unix_dir)
#
# Sources for librxsock.so
#
librxsock_la_SOURCES = $(rexutils_dir)/rxsock.cpp \
                       $(rexutils_dir)/rxsockfn.cpp
#
# Extra libraries for librxsock.so (maybe)
#
librxsock_la_LIBADD  = @ORX_LIBADD_LIBRARY@
librxsock_la_LDFLAGS = $(VERSION_INFO) @ORX_LDFLAGS_LIBRARY@

#################### librxregexp.so ################
# Preprocessor flags for librxregexp.so
#
librxregexp_la_CXXFLAGS = $(COMMON_CPPFLAGS) \
                          -I$(top_srcdir)/lib \
                          -I$(api_dir) \
                          -I$(platform_api_dir) \
                          -I$(kernel_dir) \
                          -I$(expression_dir) \
                          -I$(classes_dir) \
                          -I$(messages_dir) \
                          -I$(parser_dir) \
                          -I$(kernel_platform_common_dir) \
                          -I$(runtime_dir) \
                          -I$(kernel_platform_unix_dir) \
                          -I$(rexxapi_unix_dir)
#
# Sources for librxregexp.so
#
librxregexp_la_SOURCES = $(rxregexp_dir)/automaton.cpp \
                         $(rxregexp_dir)/dblqueue.cpp \
                         $(rxregexp_dir)/rxregexp.cpp
#
# Extra libraries for librxregexp.so (maybe)
#
librxregexp_la_LIBADD  = @ORX_LIBADD_LIBRARY@
librxregexp_la_LDFLAGS = $(VERSION_INFO) @ORX_LDFLAGS_LIBRARY@

#
# explicit targets:
#
rexx.cat: $(messages_dir)/gencat.inp
	-gencat $(@) $(messages_dir)/gencat.inp

$(messages_dir)/RexxMessageNumbers.h: $(messages_dir)/RexxMessageNumbers.xsl $(messageXML)
	sh ./xsl.sh $(@) $(messageXML) $(messages_dir)/RexxMessageNumbers.xsl

$(messages_dir)/RexxErrorCodes.h: $(messages_dir)/RexxErrorCodes.xsl $(messageXML)
	sh ./xsl.sh $(@) $(messageXML) $(messages_dir)/RexxErrorCodes.xsl

$(messages_dir)/RexxMessageTable.h: $(messages_dir)/RexxMessageTable.xsl $(messageXML)
	sh ./xsl.sh $(@) $(messageXML) $(messages_dir)/RexxMessageTable.xsl

$(messages_dir)/gencat.inp: $(kernel_platform_unix_dir)/UnixGencat.xsl $(messageXML)
	sh ./xsl.sh $(@) $(messageXML) $(kernel_platform_unix_dir)/UnixGencat.xsl

$(runtime_dir)/ClassTypeCodes.h: $(runtime_dir)/ClassTypeCodes.xsl $(classesXML)
	sh ./xsl.sh $(@) $(classesXML) $(runtime_dir)/ClassTypeCodes.xsl

$(runtime_dir)/PrimitiveBehaviourNames.h: $(runtime_dir)/PrimitiveBehaviourNames.xsl $(classesXML)
	sh ./xsl.sh $(@) $(classesXML) $(runtime_dir)/PrimitiveBehaviourNames.xsl

$(runtime_dir)/PrimitiveBehaviours.cpp: $(runtime_dir)/PrimitiveBehaviours.xsl $(classesXML)
	sh ./xsl.sh $(@) $(classesXML) $(runtime_dir)/PrimitiveBehaviours.xsl

$(runtime_dir)/VirtualFunctionTables.cpp: $(runtime_dir)/VirtualFunctionTable.xsl $(classesXML)
	sh ./xsl.sh $(@) $(classesXML) $(runtime_dir)/VirtualFunctionTable.xsl

rexx.img: rexx librexxutil.la $(core_classes) $(core_platform_classes)
	cp $(rexxclasses_dir)/*.orx .
	cp $(kernel_platform_unix_dir)/*.orx .
	./rexx -i
	rm -f *.orx

#
# distribution files
#
EXTRA_DIST = $(messages_dir)/RexxMessageNumbers.xsl \
             $(messages_dir)/RexxErrorCodes.xsl \
             $(messages_dir)/RexxMessageTable.xsl \
             $(messages_dir)/DocBookErrors.xsl \
             $(messages_dir)/DocErrorMessages.sgml \
             $(messages_dir)/gencat.inp \
             $(messageXML) \
             $(classesXML) \
             $(kernel_platform_unix_dir)/UnixGencat.xsl \
             $(runtime_dir)/ClassTypeCodes.xsl \
             $(runtime_dir)/PrimitiveBehaviourNames.xsl \
             $(runtime_dir)/PrimitiveBehaviours.xsl \
             $(runtime_dir)/VirtualFunctionTable.xsl \
             $(errorHeaders) \
             $(platform_unix_dir)/rexx.sh \
             $(platform_unix_dir)/rexx.csh \
             $(rxregexp_dir)/rxregexp.cls \
             $(kernel_platform_unix_dir)/UnixMethods.orx \
             $(kernel_platform_unix_dir)/PlatformObjects.orx \
             $(rexxclasses_dir) \
             $(classesHeaders) \
             $(expressionHeaders) \
             $(instructionsHeaders) \
             $(parserHeaders) \
             $(platformHeaders) \
             $(platformcommonHeaders) \
             $(runtimeHeaders) \
             $(streamlibraryHeaders) \
             $(libHeaders) \
             $(include_HEADERS) \
             $(rexxapiHeaders) \
             $(rexxapiunixHeaders) \
             $(rxregexpHeaders) \
             $(rexutilsHeaders) \
             $(man_MANS) \
             $(miscFiles) \
             $(windowsFiles) \
             $(windowsDefFiles)

dist-hook:
	rm -fr `find $(distdir) -name .svn`
	rm -fr `find $(distdir) -name CVS`
	rm -fr `find $(distdir) -name "*.bak"`
	rm -fr `find $(distdir) -name "*.o"`
	rm -fr `find $(distdir) -name "*.lo"`
#
# Force the timestamp of the message headers and the input to gencat
# to have a later date than the source XML, so Xalan is not needed to
# be executed by the user
	touch $(distdir)/kernel/messages/*.h
	touch $(distdir)/kernel/messages/DocErrorMessages.sgml
	touch $(distdir)/kernel/messages/gencat.inp
#
# Add oorexx.ver.incl, a file used by the Windows build, to the source
# distribution.
	sed -e '/ORX_BLD_LVL/s/1$$/$(SVN_REV)/;/ORX_VER_STR/s/\.1\"$$/.$(SVN_REV)\"/' oorexx.ver > oorexx.ver.incl
	echo 'SVN_REVSION=$(SVN_REV)' >> oorexx.ver.incl
	cp $(top_srcdir)/oorexx.ver.incl $(distdir)
	rm -f oorexx.ver.incl

clean-local: am_config_clean-local

am_config_clean-local:
	rm -fr $(PACKAGE)-*.tar.gz
	rm -fr $(PACKAGE)-*.zip
	rm -fr $(top_srcdir)/rpm

install-exec-local:
	echo $(DESTDIR) is destdir
	echo $(prefix) is prefix
	echo $(install_sh_PROGRAM)
	$(install_sh_SCRIPT) rexx.img $(DESTDIR)$(prefix)/bin/rexx.img
	if test -z "$(GENCAT)"; then \
		echo "No rexx.cat to install"; \
	else \
		$(install_sh_SCRIPT) rexx.cat $(DESTDIR)$(catdir)/rexx.cat; \
	fi
	$(install_sh_SCRIPT) $(build_platform_unix_dir)/oorexx-config $(DESTDIR)$(prefix)/bin/oorexx-config
	$(install_sh_SCRIPT) $(rxregexp_dir)/rxregexp.cls $(DESTDIR)$(prefix)/bin/rxregexp.cls
	$(install_sh_SCRIPT) $(rexutils_dir)/rxftp.cls $(DESTDIR)$(prefix)/bin/rxftp.cls
	for a in $(samples_dir)/*.rex; do \
		bn=`basename $$a`; \
		$(install_sh_SCRIPT) $$a $(DESTDIR)$(prefix)/share/ooRexx/$$bn; \
	done;
	$(install_sh_SCRIPT) $(samples_dir)/readme $(DESTDIR)$(prefix)/share/ooRexx/readme

package:
	./rexx $(top_srcdir)/platform/unix/makepkg.rex @PACKAGETYPE@ $(ORX_MAJOR).$(ORX_MINOR).$(ORX_MOD_LVL) $(target) $(prefix) $(DESTDIR)

rpm: dist
	rm -fr $(top_srcdir)/rpm
	mkdir rpm
	mkdir rpm/BUILD
	mkdir rpm/RPMS
	mkdir rpm/SOURCES
	mkdir rpm/SPECS
	mkdir rpm/SRPMS
	rpmbuild -ba platform/unix/oorexx.spec < /dev/null;

deb:
	rm -fr $(top_srcdir)/debian
	mkdir $(top_srcdir)/debian
# Generate a current change log in the required Debian format
	sed -n -e '1,2p' $(top_srcdir)/platform/unix/debian/changelog > ${top_srcdir}/debian/changelog
	sed -e 's/.*/  &/' CHANGES >> ${top_srcdir}/debian/changelog
	sed -e "1,3d; 4,6s/date/`date -R`/" $(top_srcdir)/platform/unix/debian/changelog >>  ${top_srcdir}/debian/changelog
# Just copy the rest of the files
	cp $(top_srcdir)/platform/unix/debian/compat    ${top_srcdir}/debian
	cp $(top_srcdir)/platform/unix/debian/control   ${top_srcdir}/debian
	cp $(top_srcdir)/platform/unix/debian/copyright ${top_srcdir}/debian
	cp $(top_srcdir)/platform/unix/debian/dirs      ${top_srcdir}/debian
	cp $(top_srcdir)/platform/unix/debian/docs      ${top_srcdir}/debian
	cp $(top_srcdir)/platform/unix/debian/postinst  ${top_srcdir}/debian
	cp $(top_srcdir)/platform/unix/debian/postrm    ${top_srcdir}/debian
	cp $(top_srcdir)/platform/unix/debian/prerm     ${top_srcdir}/debian
	cp $(top_srcdir)/platform/unix/debian/rules     ${top_srcdir}/debian
# Make the binaries
	make all
# Install into the debian package directory
	fakeroot dh_clean -k
	fakeroot dh_installdirs
	fakeroot make install DESTDIR=`pwd`/debian/oorexx
# Do the packaging steps
	fakeroot dh_installchangelogs CHANGES
	fakeroot dh_installdocs
	fakeroot dh_installexamples
	fakeroot dh_installman
	fakeroot dh_link
	fakeroot dh_strip
	fakeroot dh_compress
	fakeroot dh_fixperms
	fakeroot dh_installdeb
	fakeroot dh_shlibdeps
	fakeroot dh_gencontrol
	fakeroot dh_md5sums
	fakeroot dh_builddeb

