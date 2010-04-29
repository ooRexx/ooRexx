#/*----------------------------------------------------------------------------*/
#/*                                                                            */
#/* Copyright (c) 2005-2008 Rexx Language Association. All rights reserved.    */
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

DIST_SUBDIRS 		= samples

# get version details
include $(top_srcdir)/oorexx.ver

VERSION = $(ORX_MAJOR).$(ORX_MINOR).$(ORX_MOD_LVL)
#driver_name = orexx-$(ORX_MAJOR).$(ORX_MINOR).$(ORX_MOD_LVL).0-$(ORX_BLD_LVL)

# Please note that the version-info has nothing to do with the release version.
# You need to know exactly what you are doing in order to get this correct.
VERSION_INFO = -version-info ${ORX_CURRENT}:${ORX_REVISION}:${ORX_AGE}

# These are the platform independent directories
build_common_dir = $(top_srcdir)/common
build_lib_dir = $(top_srcdir)/lib
build_platform_dir = $(top_srcdir)/platform
build_rexxapi_dir = $(top_srcdir)/rexxapi
build_rexxapi_client_dir = $(build_rexxapi_dir)/client
build_rexxapi_common_dir = $(build_rexxapi_dir)/common
build_rexxapi_server_dir = $(build_rexxapi_dir)/server
build_interpreter_dir = $(top_srcdir)/interpreter
build_interpreter_api_dir = $(build_interpreter_dir)/api
build_behaviour_dir = $(build_interpreter_dir)/behaviour
build_classes_dir = $(build_interpreter_dir)/classes
build_classes_support_dir = $(build_interpreter_dir)/classes/support
build_concurrency_dir = $(build_interpreter_dir)/concurrency
build_execution_dir = $(build_interpreter_dir)/execution
build_expression_dir = $(build_interpreter_dir)/expression
build_instructions_dir = $(build_interpreter_dir)/instructions
build_memory_dir = $(build_interpreter_dir)/memory
build_messages_dir = $(build_interpreter_dir)/messages
build_package_dir = $(build_interpreter_dir)/package
build_parser_dir = $(build_interpreter_dir)/parser
build_interpreter_common_dir = $(build_interpreter_dir)/platform/common
build_rexxclasses_dir = $(build_interpreter_dir)/RexxClasses
build_runtime_dir = $(build_interpreter_dir)/runtime
build_streamlibrary_dir = $(build_interpreter_dir)/streamLibrary
build_extensions_dir = $(top_srcdir)/extensions
build_extensions_rexutils_dir = $(build_extensions_dir)/rexxutil
build_extensions_rxftp_dir = $(build_extensions_dir)/rxftp
build_extensions_csvstream_dir = $(build_extensions_dir)/csvStream
build_extensions_rxmath_dir = $(build_extensions_dir)/rxmath
build_extensions_rxregexp_dir = $(build_extensions_dir)/rxregexp
build_extensions_rxsock_dir = $(build_extensions_dir)/rxsock
build_extensions_hostemu_dir = $(build_extensions_dir)/hostemu
build_utilities_dir = $(top_srcdir)/utilities
build_utilities_rexximage_dir = $(build_utilities_dir)/rexximage
build_api_dir = $(top_srcdir)/api
build_samples_dir = $(top_srcdir)/samples


# These are the *nix directories
common_unix_dir = $(build_common_dir)/platform/unix
rexxapi_client_unix_dir = $(build_rexxapi_client_dir)/platform/unix
rexxapi_server_unix_dir = $(build_rexxapi_server_dir)/platform/unix
rexxapi_common_unix_dir = $(build_rexxapi_common_dir)/platform/unix
interpreter_unix_dir = $(build_interpreter_dir)/platform/unix
extensions_unix_dir = $(build_extensions_dir)/platform/unix
extensions_rexutils_unix_dir = $(build_extensions_rexutils_dir)/platform/unix
extensions_hostemu_unix_dir = $(build_extensions_hostemu_dir)/platform/unix
utilities_rexx_unix_dir = $(build_utilities_dir)/rexx/platform/unix
utilities_rexxc_unix_dir = $(build_utilities_dir)/rexxc/platform/unix
utilities_rxqueue_unix_dir = $(build_utilities_dir)/rxqueue/platform/unix
utilities_rxsubcom_unix_dir = $(build_utilities_dir)/rxsubcom/platform/unix
api_unix_dir = $(build_api_dir)/platform/unix
platform_unix_dir = $(build_platform_dir)/unix
samples_unix_dir = $(build_samples_dir)/unix


# These are the Windows directories
common_windows_dir = $(build_common_dir)/platform/windows
rexxapi_client_windows_dir = $(build_rexxapi_client_dir)/platform/windows
rexxapi_server_windows_dir = $(build_rexxapi_server_dir)/platform/windows
rexxapi_common_windows_dir = $(build_rexxapi_common_dir)/platform/windows
interpreter_windows_dir = $(build_interpreter_dir)/platform/windows
extensions_windows_dir = $(build_extensions_dir)/platform/windows
extensions_windows_ole_dir = $(build_extensions_dir)/platform/windows/ole
extensions_windows_oodialog_dir = $(build_extensions_dir)/platform/windows/oodialog
extensions_windows_orxscrpt_dir = $(build_extensions_dir)/platform/windows/orxscrpt
extensions_windows_rxwinsys_dir = $(build_extensions_dir)/platform/windows/rxwinsys
extensions_rexutils_windows_dir = $(build_extensions_rexutils_dir)/platform/windows
utilities_rexxhide_windows_dir = $(build_utilities_dir)/platform/windows/rexxhide
utilities_rexxpaws_windows_dir = $(build_utilities_dir)/platform/windows/rexxpaws
utilities_rexx_windows_dir = $(build_utilities_dir)/rexx/platform/windows
utilities_rexxc_windows_dir = $(build_utilities_dir)/rexxc/platform/windows
utilities_rxqueue_windows_dir = $(build_utilities_dir)/rxqueue/platform/windows
utilities_rxsubcom_windows_dir = $(build_utilities_dir)/rxsubcom/platform/windows
api_windows_dir = $(build_api_dir)/platform/windows
platform_windows_dir = $(build_platform_dir)/windows
samples_windows_dir = $(build_samples_dir)/windows


# These are the platform specific build directories
build_common_platform_dir = $(common_@ORX_PLATFORM@_dir)
build_rexxapi_client_platform_dir = $(rexxapi_client_@ORX_PLATFORM@_dir)
build_rexxapi_server_platform_dir = $(rexxapi_server_@ORX_PLATFORM@_dir)
build_rexxapi_common_platform_dir = $(rexxapi_common_@ORX_PLATFORM@_dir)
build_interpreter_platform_dir = $(interpreter_@ORX_PLATFORM@_dir)
build_utilities_rexx_platform_dir = $(utilities_rexx_@ORX_PLATFORM@_dir)
build_utilities_rexxc_platform_dir = $(utilities_rexxc_@ORX_PLATFORM@_dir)
build_utilities_rxqueue_platform_dir = $(utilities_rxqueue_@ORX_PLATFORM@_dir)
build_utilities_rxsubcom_platform_dir = $(utilities_rxsubcom_@ORX_PLATFORM@_dir)
build_extensions_rexutils_platform_dir = $(extensions_rexutils_@ORX_PLATFORM@_dir)
build_api_platform_dir = $(api_@ORX_PLATFORM@_dir)
build_os_platform_dir = $(platform_@ORX_PLATFORM@_dir)
build_samples_platform_dir = $(samples_@ORX_PLATFORM@_dir)

catdir = $(prefix)/bin

COMMON_CPPFLAGS = -DORX_VER=$(ORX_MAJOR) -DORX_REL=$(ORX_MINOR) -DORX_MOD=$(ORX_MOD_LVL) -DORX_FIX=0 \
                  -DORX_SYS_STR=\"@ORX_SYS_STR@\" \
                  -DORX_CATDIR=\"$(catdir)\" \
                  -DORX_SHARED_LIBRARY_EXT=\"@ORX_SHARED_LIBRARY_EXT@\"

messageXML = $(build_messages_dir)/rexxmsg.xml

classesXML = $(build_behaviour_dir)/PrimitiveClasses.xml

#
# header files for source distribution
#

# Common headers
commonHeaders = $(build_common_dir)/SynchronizedBlock.hpp \
                $(build_common_dir)/Utilities.hpp \
                $(build_common_platform_dir)/SysFile.hpp \
                $(build_common_platform_dir)/SysLibrary.hpp \
                $(build_common_platform_dir)/SysSemaphore.hpp \
                $(build_common_platform_dir)/SysThread.hpp
# Interpreter headers
interpreterApiHeaders = $(build_interpreter_api_dir)/ContextApi.hpp
behaviourHeaders = $(build_behaviour_dir)/ClassTypeCodes.h \
                   $(build_behaviour_dir)/PrimitiveBehaviourNames.h \
                   $(build_behaviour_dir)/RexxBehaviour.hpp
classesHeaders = $(build_classes_dir)/ArrayClass.hpp \
                 $(build_classes_dir)/BufferClass.hpp \
                 $(build_classes_dir)/ClassClass.hpp \
                 $(build_classes_dir)/ContextClass.hpp \
                 $(build_classes_dir)/DirectoryClass.hpp \
                 $(build_classes_dir)/IdentityTableClass.hpp \
                 $(build_classes_dir)/IntegerClass.hpp \
                 $(build_classes_dir)/ListClass.hpp \
                 $(build_classes_dir)/MessageClass.hpp \
                 $(build_classes_dir)/MethodClass.hpp \
                 $(build_classes_dir)/MutableBufferClass.hpp \
                 $(build_classes_dir)/NumberStringClass.hpp \
                 $(build_classes_dir)/NumberStringMath.hpp \
                 $(build_classes_dir)/ObjectClass.hpp \
                 $(build_classes_dir)/PackageClass.hpp \
                 $(build_classes_dir)/PointerClass.hpp \
                 $(build_classes_dir)/QueueClass.hpp \
                 $(build_classes_dir)/RelationClass.hpp \
                 $(build_classes_dir)/RoutineClass.hpp \
                 $(build_classes_dir)/StackClass.hpp \
                 $(build_classes_dir)/StemClass.hpp \
                 $(build_classes_dir)/StringClass.hpp \
                 $(build_classes_dir)/SupplierClass.hpp \
                 $(build_classes_dir)/TableClass.hpp \
                 $(build_classes_dir)/WeakReferenceClass.hpp \
                 $(build_classes_dir)/StackFrameClass.hpp \
                 $(build_classes_dir)/ExceptionClass.hpp
classesSupportHeaders = $(build_classes_support_dir)/ProgramMetaData.hpp \
                        $(build_classes_support_dir)/RexxCollection.hpp \
                        $(build_classes_support_dir)/RexxCompoundElement.hpp \
                        $(build_classes_support_dir)/RexxCompoundTable.hpp \
                        $(build_classes_support_dir)/RexxCompoundTail.hpp \
                        $(build_classes_support_dir)/RexxDateTime.hpp \
                        $(build_classes_support_dir)/RexxHashTable.hpp \
                        $(build_classes_support_dir)/RexxListTable.hpp \
                        $(build_classes_support_dir)/StringUtil.hpp
concurrencyHeaders = $(build_concurrency_dir)/ActivationApiContexts.hpp \
                     $(build_concurrency_dir)/ActivationFrame.hpp \
                     $(build_concurrency_dir)/ActivityDispatcher.hpp \
                     $(build_concurrency_dir)/ActivityManager.hpp \
                     $(build_concurrency_dir)/CallbackDispatcher.hpp \
                     $(build_concurrency_dir)/TrappingDispatcher.hpp \
                     $(build_concurrency_dir)/ExitHandler.hpp \
                     $(build_concurrency_dir)/CommandHandler.hpp \
                     $(build_concurrency_dir)/MessageDispatcher.hpp \
                     $(build_concurrency_dir)/RexxActivity.hpp \
                     $(build_concurrency_dir)/RexxStartDispatcher.hpp \
                     $(build_concurrency_dir)/TranslateDispatcher.hpp
executionHeaders = $(build_execution_dir)/CPPCode.hpp \
                   $(build_execution_dir)/RexxActivation.hpp \
                   $(build_execution_dir)/RexxActivationStack.hpp \
                   $(build_execution_dir)/RexxCode.hpp \
                   $(build_execution_dir)/RexxLocalVariables.hpp \
                   $(build_execution_dir)/RexxNativeActivation.hpp \
                   $(build_execution_dir)/RexxNativeCode.hpp \
                   $(build_execution_dir)/RexxVariable.hpp \
                   $(build_execution_dir)/RexxVariableDictionary.hpp \
                   $(build_execution_dir)/SecurityManager.hpp
expressionHeaders = $(build_expression_dir)/BuiltinFunctions.hpp \
                    $(build_expression_dir)/ExpressionBaseVariable.hpp \
                    $(build_expression_dir)/ExpressionCompoundVariable.hpp \
                    $(build_expression_dir)/ExpressionDotVariable.hpp \
                    $(build_expression_dir)/ExpressionFunction.hpp \
                    $(build_expression_dir)/ExpressionLogical.hpp \
                    $(build_expression_dir)/ExpressionMessage.hpp \
                    $(build_expression_dir)/ExpressionOperator.hpp \
                    $(build_expression_dir)/ExpressionStack.hpp \
                    $(build_expression_dir)/ExpressionStem.hpp \
                    $(build_expression_dir)/ExpressionVariable.hpp \
                    $(build_expression_dir)/IndirectVariableReference.hpp
instructionsHeaders = $(build_instructions_dir)/AddressInstruction.hpp \
                      $(build_instructions_dir)/AssignmentInstruction.hpp \
                      $(build_instructions_dir)/CallInstruction.hpp \
                      $(build_instructions_dir)/ClassDirective.hpp \
                      $(build_instructions_dir)/CommandInstruction.hpp \
                      $(build_instructions_dir)/DoBlock.hpp \
                      $(build_instructions_dir)/DoInstruction.hpp \
                      $(build_instructions_dir)/DropInstruction.hpp \
                      $(build_instructions_dir)/ElseInstruction.hpp \
                      $(build_instructions_dir)/EndIf.hpp \
                      $(build_instructions_dir)/EndInstruction.hpp \
                      $(build_instructions_dir)/ExitInstruction.hpp \
                      $(build_instructions_dir)/ExposeInstruction.hpp \
                      $(build_instructions_dir)/ForwardInstruction.hpp \
                      $(build_instructions_dir)/GuardInstruction.hpp \
                      $(build_instructions_dir)/IfInstruction.hpp \
                      $(build_instructions_dir)/InterpretInstruction.hpp \
                      $(build_instructions_dir)/LabelInstruction.hpp \
                      $(build_instructions_dir)/LeaveInstruction.hpp \
                      $(build_instructions_dir)/LibraryDirective.hpp \
                      $(build_instructions_dir)/MessageInstruction.hpp \
                      $(build_instructions_dir)/NopInstruction.hpp \
                      $(build_instructions_dir)/NumericInstruction.hpp \
                      $(build_instructions_dir)/OptionsInstruction.hpp \
                      $(build_instructions_dir)/OtherwiseInstruction.hpp \
                      $(build_instructions_dir)/ParseInstruction.hpp \
                      $(build_instructions_dir)/ParseTarget.hpp \
                      $(build_instructions_dir)/ParseTrigger.hpp \
                      $(build_instructions_dir)/ProcedureInstruction.hpp \
                      $(build_instructions_dir)/QueueInstruction.hpp \
                      $(build_instructions_dir)/RaiseInstruction.hpp \
                      $(build_instructions_dir)/ReplyInstruction.hpp \
                      $(build_instructions_dir)/RequiresDirective.hpp \
                      $(build_instructions_dir)/ReturnInstruction.hpp \
                      $(build_instructions_dir)/RexxDirective.hpp \
                      $(build_instructions_dir)/RexxInstruction.hpp \
                      $(build_instructions_dir)/SayInstruction.hpp \
                      $(build_instructions_dir)/SelectInstruction.hpp \
                      $(build_instructions_dir)/SignalInstruction.hpp \
                      $(build_instructions_dir)/ThenInstruction.hpp \
                      $(build_instructions_dir)/TraceInstruction.hpp \
                      $(build_instructions_dir)/UseStrictInstruction.hpp
memoryHeaders = $(build_memory_dir)/DeadObject.hpp \
                $(build_memory_dir)/MemorySegment.hpp \
                $(build_memory_dir)/MemoryStats.hpp \
                $(build_memory_dir)/ProtectedObject.hpp \
                $(build_memory_dir)/GlobalProtectedObject.hpp \
                $(build_memory_dir)/RexxEnvelope.hpp \
                $(build_memory_dir)/RexxInternalStack.hpp \
                $(build_memory_dir)/RexxMemory.hpp \
                $(build_memory_dir)/RexxSmartBuffer.hpp \
                $(build_memory_dir)/UninitDispatcher.hpp
errorHeaders = $(build_messages_dir)/RexxMessageNumbers.h \
               $(build_messages_dir)/RexxErrorCodes.h \
               $(build_messages_dir)/RexxMessageTable.h \
               $(build_api_dir)/oorexxerrors.h
packageHeaders = $(build_package_dir)/LibraryPackage.hpp \
                 $(build_package_dir)/PackageManager.hpp
parserHeaders = $(build_parser_dir)/Clause.hpp \
                $(build_parser_dir)/SourceFile.hpp \
                $(build_parser_dir)/SourceLocation.hpp \
                $(build_parser_dir)/Token.hpp
platformHeaders = $(build_interpreter_platform_dir)/APIDefinitions.h \
                  $(build_interpreter_platform_dir)/PlatformDefinitions.h \
                  $(build_interpreter_platform_dir)/SysActivity.hpp \
                  $(build_interpreter_platform_dir)/SysFileSystem.hpp \
                  $(build_interpreter_platform_dir)/SysInterpreterInstance.hpp \
                  $(build_interpreter_platform_dir)/SysNativeFunctions.h \
                  $(build_interpreter_platform_dir)/SysNativeMethods.h \
                  $(build_interpreter_platform_dir)/SystemInterpreter.hpp
runtimeHeaders = $(build_runtime_dir)/GlobalNames.h \
                 $(build_runtime_dir)/Interpreter.hpp \
                 $(build_runtime_dir)/InterpreterInstance.hpp \
                 $(build_runtime_dir)/NativeFunctions.h \
                 $(build_runtime_dir)/NativeMethods.h \
                 $(build_runtime_dir)/Numerics.hpp \
                 $(build_runtime_dir)/RexxConstants.hpp \
                 $(build_runtime_dir)/RexxCore.h \
                 $(build_runtime_dir)/RexxMisc.hpp
streamlibraryHeaders = $(build_streamlibrary_dir)/StreamCommandParser.h \
                       $(build_streamlibrary_dir)/StreamNative.hpp
libHeaders = $(build_lib_dir)/RexxInternalApis.h \
             $(build_lib_dir)/RexxPlatformDefinitions.h
rexxapiClientHeaders = $(build_rexxapi_client_dir)/ClientMessage.hpp \
                 $(build_rexxapi_client_dir)/LocalAPIContext.hpp \
                 $(build_rexxapi_client_dir)/LocalAPIManager.hpp \
                 $(build_rexxapi_client_dir)/LocalAPISubsystem.hpp \
                 $(build_rexxapi_client_dir)/LocalMacroSpaceManager.hpp \
                 $(build_rexxapi_client_dir)/LocalQueueManager.hpp \
                 $(build_rexxapi_client_dir)/LocalRegistrationManager.hpp \
                 $(build_rexxapi_client_dir)/RexxAPI.h \
                 $(build_rexxapi_client_platform_dir)/SysLocalAPIManager.hpp
rexxapiCommonHeaders = $(build_rexxapi_common_dir)/Encodings.hpp \
                 $(build_rexxapi_common_dir)/Rxstring.hpp \
                 $(build_rexxapi_common_dir)/ServiceException.hpp \
                 $(build_rexxapi_common_dir)/ServiceMessage.hpp \
                 $(build_rexxapi_common_platform_dir)/SysAPIManager.hpp \
                 $(build_rexxapi_common_platform_dir)/SysCSStream.hpp \
                 $(build_rexxapi_common_platform_dir)/SysProcess.hpp
rexxapiServerHeaders = $(build_rexxapi_server_dir)/APIServer.hpp \
                 $(build_rexxapi_server_dir)/APIServerInstance.hpp \
                 $(build_rexxapi_server_dir)/APIServerThread.hpp \
                 $(build_rexxapi_server_dir)/MacroSpaceManager.hpp \
                 $(build_rexxapi_server_dir)/QueueManager.hpp \
                 $(build_rexxapi_server_dir)/RegistrationManager.hpp
# Extension headers
rxregexpHeaders = $(build_extensions_rxregexp_dir)/automaton.hpp \
                  $(build_extensions_rxregexp_dir)/dblqueue.hpp \
                  $(build_extensions_rxregexp_dir)/regexp.hpp
rxsockHeaders = $(build_extensions_rxsock_dir)/rxsock.h
hostemuHeaders = $(build_extensions_hostemu_dir)/hostemu.h
# Platform specific files
windowsMiscFiles =
unixMiscFiles = $(interpreter_unix_dir)/*.cat \
                $(rexxapi_server_unix_dir)/rxapid \
                $(build_messages_dir)/gencat.inp \
                $(build_os_platform_dir)/rexx.sh \
                $(build_os_platform_dir)/rexx.csh \
                $(build_os_platform_dir)/*.1 \
                $(build_os_platform_dir)/oorexx.spec \
                $(build_os_platform_dir)/oorexx-config
commonMiscFiles = $(top_srcdir)/CPLv1.0.txt \
                  $(top_srcdir)/CHANGES \
                  $(top_srcdir)/ReleaseNotes \
                  $(top_srcdir)/CONTRIBUTORS \
                  $(top_srcdir)/NOTICE \
                  $(top_srcdir)/unix-like-build.txt \
                  $(top_srcdir)/windows-build.txt \
                  $(top_srcdir)/oorexx.ver \
                  $(build_extensions_csvstream_dir)/csvStream.cls \
                  $(build_extensions_rxftp_dir)/rxftp.cls \
                  $(build_extensions_csvstream_dir)/csvStream.cls \
                  $(build_extensions_rxsock_dir)/socket.cls \
                  $(build_extensions_rxsock_dir)/streamsocket.cls \
                  $(build_extensions_rxsock_dir)/mime.cls \
                  $(build_extensions_rxsock_dir)/smtp.cls \
                  $(build_extensions_rxregexp_dir)/*.cls \
                  $(build_messages_dir)/*.xsl \
                  $(build_messages_dir)/*.sgml \
                  $(build_messages_dir)/*.xml \
                  $(build_behaviour_dir)/*.xsl \
                  $(build_behaviour_dir)/*.xml \
                  $(commonHeaders) \
                  $(interpreterApiHeaders) \
                  $(behaviourHeaders) \
                  $(classesHeaders) \
                  $(classesSupportHeaders) \
                  $(concurrencyHeaders) \
                  $(executionHeaders) \
                  $(expressionHeaders) \
                  $(instructionsHeaders) \
                  $(memoryHeaders) \
                  $(errorHeaders) \
                  $(packageHeaders) \
                  $(parserHeaders) \
                  $(platformHeaders) \
                  $(build_interpreter_platform_dir)/*.orx \
                  $(build_interpreter_platform_dir)/*.xsl \
                  $(build_rexxclasses_dir)/*.orx \
                  $(runtimeHeaders) \
                  $(streamlibraryHeaders) \
                  $(libHeaders) \
                  $(rexxapiClientHeaders) \
                  $(rexxapiCommonHeaders) \
                  $(rexxapiServerHeaders) \
                  $(rxregexpHeaders) \
                  $(rxsockHeaders) \
                  $(hostemuHeaders)

#
# These error message files are built BEFORE any other target
#
BUILT_SOURCES = $(errorHeaders) \
                rexx.img @REXXCAT@ ooTest $(build_messages_dir)/DocErrorMessages.sgml
#
# Binary targets
#
bin_PROGRAMS = rxapi rexximage rexx rexxc rxqueue rxsubcom

#
# Shared library targets
#
pkglib_LTLIBRARIES = librexxapi.la librexx.la librexxutil.la librxmath.la librxregexp.la librxsock.la libhostemu.la

#
# Scripts to install
#
pkgdata_SCRIPTS = $(top_srcdir)/platform/unix/rexx.sh $(top_srcdir)/platform/unix/rexx.csh

#
# headers to install
#
include_HEADERS = $(build_api_dir)/rexx.h \
                  $(build_api_dir)/rexxapidefs.h \
                  $(build_api_dir)/oorexxapi.h \
                  $(build_api_dir)/oorexxerrors.h \
                  $(build_api_platform_dir)/rexxapitypes.h \
                  $(build_api_platform_dir)/rexxplatformapis.h \
                  $(build_api_platform_dir)/rexxplatformdefs.h

#
# man pages to install
#
man_MANS = $(top_srcdir)/platform/unix/rexx.1 \
           $(top_srcdir)/platform/unix/rexxc.1 \
           $(top_srcdir)/platform/unix/rxsubcom.1 \
           $(top_srcdir)/platform/unix/rxqueue.1 \
           $(top_srcdir)/platform/unix/oorexx-config.1

#################### librexxapi.so ##########################
# Sources for librexxapi.so
#
librexxapi_la_SOURCES = $(build_rexxapi_client_dir)/ClientMessage.cpp \
                        $(build_rexxapi_client_dir)/LocalAPIContext.cpp \
                        $(build_rexxapi_client_dir)/LocalAPIManager.cpp \
                        $(build_rexxapi_client_dir)/LocalMacroSpaceManager.cpp \
                        $(build_rexxapi_client_dir)/LocalQueueManager.cpp \
                        $(build_rexxapi_client_dir)/LocalRegistrationManager.cpp \
                        $(build_rexxapi_client_dir)/MacroSpaceApi.cpp \
                        $(build_rexxapi_client_dir)/QueuesAPI.cpp \
                        $(build_rexxapi_client_dir)/RegistrationAPI.cpp \
                        $(build_rexxapi_client_dir)/platform/unix/SysLegacyAPI.cpp \
                        $(build_rexxapi_client_dir)/platform/unix/SysLocalAPIManager.cpp \
                        $(build_rexxapi_common_dir)/ServiceMessage.cpp \
                        $(build_rexxapi_common_platform_dir)/SysAPIManager.cpp \
                        $(build_rexxapi_common_platform_dir)/SysCSStream.cpp \
                        $(build_rexxapi_common_platform_dir)/SysProcess.cpp \
                        $(build_common_dir)/Utilities.cpp \
                        $(build_common_platform_dir)/SysSemaphore.cpp \
                        $(build_common_platform_dir)/SysLibrary.cpp \
                        $(build_common_platform_dir)/SysFile.cpp \
                        $(build_common_platform_dir)/SysThread.cpp

#
# Preprocessor flags for librexxapi.so
#
librexxapi_la_CXXFLAGS  = $(COMMON_CPPFLAGS) \
                          -I$(build_lib_dir) \
                          -I$(build_api_dir) \
                          -I$(build_api_platform_dir) \
                          -I$(build_rexxapi_client_dir) \
                          -I$(build_rexxapi_client_platform_dir) \
                          -I$(build_rexxapi_common_dir) \
                          -I$(build_rexxapi_common_platform_dir) \
                          -I$(build_common_dir) \
                          -I$(build_common_platform_dir)

librexxapi_la_LDFLAGS = $(VERSION_INFO)

#################### librexx.so ##########################
# Sources for librexx.so
#
interpreter_api_sources = $(build_interpreter_api_dir)/CallContextStubs.cpp \
                     $(build_interpreter_api_dir)/InterpreterAPI.cpp \
                     $(build_interpreter_api_dir)/InterpreterInstanceStubs.cpp \
                     $(build_interpreter_api_dir)/MethodContextStubs.cpp \
                     $(build_interpreter_api_dir)/ThreadContextStubs.cpp
classes_support_sources = $(build_classes_support_dir)/ProgramMetaData.cpp \
                          $(build_classes_support_dir)/RexxCollection.cpp \
                          $(build_classes_support_dir)/RexxCompoundElement.cpp \
                          $(build_classes_support_dir)/RexxCompoundTable.cpp \
                          $(build_classes_support_dir)/RexxCompoundTail.cpp \
                          $(build_classes_support_dir)/RexxDateTime.cpp \
                          $(build_classes_support_dir)/RexxHashTable.cpp \
                          $(build_classes_support_dir)/RexxListTable.cpp \
                          $(build_classes_support_dir)/StringUtil.cpp
classes_sources = $(build_classes_dir)/ArrayClass.cpp \
                  $(build_classes_dir)/BufferClass.cpp \
                  $(build_classes_dir)/ClassClass.cpp \
                  $(build_classes_dir)/ContextClass.cpp \
                  $(build_classes_dir)/DirectoryClass.cpp \
                  $(build_classes_dir)/IntegerClass.cpp \
                  $(build_classes_dir)/ListClass.cpp \
                  $(build_classes_dir)/MessageClass.cpp \
                  $(build_classes_dir)/MethodClass.cpp \
                  $(build_classes_dir)/MutableBufferClass.cpp \
                  $(build_classes_dir)/NumberStringClass.cpp \
                  $(build_classes_dir)/NumberStringMath.cpp \
                  $(build_classes_dir)/NumberStringMath2.cpp \
                  $(build_classes_dir)/ObjectClass.cpp \
                  $(build_classes_dir)/PackageClass.cpp \
                  $(build_classes_dir)/PointerClass.cpp \
                  $(build_classes_dir)/QueueClass.cpp \
                  $(build_classes_dir)/RelationClass.cpp \
                  $(build_classes_dir)/RoutineClass.cpp \
                  $(build_classes_dir)/RexxQueueMethods.cpp \
                  $(build_classes_dir)/StackClass.cpp \
                  $(build_classes_dir)/StemClass.cpp \
                  $(build_classes_dir)/StringClass.cpp \
                  $(build_classes_dir)/StringClassBit.cpp \
                  $(build_classes_dir)/StringClassConversion.cpp \
                  $(build_classes_dir)/StringClassMisc.cpp \
                  $(build_classes_dir)/StringClassSub.cpp \
                  $(build_classes_dir)/StringClassUtil.cpp \
                  $(build_classes_dir)/StringClassWord.cpp \
                  $(build_classes_dir)/SupplierClass.cpp \
                  $(build_classes_dir)/TableClass.cpp \
                  $(build_classes_dir)/IdentityTableClass.cpp \
                  $(build_classes_dir)/WeakReferenceClass.cpp \
                  $(build_classes_dir)/StackFrameClass.cpp \
                  $(build_classes_dir)/ExceptionClass.cpp
package_sources = $(build_package_dir)/LibraryPackage.cpp \
                  $(build_package_dir)/PackageManager.cpp
memory_sources = $(build_memory_dir)/DeadObject.cpp \
                 $(build_memory_dir)/MemorySegment.cpp \
                 $(build_memory_dir)/MemoryStats.cpp \
                 $(build_memory_dir)/ProtectedObject.cpp \
                 $(build_memory_dir)/RexxEnvelope.cpp \
                 $(build_memory_dir)/RexxInternalStack.cpp \
                 $(build_memory_dir)/RexxSmartBuffer.cpp \
                 $(build_memory_dir)/UninitDispatcher.cpp \
                 $(build_memory_dir)/Setup.cpp \
                 $(build_memory_dir)/RexxMemory.cpp
execution_sources = $(build_execution_dir)/CPPCode.cpp \
                    $(build_execution_dir)/RexxActivation.cpp \
                    $(build_execution_dir)/RexxActivationStack.cpp \
                    $(build_execution_dir)/RexxCode.cpp \
                    $(build_execution_dir)/RexxLocalVariables.cpp \
                    $(build_execution_dir)/RexxNativeActivation.cpp \
                    $(build_execution_dir)/RexxNativeCode.cpp \
                    $(build_execution_dir)/RexxVariable.cpp \
                    $(build_execution_dir)/RexxVariableDictionary.cpp \
                    $(build_execution_dir)/SecurityManager.cpp
behaviour_sources = $(build_behaviour_dir)/PrimitiveBehaviours.cpp \
                    $(build_behaviour_dir)/RexxBehaviour.cpp \
                    $(build_behaviour_dir)/VirtualFunctionTable.cpp
concurrency_sources = $(build_concurrency_dir)/ActivityDispatcher.cpp \
                      $(build_concurrency_dir)/ActivityManager.cpp \
                      $(build_concurrency_dir)/ActivationFrame.cpp \
                      $(build_concurrency_dir)/CallbackDispatcher.cpp \
                      $(build_concurrency_dir)/TrappingDispatcher.cpp \
                      $(build_concurrency_dir)/ExitHandler.cpp \
                      $(build_concurrency_dir)/CommandHandler.cpp \
                      $(build_concurrency_dir)/MessageDispatcher.cpp \
                      $(build_concurrency_dir)/RexxActivity.cpp \
                      $(build_concurrency_dir)/RexxStartDispatcher.cpp \
                      $(build_concurrency_dir)/TranslateDispatcher.cpp
expression_sources = $(build_expression_dir)/BuiltinFunctions.cpp \
                     $(build_expression_dir)/CommonExternalFunctions.cpp \
                     $(build_expression_dir)/ExpressionCompoundVariable.cpp \
                     $(build_expression_dir)/ExpressionDotVariable.cpp \
                     $(build_expression_dir)/ExpressionFunction.cpp \
                     $(build_expression_dir)/ExpressionMessage.cpp \
                     $(build_expression_dir)/ExpressionOperator.cpp \
                     $(build_expression_dir)/ExpressionStack.cpp \
                     $(build_expression_dir)/ExpressionStem.cpp \
                     $(build_expression_dir)/ExpressionVariable.cpp \
                     $(build_expression_dir)/IndirectVariableReference.cpp \
                     $(build_expression_dir)/ExpressionLogical.cpp
instructions_sources  = $(build_instructions_dir)/AddressInstruction.cpp \
                        $(build_instructions_dir)/AssignmentInstruction.cpp \
                        $(build_instructions_dir)/CallInstruction.cpp \
                        $(build_instructions_dir)/ClassDirective.cpp \
                        $(build_instructions_dir)/CommandInstruction.cpp \
                        $(build_instructions_dir)/DoBlock.cpp \
                        $(build_instructions_dir)/DoInstruction.cpp \
                        $(build_instructions_dir)/DropInstruction.cpp \
                        $(build_instructions_dir)/ElseInstruction.cpp \
                        $(build_instructions_dir)/EndIf.cpp \
                        $(build_instructions_dir)/EndInstruction.cpp \
                        $(build_instructions_dir)/ExitInstruction.cpp \
                        $(build_instructions_dir)/ExposeInstruction.cpp \
                        $(build_instructions_dir)/ForwardInstruction.cpp \
                        $(build_instructions_dir)/GuardInstruction.cpp \
                        $(build_instructions_dir)/IfInstruction.cpp \
                        $(build_instructions_dir)/InterpretInstruction.cpp \
                        $(build_instructions_dir)/LabelInstruction.cpp \
                        $(build_instructions_dir)/LeaveInstruction.cpp \
                        $(build_instructions_dir)/LibraryDirective.cpp \
                        $(build_instructions_dir)/MessageInstruction.cpp \
                        $(build_instructions_dir)/NopInstruction.cpp \
                        $(build_instructions_dir)/NumericInstruction.cpp \
                        $(build_instructions_dir)/OptionsInstruction.cpp \
                        $(build_instructions_dir)/OtherwiseInstruction.cpp \
                        $(build_instructions_dir)/ParseInstruction.cpp \
                        $(build_instructions_dir)/ParseTarget.cpp \
                        $(build_instructions_dir)/ParseTrigger.cpp \
                        $(build_instructions_dir)/ProcedureInstruction.cpp \
                        $(build_instructions_dir)/QueueInstruction.cpp \
                        $(build_instructions_dir)/RaiseInstruction.cpp \
                        $(build_instructions_dir)/ReplyInstruction.cpp \
                        $(build_instructions_dir)/RequiresDirective.cpp \
                        $(build_instructions_dir)/ReturnInstruction.cpp \
                        $(build_instructions_dir)/RexxInstruction.cpp \
                        $(build_instructions_dir)/SayInstruction.cpp \
                        $(build_instructions_dir)/SelectInstruction.cpp \
                        $(build_instructions_dir)/SignalInstruction.cpp \
                        $(build_instructions_dir)/ThenInstruction.cpp \
                        $(build_instructions_dir)/TraceInstruction.cpp \
                        $(build_instructions_dir)/UseStrictInstruction.cpp
parser_sources = $(build_parser_dir)/Clause.cpp \
                 $(build_parser_dir)/KeywordConstants.cpp \
                 $(build_parser_dir)/InstructionParser.cpp \
                 $(build_parser_dir)/Scanner.cpp \
                 $(build_parser_dir)/SourceFile.cpp \
                 $(build_parser_dir)/Token.cpp
platform_sources = $(build_interpreter_platform_dir)/ErrorMessages.cpp \
                   $(build_interpreter_platform_dir)/ExternalFunctions.cpp \
                   $(build_interpreter_platform_dir)/FileSystem.cpp \
                   $(build_interpreter_platform_dir)/MemorySupport.cpp \
                   $(build_interpreter_platform_dir)/MiscSystem.cpp \
                   $(build_interpreter_platform_dir)/RexxMain.cpp \
                   $(build_interpreter_platform_dir)/SysActivity.cpp \
                   $(build_interpreter_platform_dir)/SysFileSystem.cpp \
                   $(build_interpreter_platform_dir)/SysInterpreterInstance.cpp \
                   $(build_interpreter_platform_dir)/SystemCommands.cpp \
                   $(build_interpreter_platform_dir)/SystemInitialization.cpp \
                   $(build_interpreter_platform_dir)/SystemInterpreter.cpp \
                   $(build_interpreter_platform_dir)/TimeSupport.cpp \
                   $(build_interpreter_platform_dir)/UseridFunction.cpp \
                   $(build_interpreter_platform_dir)/ValueFunction.cpp
common_sources = $(build_common_dir)/Utilities.cpp \
                 $(build_common_platform_dir)/SysFile.cpp \
                 $(build_common_platform_dir)/SysLibrary.cpp \
                 $(build_common_platform_dir)/SysSemaphore.cpp \
                 $(build_common_platform_dir)/SysThread.cpp
runtime_sources  = $(build_runtime_dir)/GlobalData.cpp \
                   $(build_runtime_dir)/GlobalNames.cpp \
                   $(build_runtime_dir)/InternalPackage.cpp \
                   $(build_runtime_dir)/Interpreter.cpp \
                   $(build_runtime_dir)/InterpreterInstance.cpp \
                   $(build_runtime_dir)/Numerics.cpp \
                   $(build_runtime_dir)/RexxMisc.cpp \
                   $(build_runtime_dir)/Version.cpp
streamlibrary_sources = $(build_streamlibrary_dir)/StreamCommandParser.cpp \
                        $(build_streamlibrary_dir)/StreamNative.cpp \
                        $(build_streamlibrary_dir)/FileNative.cpp
librexx_la_SOURCES = $(classes_sources) \
                     $(classes_support_sources) \
                     $(interpreter_api_sources) \
                     $(behaviour_sources) \
                     $(execution_sources) \
                     $(memory_sources) \
                     $(package_sources) \
                     $(concurrency_sources) \
                     $(expression_sources) \
                     $(instructions_sources) \
                     $(parser_sources) \
                     $(platform_sources) \
                     $(common_sources) \
                     $(runtime_sources) \
                     $(streamlibrary_sources)
core_classes = $(build_rexxclasses_dir)/CoreClasses.orx \
               $(build_rexxclasses_dir)/StreamClasses.orx \
               $(build_rexxclasses_dir)/Serializable.orx
core_platform_classes = $(build_interpreter_platform_dir)/PlatformObjects.orx
#
# Preprocessor flags for librexx.so
#
librexx_la_CXXFLAGS  = $(COMMON_CPPFLAGS) \
                       -I$(build_lib_dir) \
                       -I$(build_api_dir) \
                       -I$(build_api_platform_dir) \
                       -I$(build_common_dir) \
                       -I$(build_common_platform_dir) \
                       -I$(build_interpreter_dir) \
                       -I$(build_behaviour_dir) \
                       -I$(build_execution_dir) \
                       -I$(build_memory_dir) \
                       -I$(build_package_dir) \
                       -I$(build_concurrency_dir) \
                       -I$(build_expression_dir) \
                       -I$(build_instructions_dir) \
                       -I$(build_classes_dir) \
                       -I$(build_classes_support_dir) \
                       -I$(build_runtime_dir) \
                       -I$(build_parser_dir) \
                       -I$(build_messages_dir) \
                       -I$(build_streamlibrary_dir) \
                       -I$(build_interpreter_common_dir) \
                       -I$(build_interpreter_platform_dir)

librexx_la_LDFLAGS = $(VERSION_INFO) @ORX_LDFLAGS_PACKAGE@

#################### rxapi ##########################
# Sources for rxapi
#
rxapi_SOURCES = $(build_rexxapi_server_dir)/APIServer.cpp \
                $(build_rexxapi_server_dir)/APIServerInstance.cpp \
                $(build_rexxapi_server_dir)/APIServerThread.cpp \
                $(build_rexxapi_server_dir)/MacroSpaceManager.cpp \
                $(build_rexxapi_server_dir)/QueueManager.cpp \
                $(build_rexxapi_server_dir)/RegistrationManager.cpp \
                $(build_rexxapi_server_dir)/platform/unix/APIService.cpp \
                $(build_rexxapi_common_dir)/ServiceMessage.cpp \
                $(build_rexxapi_common_platform_dir)/SysAPIManager.cpp \
                $(build_rexxapi_common_platform_dir)/SysCSStream.cpp \
                $(build_rexxapi_common_platform_dir)/SysProcess.cpp \
                $(build_common_dir)/Utilities.cpp \
                $(build_common_dir)/platform/unix/SysSemaphore.cpp \
                $(build_common_dir)/platform/unix/SysThread.cpp
#
# Preprocessor flags for rxapi
#
rxapi_CXXFLAGS  = $(COMMON_CPPFLAGS) \
                 -I$(build_lib_dir) \
                 -I$(build_api_dir) \
                 -I$(build_api_platform_dir) \
                 -I$(build_messages_dir) \
                 -I$(build_rexxapi_server_dir) \
                 -I$(build_rexxapi_server_platform_dir) \
                 -I$(build_rexxapi_common_dir) \
                 -I$(build_rexxapi_common_platform_dir) \
                 -I$(build_common_dir) \
                 -I$(build_common_platform_dir)
#
# Extra libraries for rxapi
#
rxapi_LDADD   = @ORX_LDADD_EXECUTABLE@
rxapi_LDFLAGS = @ORX_LDFLAGS_EXECUTABLE@

#################### rexx ##########################
# Sources for rexx
#
rexx_SOURCES = $(build_utilities_rexx_platform_dir)/rexx.cpp
#
# Preprocessor flags for rexx
#
rexx_CXXFLAGS  = $(COMMON_CPPFLAGS) \
                 -I$(build_lib_dir) \
                 -I$(build_api_dir) \
                 -I$(build_api_platform_dir) \
                 -I$(build_messages_dir)
#
# Extra libraries for rexx
#
rexx_LDADD   = librexx.la librexxapi.la @ORX_LDADD_EXECUTABLE@
rexx_LDFLAGS = @ORX_INIT_LDFLAGS@ @ORX_LDFLAGS_EXECUTABLE@

#################### rexximage ##########################
# Sources for rexx
#
rexximage_SOURCES = $(build_utilities_rexximage_dir)/rexximage.cpp
#
# Preprocessor flags for rexximage
#
rexximage_CXXFLAGS  = $(COMMON_CPPFLAGS) \
                 -I$(build_lib_dir) \
                 -I$(build_api_dir) \
                 -I$(build_api_platform_dir)
#
# Extra libraries for rexx
#
rexximage_LDADD   = librexx.la librexxapi.la @ORX_LDADD_EXECUTABLE@
rexximage_LDFLAGS = @ORX_INIT_LDFLAGS@ @ORX_LDFLAGS_EXECUTABLE@

#################### rexxc #########################
# Sources for rexxc
#
rexxc_SOURCES = $(build_utilities_rexxc_platform_dir)/RexxCompiler.cpp
#
# Preprocessor flags for rexxc
#
rexxc_CXXFLAGS  = $(COMMON_CPPFLAGS) \
                  -I$(build_lib_dir) \
                  -I$(build_api_dir) \
                  -I$(build_api_platform_dir) \
                  -I$(build_messages_dir)

#
# Extra libraries for rexxc
#
rexxc_LDADD   = librexx.la librexxapi.la @ORX_LDADD_EXECUTABLE@
rexxc_LDFLAGS = @ORX_INIT_LDFLAGS@ @ORX_LDFLAGS_EXECUTABLE@

#################### rxqueue #########################
# Sources for rxqueue
#
rxqueue_SOURCES =  $(build_utilities_rxqueue_platform_dir)/rxqueue.cpp
#
# Preprocessor flags for rxqueue
#
rxqueue_CXXFLAGS  = $(COMMON_CPPFLAGS) \
                    -I$(build_lib_dir) \
                    -I$(build_api_dir) \
                    -I$(build_api_platform_dir) \
                    -I$(build_interpreter_platform_dir) \
                    -I$(build_messages_dir)
#
# Extra libraries for rxqueue
#
rxqueue_LDADD   = librexx.la librexxapi.la @ORX_LDADD_EXECUTABLE@
rxqueue_LDFLAGS = @ORX_INIT_LDFLAGS@ @ORX_LDFLAGS_EXECUTABLE@

#################### rxsubcom #########################
# Sources for rxsubcom
#
rxsubcom_SOURCES =  $(build_utilities_rxsubcom_platform_dir)/rxsubcom.cpp
#
# Preprocessor flags for rxsubcom
#
rxsubcom_CXXFLAGS  = $(COMMON_CPPFLAGS) \
                    -I$(build_lib_dir) \
                    -I$(build_api_dir) \
                    -I$(build_api_platform_dir) \
                    -I$(build_interpreter_platform_dir) \
                    -I$(build_messages_dir)
#
# Extra libraries for rxsubcom
#
rxsubcom_LDADD   = librexx.la librexxapi.la @ORX_LDADD_EXECUTABLE@
rxsubcom_LDFLAGS = @ORX_INIT_LDFLAGS@ @ORX_LDFLAGS_EXECUTABLE@

#################### librexxutil.so ################
#
# Sources for librexxutil.so
#
librexxutil_la_SOURCES = $(build_extensions_rexutils_platform_dir)/rexxutil.cpp
#
# Preprocessor flags for librexxutil.so
#
librexxutil_la_CXXFLAGS = $(COMMON_CPPFLAGS) \
                          -I$(build_lib_dir) \
                          -I$(build_api_dir) \
                          -I$(build_api_platform_dir) \
                          -I$(build_messages_dir)
#
# Extra libraries for librexxutil.so (maybe)
#
librexxutil_la_LIBADD  = @ORX_LIBADD_LIBRARY@
librexxutil_la_LDFLAGS = $(VERSION_INFO) @ORX_LDFLAGS_LIBRARY@

#################### librxmath.so ################
#
# Sources for librxmath.so
#
librxmath_la_SOURCES = $(build_extensions_rxmath_dir)/rxmath.cpp
#
# Preprocessor flags for librxmath.so
#
librxmath_la_CXXFLAGS = $(COMMON_CPPFLAGS) \
                        -I$(build_lib_dir) \
                        -I$(build_api_dir) \
                        -I$(build_api_platform_dir)
#
# Extra libraries for librxmath.so (maybe)
#
librxmath_la_LIBADD  = @ORX_LIBADD_LIBRARY@
librxmath_la_LDFLAGS = $(VERSION_INFO) @ORX_LDFLAGS_LIBRARY@

#################### librxsock.so ################
#
# Sources for librxsock.so
#
librxsock_la_SOURCES = $(build_extensions_rxsock_dir)/rxsock.cpp \
                       $(build_extensions_rxsock_dir)/rxsockfn.cpp
#
# Preprocessor flags for librxsock.so
#
librxsock_la_CXXFLAGS = $(COMMON_CPPFLAGS) \
                      -I$(build_api_dir) \
                      -I$(build_api_platform_dir) \
                      -I$(build_extensions_dir)
#
# Extra libraries for librxsock.so (maybe)
#
librxsock_la_LIBADD  = @ORX_LIBADD_LIBRARY@
librxsock_la_LDFLAGS = $(VERSION_INFO) @ORX_LDFLAGS_LIBRARY@

#################### librxregexp.so ################
#
# Sources for librxregexp.so
#
librxregexp_la_SOURCES = $(build_extensions_rxregexp_dir)/automaton.cpp \
                         $(build_extensions_rxregexp_dir)/dblqueue.cpp \
                         $(build_extensions_rxregexp_dir)/rxregexp.cpp
#
# Preprocessor flags for librxregexp.so
#
librxregexp_la_CXXFLAGS = $(COMMON_CPPFLAGS) \
                          -I$(build_lib_dir) \
                          -I$(build_api_dir) \
                          -I$(build_api_platform_dir) \
                          -I$(build_messages_dir)
#
# Extra libraries for librxregexp.so (maybe)
#
librxregexp_la_LIBADD  = @ORX_LIBADD_LIBRARY@
librxregexp_la_LDFLAGS = $(VERSION_INFO) @ORX_LDFLAGS_LIBRARY@

#################### libhostemu.so ################
#
# Sources for libhostemu.so
#
libhostemu_la_SOURCES = $(extensions_hostemu_unix_dir)/hostemu.cpp \
                        $(build_extensions_hostemu_dir)/hostemu.h \
                        $(build_extensions_hostemu_dir)/cmdparse.ypp
libhostemu_la_BUILTSOURCES = $(build_extensions_hostemu_dir)/cmdparse.cpp
#
# Preprocessor flags for libhostemu.so
#
libhostemu_la_CXXFLAGS = $(COMMON_CPPFLAGS) \
                        -I$(build_api_dir) \
                        -I$(build_api_platform_dir) \
                        -I$(build_extensions_hostemu_dir)
#
# Extra libraries for libhostemu.so (maybe)
#
libhostemu_la_LIBADD  = @ORX_LIBADD_LIBRARY@
libhostemu_la_LDFLAGS = $(VERSION_INFO) @ORX_LDFLAGS_LIBRARY@

#
# explicit targets:
#

rexx.cat: $(build_messages_dir)/gencat.inp
	-gencat $(@) $(build_messages_dir)/gencat.inp

$(build_messages_dir)/RexxMessageNumbers.h: $(build_messages_dir)/RexxMessageNumbers.xsl $(messageXML)
	sh ./xsl.sh $(@) $(messageXML) $(build_messages_dir)/RexxMessageNumbers.xsl

$(build_messages_dir)/RexxErrorCodes.h: $(build_messages_dir)/RexxErrorCodes.xsl $(messageXML)
	sh ./xsl.sh $(@) $(messageXML) $(build_messages_dir)/RexxErrorCodes.xsl

$(build_messages_dir)/DocErrorMessages.sgml: $(build_messages_dir)/DocBookErrors.xsl $(messageXML)
	sh ./xsl.sh $(@) $(messageXML) $(build_messages_dir)/DocBookErrors.xsl

$(build_api_dir)/oorexxerrors.h: $(build_messages_dir)/ApiErrorCodes.xsl $(messageXML)
	sh ./xsl.sh $(@) $(messageXML) $(build_messages_dir)/ApiErrorCodes.xsl

$(build_messages_dir)/RexxMessageTable.h: $(build_messages_dir)/RexxMessageTable.xsl $(messageXML)
	sh ./xsl.sh $(@) $(messageXML) $(build_messages_dir)/RexxMessageTable.xsl

$(build_messages_dir)/gencat.inp: $(build_interpreter_platform_dir)/UnixGencat.xsl $(messageXML)
	sh ./xsl.sh $(@) $(messageXML) $(build_interpreter_platform_dir)/UnixGencat.xsl

$(build_behaviour_dir)/ClassTypeCodes.h: $(build_behaviour_dir)/ClassTypeCodes.xsl $(classesXML)
	sh ./xsl.sh $(@) $(classesXML) $(build_behaviour_dir)/ClassTypeCodes.xsl

$(build_behaviour_dir)/PrimitiveBehaviourNames.h: $(build_behaviour_dir)/PrimitiveBehaviourNames.xsl $(classesXML)
	sh ./xsl.sh $(@) $(classesXML) $(build_behaviour_dir)/PrimitiveBehaviourNames.xsl

$(build_behaviour_dir)/PrimitiveBehaviours.cpp: $(build_behaviour_dir)/PrimitiveBehaviours.xsl $(classesXML)
	sh ./xsl.sh $(@) $(classesXML) $(build_behaviour_dir)/PrimitiveBehaviours.xsl

$(build_behaviour_dir)/VirtualFunctionTable.cpp: $(build_behaviour_dir)/VirtualFunctionTable.xsl $(classesXML)
	sh ./xsl.sh $(@) $(classesXML) $(build_behaviour_dir)/VirtualFunctionTable.xsl

rexx.img: rexximage librexxutil.la $(core_classes) $(core_platform_classes)
	cp $(build_rexxclasses_dir)/*.orx .
	cp $(build_interpreter_platform_dir)/*.orx .
	./rexximage
	rm -f *.orx

#
# This ensures a developer can run the test suite from the build directory
#
ooTest:
	cp $(build_extensions_dir)/rxregexp/rxregexp.cls ./
	cp $(build_extensions_dir)/rxftp/rxftp.cls ./
	cp $(build_extensions_dir)/csvStream/csvStream.cls ./
	cp $(build_extensions_dir)/rxsock/socket.cls ./
	cp $(build_extensions_dir)/rxsock/streamsocket.cls ./

#
# distribution files
#
EXTRA_DIST = $(commonMiscFiles) \
             $(@ORX_PLATFORM@MiscFiles)

dist-hook:
	rm -fr `find $(distdir) -name .svn`
	rm -fr `find $(distdir) -name "*.bak"`
	rm -fr `find $(distdir) -name "*.o"`
	rm -fr `find $(distdir) -name "*.lo"`

#
# Force the timestamp of the message headers and the input to gencat
# to have a later date than the source XML, so Xalan is not needed to
# be executed by the user
	touch $(distdir)/interpreter/messages/*.h
	touch $(distdir)/api/oorexxerrors.h
	touch $(distdir)/interpreter/messages/DocErrorMessages.sgml
	touch $(distdir)/interpreter/messages/gencat.inp

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
	rm -fr $(top_srcdir)/debian
	rm -f $(top_srcdir)/*.cls

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
	$(install_sh_SCRIPT) $(build_os_platform_dir)/oorexx-config $(DESTDIR)$(prefix)/bin/oorexx-config
	$(install_sh_SCRIPT) $(build_extensions_rxregexp_dir)/rxregexp.cls $(DESTDIR)$(prefix)/bin/rxregexp.cls
	$(install_sh_SCRIPT) $(build_extensions_rxftp_dir)/rxftp.cls $(DESTDIR)$(prefix)/bin/rxftp.cls
	$(install_sh_SCRIPT) $(build_extensions_csvstream_dir)/csvStream.cls $(DESTDIR)$(prefix)/bin/csvStream.cls
	$(install_sh_SCRIPT) $(build_extensions_rxsock_dir)/socket.cls $(DESTDIR)$(prefix)/bin/socket.cls
	$(install_sh_SCRIPT) $(build_extensions_rxsock_dir)/streamsocket.cls $(DESTDIR)$(prefix)/bin/streamsocket.cls
	$(install_sh_SCRIPT) $(build_extensions_rxsock_dir)/mime.cls $(DESTDIR)$(prefix)/bin/mime.cls
	$(install_sh_SCRIPT) $(build_extensions_rxsock_dir)/smtp.cls $(DESTDIR)$(prefix)/bin/smtp.cls
	if test "$(ORX_SYS_STR)" != "AIX"; then \
		$(install_sh_SCRIPT) $(rexxapi_server_unix_dir)/rxapid $(DESTDIR)$(prefix)/bin/rxapid; \
	fi
	for a in $(build_samples_dir)/*.rex; do \
		bn=`basename $$a`; \
		$(install_sh_SCRIPT) $$a $(DESTDIR)$(prefix)/share/ooRexx/$$bn; \
	done;
	$(install_sh_SCRIPT) $(build_samples_dir)/readme $(DESTDIR)$(prefix)/share/ooRexx/readme

package:
	./rexx $(build_os_platform_dir)/makepkg.rex @PACKAGETYPE@ $(ORX_MAJOR).$(ORX_MINOR).$(ORX_MOD_LVL) $(target) $(prefix) $(DESTDIR)

rpm: dist
	rm -fr $(top_srcdir)/rpm
	mkdir rpm
	mkdir rpm/BUILD
	mkdir rpm/RPMS
	mkdir rpm/SOURCES
	mkdir rpm/SPECS
	mkdir rpm/SRPMS
	rpmbuild -ba $(build_os_platform_dir)/oorexx.spec < /dev/null;

deb:
	rm -fr $(top_srcdir)/debian
	mkdir $(top_srcdir)/debian
# Generate a current change log in the required Debian format
	sed -n -e '1,2p' $(build_os_platform_dir)/debian/changelog > ${top_srcdir}/debian/changelog
	sed -e 's/.*/  &/' CHANGES >> ${top_srcdir}/debian/changelog
	sed -e "1,3d; 4,6s/date/`date -R`/" $(build_os_platform_dir)/debian/changelog >>  ${top_srcdir}/debian/changelog
# Just copy the rest of the files
	cp $(build_os_platform_dir)/debian/compat    ${top_srcdir}/debian
	cp $(build_os_platform_dir)/debian/control   ${top_srcdir}/debian
	cp $(build_os_platform_dir)/debian/copyright ${top_srcdir}/debian
	cp $(build_os_platform_dir)/debian/dirs      ${top_srcdir}/debian
	cp $(build_os_platform_dir)/debian/docs      ${top_srcdir}/debian
	cp $(build_os_platform_dir)/debian/postinst  ${top_srcdir}/debian
	cp $(build_os_platform_dir)/debian/postrm    ${top_srcdir}/debian
	cp $(build_os_platform_dir)/debian/prerm     ${top_srcdir}/debian
	cp $(build_os_platform_dir)/debian/rules     ${top_srcdir}/debian
# Make the binaries
	make all
# Install into the debian package directory
	fakeroot dh_clean -k
	fakeroot dh_installdirs
	fakeroot make install DESTDIR=`pwd`/debian/oorexx
# Do the packaging steps
	fakeroot dh_installchangelogs
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

