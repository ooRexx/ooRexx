#/*----------------------------------------------------------------------------*/
#/*                                                                            */
#/* Copyright (c) 2014-2014 Rexx Language Association. All rights reserved.    */
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


#/*----------------------------------------------------------------------------*/
#/* Global settings                                                            */
#/*----------------------------------------------------------------------------*/
cmake_minimum_required (VERSION 2.6)
cmake_policy(SET CMP0010 OLD)
include(CheckIncludeFile)
include(CheckLibraryExists)
include(CheckFunctionExists)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin)
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin)
set (CMAKE_PDB_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin)


#/*----------------------------------------------------------------------------*/
#/* Project settings                                                           */
#/*----------------------------------------------------------------------------*/
project (ooRexx)
# The version of ooRexx to make
set (ORX_MAJOR 4)
set (ORX_MINOR 3)
set (ORX_MOD_LVL 0)
set (ORX_BLD_LVL 0)
set (ORX_VERSION ${ORX_MAJOR}.${ORX_MINOR}.${ORX_MOD_LVL})
set (ORX_VER_STR ${ORX_MAJOR}.${ORX_MINOR}.${ORX_MOD_LVL})
string(TIMESTAMP ORX_YEAR %Y)
set (ORX_COPY_YEAR 2005-${ORX_YEAR})

# The following supports the versioning of the rexx shared libraries
# This defines the oorexx library version. For ooRexx this should always be the
# same as the ORX_MAJOR number.
set (ORX_CURRENT ${ORX_MAJOR})
# Each public release of ooRexx should increment this number by one except
# when the ORX_MINOR goes back to zero. If that is the case then this entry
# should also be set to zero.
set (ORX_REVISION 7)
# For ooRexx, this number should ALWAYS be zero. This will force the linker to
# utilize all previous releases for linking.
set (ORX_AGE 0)
# Please note that the version-info has nothing to do with the release version.
# You need to know exactly what you are doing in order to get this correct.
set(VERSION_INFO "-version-info ${ORX_CURRENT}:${ORX_REVISION}:${ORX_AGE}")

# The following are general settings
# Always create shared libraries (DLLs)
set (CMAKE_CXX_CREATE_SHARED_MODULE 1)
# Set the platform subdirectory name
if (WIN32)
   set (ORX_PLATFORM_DIR platform/windows)
else ()
   set (ORX_PLATFORM_DIR platform/unix)
endif ()
# Set path to look for include files
include_directories(BEFORE api api/${ORX_PLATFORM_DIR})

# Set install variables
if (WIN32)
   set (catdir ${CMAKE_INSTALL_PREFIX}/ooRexx)
else ()
   set (CMAKE_INSTALL_PREFIX /usr)
   set (catdir ${CMAKE_INSTALL_PREFIX}/bin)
endif ()

# Set compiler and linker flags common to all build environments
if (WIN32)
   add_definitions(/DORX_VER=${ORX_MAJOR} -DORX_REL=${ORX_MINOR} /DORX_MOD=${ORX_MOD_LVL}
                   /DOOREXX_COPY_YEAR="${ORX_COPY_YEAR}"
                   /DORX_SYS_STR="${CMAKE_SYSTEM_NAME}"
                   /DORX_CATDIR="${catdir}"
                   /DORX_SHARED_LIBRARY_EXT="${CMAKE_SHARED_LIBRARY_SUFFIX}"
                   /DWIN32 /D_WINDOWS /W3 /GR /EHsc /MDd /Zi /Ob0 /Od /RTC)
else ()
   add_definitions(-DORX_VER=${ORX_MAJOR} -DORX_REL=${ORX_MINOR} -DORX_MOD=${ORX_MOD_LVL} -DORX_FIX=0
                   -DOOREXX_COPY_YEAR="${ORX_COPY_YEAR}"
                   -DORX_SYS_STR="${CMAKE_SYSTEM_NAME}"
                   -DORX_CATDIR="${catdir}"
                   -DORX_SHARED_LIBRARY_EXT="${CMAKE_SHARED_LIBRARY_SUFFIX}")
endif ()

# Check for libraries and functions
if (WIN32)
   # TODO: add Windows checks here
else ()
   find_library(ORX_SYSLIB_DL dl)
   find_library(ORX_SYSLIB_PTHREAD pthread)
   if (${ORX_SYSLIB_PTHREAD})
      check_function_exists(pthread_mutexattr_settype HAVE_PTHREAD_MUTEXATTR_SETTYPE)
   endif ()
   find_library(ORX_SYSLIB_RT rt)
   if (${ORX_SYSLIB_RT})
      check_function_exists(nanosleep HAVE_NANOSLEEP)
   endif ()
endif ()

# Check header files
# Std C headers
check_include_file(stdlib.h HAVE_STDLIB_H)
check_include_file(stdarg.h HAVE_STDARG_H)
check_include_file(stddef.h HAVE_STDDEF_H)
check_include_file(stdio.h HAVE_STDIO_H)
check_include_file(string.h HAVE_STRING_H)
check_include_file(float.h HAVE_FLOAT_H)
check_include_file(ctype.h HAVE_CTYPE_H)
check_include_file(locale.h HAVE_LOCAL_H)
check_include_file(signal.h HAVE_SIGNAL_H)
check_include_file(time.h HAVE_TIME_H)
check_include_file(limits.h HAVE_LIMITS_H)
# Other header files
check_include_file(fcntl.h HAVE_FCNTL_H)
check_include_file(features.h HAVE_FEATURES_H)
check_include_file(filehdr.h HAVE_FILEHDR_H)
check_include_file(mesg.h HAVE_MESG_H)
check_include_file(malloc.h HAVE_MALLOC_H)
check_include_file(netinet/in.h HAVE_IN_H)
check_include_file(nl_types.h HAVE_NL_TYPES_H)
check_include_file(pthread.h HAVE_PTHREAD_H)
check_include_file(pwd.h HAVE_PWD_H)
check_include_file(sched.h HAVE_SCHED_H)
check_include_file(stropts.h HAVE_STROPTS_H)
check_include_file(sys/filio.h HAVE_FILIO_H)
check_include_file(sys/ldr.h HAVE_SYS_LDR_H)
check_include_file(sys/resource.h HAVE_RESOURCE_H)
check_include_file(sys/select.h HAVE_SELECT_H)
check_include_file(sys/sem.h HAVE_SEM_H)
check_include_file(sys/signal.h HAVE_SYS_SIGNAL_H)
check_include_file(sys/socket.h HAVE_SYS_SOCKET_H)
check_include_file(sys/time.h HAVE_SYS_TIME_H)
check_include_file(sys/utsname.h HAVE_SYS_UTSNAME_H)
check_include_file(sys/wait.h HAVE_SYS_WAIT_H)
check_include_file(sys/syscall.h HAVE_SYS_SYSCALL_H)
check_include_file(unistd.h HAVE_UNISTD_H)
check_include_file(usersec.h HAVE_USERSEC_H)
check_include_file(attr/xattr.h HAVE_ATTR_XATTR_H)

# Configure files
# TODO: revisit this section later
#if (NOT WIN32)
#   configure_file(platform/unix/oorexx.spec.in platform/unix/oorexx.spec)
#   configure_file(platform/unix/debian/changelog.in platform/unix/debian/changelog)
#   configure_file(platform/unix/debian/postinst.in platform/unix/debian/postinst)
#   configure_file(rexxapi/server/platform/unix/rxapid.in rexxapi/server/platform/unix/rxapid)
#   configure_file(platform/unix/oorexx-config.in platform/unix/oorexx-config)
#endif ()



#/*----------------------------------------------------------------------------*/
#/* Subdirectory short names                                                   */
#/*----------------------------------------------------------------------------*/
set (build_common_dir common)
set (build_common_platform_dir ${build_common_dir}/${ORX_PLATFORM_DIR})
set (build_lib_dir lib)
set (build_platform_dir ${ORX_PLATFORM_DIR})
set (build_rexxapi_dir rexxapi)
set (build_rexxapi_client_dir ${build_rexxapi_dir}/client)
set (build_rexxapi_client_platform_dir ${build_rexxapi_client_dir}/${ORX_PLATFORM_DIR})
set (build_rexxapi_common_dir ${build_rexxapi_dir}/common)
set (build_rexxapi_common_platform_dir ${build_rexxapi_common_dir}/${ORX_PLATFORM_DIR})
set (build_rexxapi_server_dir ${build_rexxapi_dir}/server)
set (build_rexxapi_server_platform_dir ${build_rexxapi_server_dir}/${ORX_PLATFORM_DIR})
set (build_interpreter_dir interpreter)
set (build_interpreter_platform_dir ${build_interpreter_dir}/${ORX_PLATFORM_DIR})
set (build_interpreter_api_dir ${build_interpreter_dir}/api)
set (build_behaviour_dir ${build_interpreter_dir}/behaviour)
set (build_classes_dir ${build_interpreter_dir}/classes)
set (build_classes_support_dir ${build_interpreter_dir}/classes/support)
set (build_concurrency_dir ${build_interpreter_dir}/concurrency)
set (build_execution_dir ${build_interpreter_dir}/execution)
set (build_expression_dir ${build_interpreter_dir}/expression)
set (build_instructions_dir ${build_interpreter_dir}/instructions)
set (build_memory_dir ${build_interpreter_dir}/memory)
set (build_messages_dir ${build_interpreter_dir}/messages)
set (build_package_dir ${build_interpreter_dir}/package)
set (build_parser_di= ${build_interpreter_dir}/parser)
set (build_interpreter_common_dir ${build_interpreter_dir}/platform/common)
set (build_rexxclasses_dir ${build_interpreter_dir}/RexxClasses)
set (build_runtime_dir ${build_interpreter_dir}/runtime)
set (build_streamlibrary_dir ${build_interpreter_dir}/streamLibrary)
set (build_extensions_dir extensions)
set (build_extensions_platform_dir ${build_extensions_dir}/${ORX_PLATFORM_DIR})
set (build_extensions_rexutils_dir ${build_extensions_dir}/rexxutil)
set (build_extensions_rexutils_platform_dir ${build_extensions_rexutils_dir}/${ORX_PLATFORM_DIR})
set (build_extensions_rxftp_dir ${build_extensions_dir}/rxftp)
set (build_extensions_csvstream_dir ${build_extensions_dir}/csvStream)
set (build_extensions_rxmath_dir ${build_extensions_dir}/rxmath)
set (build_extensions_rxregexp_dir ${build_extensions_dir}/rxregexp)
set (build_extensions_rxsock_dir ${build_extensions_dir}/rxsock)
set (build_extensions_hostemu_dir ${build_extensions_dir}/hostemu)
set (build_utilities_dir utilities)
set (build_utilities_rexx_dir ${build_utilities_dir}/rexx/${ORX_PLATFORM_DIR})
set (build_utilities_rexxc_dir ${build_utilities_dir}/rexxc/${ORX_PLATFORM_DIR})
set (build_utilities_rexximage_dir ${build_utilities_dir}/rexximage)
set (build_utilities_rxqueue_dir ${build_utilities_dir}/rxqueue/${ORX_PLATFORM_DIR})
set (build_utilities_rxsubcom_dir ${build_utilities_dir}/rxsubcom/${ORX_PLATFORM_DIR})
set (build_api_dir api)
set (build_samples_dir samples)

set (api_dir ${build_api_dir}/${ORX_PLATFORM_DIR})
if (WIN32)
   set (samples_dir ${build_samples_dir}/windows)
else ()
   set (samples_dir ${build_samples_dir}/unix)
endif ()

# The following are unix only
set (build_extensions_hostemu_dir ${build_extensions_hostemu_dir}/platform/unix)
# The following are windows only
set (build_extensions_ole_dir ${build_extensions_dir}/platform/windows/ole)
set (build_extensions_oodialog_dir ${build_extensions_dir}/platform/windows/oodialog)
set (build_extensions_orxscrpt_dir ${build_extensions_dir}/platform/windows/orxscrpt)
set (build_extensions_rxwinsys_dir ${build_extensions_dir}/platform/windows/rxwinsys)
set (build_utilities_rexxpaws_dir ${build_utilities_dir}/platform/windows/rexxpaws)
set (build_utilities_rexxhide_dir ${build_utilities_dir}/platform/windows/rexxhide)


#/*----------------------------------------------------------------------------*/
#/* Build targets                                                              */
#/*----------------------------------------------------------------------------*/

#################### explicit targets ##########################
find_program(ORX_HAVE_XALAN xalan)
if (NOT $ORX_HAVE_XALAN})
   find_program(ORX_HAVE_XALAN Xalan)
endif ()
if ($ORX_HAVE_XALAN})
   if (${ORX_HAVE_XALAN} EQUAL "Xalan")
      add_custom_command(OUTPUT ${build_messages_dir}/errnums.xml
                         COMMAND ${ORX_HAVE_XALAN} -o ${build_messages_dir}/errnums.xml ${build_messages_dir}/rexxmsg.xml ${build_messages_dir}/errnums.xsl
                         DEPENDS ${build_messages_dir}/rexxmsg.xml
                         WORKING_DIRECTORY ${build_messages_dir})
      add_custom_command(OUTPUT ${build_messages_dir}/errnumsrexxc.xml
                         COMMAND ${ORX_HAVE_XALAN} -o ${build_messages_dir}/errnumsrexxc.xml ${build_messages_dir}/rexxmsg.xml ${build_messages_dir}/errnumsrexxc.xsl
                         DEPENDS ${build_messages_dir}/rexxmsg.xml
                         WORKING_DIRECTORY ${build_messages_dir})
      add_custom_command(OUTPUT ${build_messages_dir}/errnumsrxqueue.xml
                         COMMAND ${ORX_HAVE_XALAN} -o ${build_messages_dir}/errnumsrxqueue.xml ${build_messages_dir}/rexxmsg.xml ${build_messages_dir}/errnumsrxqueue.xsl
                         DEPENDS ${build_messages_dir}/rexxmsg.xml
                         WORKING_DIRECTORY ${build_messages_dir})
      add_custom_command(OUTPUT ${build_messages_dir}/errnumssubcom.xml
                         COMMAND ${ORX_HAVE_XALAN} -o ${build_messages_dir}/errnumssubcom.xml ${build_messages_dir}/rexxmsg.xml ${build_messages_dir}/errnumssubcom.xsl
                         DEPENDS ${build_messages_dir}/rexxmsg.xml
                         WORKING_DIRECTORY ${build_messages_dir})
      add_custom_command(OUTPUT ${build_messages_dir}/gencat.inp
                         COMMAND ${ORX_HAVE_XALAN} -o ${build_messages_dir}/gencat.inp ${build_messages_dir}/rexxmsg.xml ${build_interpreter_platform_dir}/UnixGencat.xsl
                         DEPENDS ${build_messages_dir}/rexxmsg.xml
                         WORKING_DIRECTORY ${build_messages_dir})
      add_custom_command(OUTPUT ${build_behaviour_dir}/ClassTypeCodes.h
                         COMMAND ${ORX_HAVE_XALAN} -o ${build_behaviour_dir}/ClassTypeCodes.h ${build_behaviour_dir}/PrimitiveClasses.xml ${build_behaviour_dir}/ClassTypeCodes.xsl
                         DEPENDS ${build_behaviour_dir}/PrimitiveClasses.xml
                         WORKING_DIRECTORY ${build_behaviour_dir})
      add_custom_command(OUTPUT ${build_behaviour_dir}/PrimitiveBehaviourNames.h
                         COMMAND ${ORX_HAVE_XALAN} -o ${build_behaviour_dir}/PrimitiveBehaviourNames.h ${build_behaviour_dir}/PrimitiveClasses.xml ${build_behaviour_dir}/PrimitiveBehaviourNames.xsl
                         DEPENDS ${build_behaviour_dir}/PrimitiveClasses.xml
                         WORKING_DIRECTORY ${build_behaviour_dir})
      add_custom_command(OUTPUT ${build_behaviour_dir}/PrimitiveBehaviours.cpp
                         COMMAND ${ORX_HAVE_XALAN} -o ${build_behaviour_dir}/PrimitiveBehaviours.cpp ${build_behaviour_dir}/PrimitiveClasses.xml ${build_behaviour_dir}/PrimitiveBehaviours.xsl
                         DEPENDS ${build_behaviour_dir}/PrimitiveClasses.xml
                         WORKING_DIRECTORY ${build_behaviour_dir})
      add_custom_command(OUTPUT ${build_behaviour_dir}/VirtualFunctionTable.cpp
                         COMMAND ${ORX_HAVE_XALAN} -o ${build_behaviour_dir}/VirtualFunctionTable.cpp ${build_behaviour_dir}/PrimitiveClasses.xml ${build_behaviour_dir}/VirtualFunctionTable.xsl
                         DEPENDS ${build_behaviour_dir}/PrimitiveClasses.xml
                         WORKING_DIRECTORY ${build_behaviour_dir})
   else ()
      add_custom_command(OUTPUT ${build_messages_dir}/errnums.xml
                         COMMAND ${ORX_HAVE_XALAN} -out ${build_messages_dir}/errnums.xml -in ${build_messages_dir}/rexxmsg.xml -xsl ${build_messages_dir}/errnums.xsl
                         DEPENDS ${build_messages_dir}/rexxmsg.xml
                         WORKING_DIRECTORY ${build_messages_dir})
      add_custom_command(OUTPUT ${build_messages_dir}/errnumsrexxc.xml
                         COMMAND ${ORX_HAVE_XALAN} -out ${build_messages_dir}/errnumsrexxc.xml -in ${build_messages_dir}/rexxmsg.xml -xsl ${build_messages_dir}/errnumsrexxc.xsl
                         DEPENDS ${build_messages_dir}/rexxmsg.xml
                         WORKING_DIRECTORY ${build_messages_dir})
      add_custom_command(OUTPUT ${build_messages_dir}/errnumsrxqueue.xml
                         COMMAND ${ORX_HAVE_XALAN} -out ${build_messages_dir}/errnumsrxqueue.xml -in ${build_messages_dir}/rexxmsg.xml -xsl ${build_messages_dir}/errnumsrxqueue.xsl
                         DEPENDS ${build_messages_dir}/rexxmsg.xml
                         WORKING_DIRECTORY ${build_messages_dir})
      add_custom_command(OUTPUT ${build_messages_dir}/errnumssubcom.xml
                         COMMAND ${ORX_HAVE_XALAN} -out ${build_messages_dir}/errnumssubcom.xml -in ${build_messages_dir}/rexxmsg.xml -xsl ${build_messages_dir}/errnumssubcom.xsl
                         DEPENDS ${build_messages_dir}/rexxmsg.xml
                         WORKING_DIRECTORY ${build_messages_dir})
      add_custom_command(OUTPUT ${build_messages_dir}/gencat.inp
                         COMMAND ${ORX_HAVE_XALAN} -out ${build_messages_dir}/gencat.inp -in ${build_messages_dir}/rexxmsg.xml -xsl ${build_interpreter_platform_dir}/UnixGencat.xsl
                         DEPENDS ${build_messages_dir}/rexxmsg.xml
                         WORKING_DIRECTORY ${build_messages_dir})
      add_custom_command(OUTPUT ${build_behaviour_dir}/ClassTypeCodes.h
                         COMMAND ${ORX_HAVE_XALAN} -out ${build_behaviour_dir}/ClassTypeCodes.h -in ${build_messages_dir}/rexxmsg.xml -xsl ${build_behaviour_dir}/ClassTypeCodes.xsl
                         DEPENDS ${build_behaviour_dir}/PrimitiveClasses.xml
                         WORKING_DIRECTORY ${build_behaviour_dir})
      add_custom_command(OUTPUT ${build_behaviour_dir}/PrimitiveBehaviourNames.h
                         COMMAND ${ORX_HAVE_XALAN} -out ${build_behaviour_dir}/PrimitiveBehaviourNames.h -in ${build_messages_dir}/rexxmsg.xml -xsl ${build_behaviour_dir}/PrimitiveBehaviourNames.xsl
                         DEPENDS ${build_behaviour_dir}/PrimitiveClasses.xml
                         WORKING_DIRECTORY ${build_behaviour_dir})
      add_custom_command(OUTPUT ${build_behaviour_dir}/PrimitiveBehaviours.cpp
                         COMMAND ${ORX_HAVE_XALAN} -out ${build_behaviour_dir}/PrimitiveBehaviours.cpp -in ${build_behaviour_dir}/PrimitiveClasses.xml -xsl ${build_behaviour_dir}/PrimitiveBehaviours.xsl
                         DEPENDS ${build_behaviour_dir}/PrimitiveClasses.xml
                         WORKING_DIRECTORY ${build_behaviour_dir})
      add_custom_command(OUTPUT ${build_behaviour_dir}/VirtualFunctionTable.cpp
                         COMMAND ${ORX_HAVE_XALAN} -out ${build_behaviour_dir}/VirtualFunctionTable.cpp -in ${build_messages_dir}/rexxmsg.xml -xsl ${build_behaviour_dir}/VirtualFunctionTable.xsl
                         DEPENDS ${build_behaviour_dir}/PrimitiveClasses.xml
                         WORKING_DIRECTORY ${build_behaviour_dir})
   endif ()
endif ()

#################### librexxapi.so ##########################
add_library(rexxapi SHARED
            ${build_rexxapi_client_dir}/ClientMessage.cpp
            ${build_rexxapi_client_dir}/LocalAPIContext.cpp
            ${build_rexxapi_client_dir}/LocalAPIManager.cpp
            ${build_rexxapi_client_dir}/LocalMacroSpaceManager.cpp
            ${build_rexxapi_client_dir}/LocalQueueManager.cpp
            ${build_rexxapi_client_dir}/LocalRegistrationManager.cpp
            ${build_rexxapi_client_dir}/MacroSpaceApi.cpp
            ${build_rexxapi_client_dir}/QueuesAPI.cpp
            ${build_rexxapi_client_dir}/RegistrationAPI.cpp
            ${build_rexxapi_client_platform_dir}/SysLegacyAPI.cpp
            ${build_rexxapi_client_platform_dir}/SysLocalAPIManager.cpp
            ${build_rexxapi_common_dir}/RegistrationTable.cpp
            ${build_rexxapi_common_dir}/ServiceMessage.cpp
            ${build_rexxapi_common_platform_dir}/SysAPIManager.cpp
            ${build_rexxapi_common_platform_dir}/SysCSStream.cpp
            ${build_rexxapi_common_platform_dir}/SysProcess.cpp
            ${build_common_dir}/Utilities.cpp
            ${build_common_platform_dir}/SysSemaphore.cpp
            ${build_common_platform_dir}/SysLibrary.cpp
            ${build_common_platform_dir}/SysFile.cpp
            ${build_common_platform_dir}/SysThread.cpp)
target_include_directories(rexxapi PUBLIC
            ${build_rexxapi_client_dir}
            ${build_rexxapi_client_platform_dir}
            ${build_rexxapi_common_dir}
            ${build_rexxapi_common_platform_dir}
            ${build_common_dir}
            ${build_common_platform_dir}
            ${build_lib_dir})
if (WIN32)
   # TODO: add any Win libs as necessary
else ()
   target_link_libraries(rexxapi ${ORX_SYSLIB_DL} ${ORX_SYSLIB_PTHREAD})
endif ()



















