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
cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0010 OLD)
if(POLICY CMP0042)
  cmake_policy(SET CMP0042 NEW)
endif()
include(CheckIncludeFile)
include(CheckLibraryExists)
include(CheckFunctionExists)
include(CheckSymbolExists)
include(CheckCSourceCompiles)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set (CMAKE_PDB_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)


#/*----------------------------------------------------------------------------*/
#/* Project settings                                                           */
#/*----------------------------------------------------------------------------*/
project (ooRexx)
# The version of ooRexx to make
set (ORX_MAJOR 5)
set (ORX_MINOR 0)
set (ORX_MOD_LVL 0)
set (ORX_BLD_LVL 0)
set (ORX_VERSION ${ORX_MAJOR}.${ORX_MINOR}.${ORX_MOD_LVL})
set (ORX_NODOT_VERSION ${ORX_MAJOR}${ORX_MINOR}${ORX_MOD_LVL})
set (ORX_VER_STR ${ORX_MAJOR}.${ORX_MINOR}.${ORX_MOD_LVL})
string(TIMESTAMP ORX_YEAR %Y)
set (ORX_COPY_YEAR 2005-${ORX_YEAR})
string(TOUPPER ${CMAKE_SYSTEM_NAME} ORX_SYS_STR)

# Platform independent architecture test
if (CMAKE_SIZEOF_VOID_P MATCHES 8)
    set(ORX_ARCHITECTURE "64")
    message(STATUS "Building for a 64-bit architecture")
else ()
    set(ORX_ARCHITECTURE "32")
    message(STATUS "Building for a 32-bit architecture")
endif ()

find_package(Subversion)
if(SUBVERSION_FOUND AND IS_DIRECTORY ".svn")
  Subversion_WC_INFO(${PROJECT_SOURCE_DIR} ORX)
  set(ORX_BLD_LVL ${ORX_WC_REVISION})
endif()


message(STATUS "SVN Revision Number is ${ORX_BLD_LVL}")

if (WIN32)
# find a xalan executable, but only on windows for now.
    FIND_PROGRAM(XALAN_EXECUTABLE xalan
       DOC "xalan command line xslt converter")
endif()

# The following supports the versioning of the rexx shared libraries
# This defines the oorexx library version. For ooRexx this should always be the
# same as the ORX_MAJOR number.
set (ORX_CURRENT ${ORX_MAJOR})
# Each public release of ooRexx should increment this number by one except
# when the ORX_MINOR goes back to zero. If that is the case then this entry
# should also be set to zero.
set (ORX_REVISION 0)
# For ooRexx, this number should ALWAYS be zero. This will force the linker to
# utilize all previous releases for linking.
set (ORX_AGE 0)
# Please note that the version-info has nothing to do with the release version.
# You need to know exactly what you are doing in order to get this correct.
set(VERSION_INFO "-version-info ${ORX_CURRENT}:${ORX_REVISION}:${ORX_AGE}")
# This is version information set on library properties
set(ORX_VERSION ${ORX_CURRENT}.${ORX_AGE}.${ORX_REVISION})

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


# Common install variables
   SET(CPACK_PACKAGE_VERSION_MAJOR ${ORX_MAJOR})
   SET(CPACK_PACKAGE_VERSION_MINOR ${ORX_MINOR})
   SET(CPACK_PACKAGE_VERSION_PATCH ${ORX_MOD_LVL})
   set(CPACK_PACKAGE_NAME "ooRexx")
   set(CPACK_PACKAGE_LONG_NAME "Open Object Rexx")
   set(CPACK_PACKAGE_VENDOR "Rexx Language Association")
   set(CPACK_PACKAGE_DESCRIPTION_FILE ${CMAKE_SOURCE_DIR}/ReleaseNotes)
   set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Open Object Rexx Interpreter")
   set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/CPLv1.0.txt)
   set(CPACK_RESOURCE_FILE_README ${CMAKE_SOURCE_DIR}/ReleaseNotes)

# Set install variables
if (WIN32)
   # NOTE: specifying "." for the destination will place the files in the
   # root directory of the NSIS install.  This needs to be a "." to get
   # the files in the correct location.
   set (INSTALL_EXECUTABLE_DIR .)
   set (INSTALL_LIB_DIR .)
   set (INSTALL_CLS_DIR .)
   set (INSTALL_REX_DIR .)
   set (INSTALL_DOC_DIR doc)
   set (INSTALL_INCLUDE_DIR api)
   set (INSTALL_SAMPLES_DIR samples)

# NSIS builder locations

   # locations of NSIS directories.  The first is where we build from,
   # the second is source for any template files for configuring the build.
   set(NSIS_INSTALLER_DIR ${CMAKE_CURRENT_BINARY_DIR}/NSIS)
   set(NSIS_INSTALLER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${ORX_PLATFORM_DIR}/install)

   # NSIS version identifier that includes the build level
   set (ORX_VERSION_NSIS ${ORX_MAJOR}.${ORX_MINOR}.${ORX_MOD_LVL}.${ORX_BLD_LVL})

   # The DOC source dir is usually specified in native form...need to convert it
   # to the CMAKE format for the install commands.
   if (DOC_SOURCE_DIR)
      file(TO_CMAKE_PATH ${DOC_SOURCE_DIR} DOC_SOURCE_DIR)
   endif()

   # location where we assemble everything to build the installer
   set (NSIS_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/NSIS)
   # location for pulling together the install file set
   set (NSIS_FILE_DIR ${CMAKE_CURRENT_BINARY_DIR}/NSIS/files)

   # Create and add a shortcut item to the list for a component
   # This also adds entries to delete the equivalent shortcut
   MACRO(install_component_shortcut component name prog arg icon icon_index)
     set(NSIS_COMPONENT_SHORTCUTS_${component} "${NSIS_COMPONENT_SHORTCUTS_${component}}\n!insertmacro ConfigureShortCut \"${name}\" ${prog} \"${arg}\" ${icon} \"${icon_index}\"")
   ENDMACRO ()

   # Create and add a shortcut item to the list for a component for running a rexx program
   # This also adds entries to delete the equivalent shortcut
   MACRO(install_rexx_shortcut component name launcher program)
     set(NSIS_COMPONENT_SHORTCUTS_${component} "${NSIS_COMPONENT_SHORTCUTS_${component}}\n!insertmacro ConfigureRexxShortCut \"${name}\" ${launcher} \"${program}\"")
   ENDMACRO ()

   # Create and add a shortcut for a documentation element
   # This also adds entries to delete the equivalent shortcut
   MACRO(install_doc_shortcut name file)
     set(NSIS_COMPONENT_SHORTCUTS_Docs "${NSIS_COMPONENT_SHORTCUTS_Docs}\n!insertmacro ConfigureDocShortCut \"${name}\" \"${file}\"")
   ENDMACRO ()

else ()
   set (CMAKE_INSTALL_PREFIX /usr)
   set (catdir ${CMAKE_INSTALL_PREFIX}/bin)
   set (INSTALL_EXECUTABLE_DIR ${CMAKE_INSTALL_PREFIX}/bin)
   set (INSTALL_CLS_DIR ${CMAKE_INSTALL_PREFIX}/bin)
   set (INSTALL_REX_DIR ${CMAKE_INSTALL_PREFIX}/bin)
   set (INSTALL_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/include)
   set (INSTALL_SAMPLES_DIR ${CMAKE_INSTALL_PREFIX}/share/${CMAKE_PROJECT_NAME})
   find_path(INSTALL_LIB_DIR libc.so PATHS ${CMAKE_INSTALL_PREFIX}/lib/* ${CMAKE_INSTALL_PREFIX}/lib64)
   set (INSTALL_MAN_DIR ${CMAKE_INSTALL_PREFIX}/share/man)
endif ()

# Set compiler and linker flags common to all build environments
include_directories(${PROJECT_BINARY_DIR})
if (WIN32)
   add_definitions(/DORX_VER=${ORX_MAJOR}
            /DORX_REL=${ORX_MINOR}
            /DORX_MOD=${ORX_MOD_LVL}
            /DORX_BLD=${ORX_BLD_LVL}
            /DORX_VER_STR=${ORX_VER_STR}
            /DOOREXX_COPY_YEAR="${ORX_COPY_YEAR}"
            /DORX_SYS_STR="${CMAKE_SYSTEM_NAME}"
            /DORX_CATDIR="${catdir}"
            /DORX_SHARED_LIBRARY_EXT="${CMAKE_SHARED_LIBRARY_SUFFIX}"
            /D_CRT_SECURE_NO_DEPRECATE
            /D_CRT_SECURE_NO_WARNINGS
            /D_CRT_NONSTDC_NO_DEPRECATE
            /wd4291
            /DWIN32 /D_WINDOWS /DHAVE_CONFIG_H)
   if (${ORX_ARCHITECTURE} STREQUAL "64")
      set (EXE_MANIFEST_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${ORX_PLATFORM_DIR}/rexx64.exe.manifest)
      set (NSIS_CPU "x86_64")
   else ()
      set (EXE_MANIFEST_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${ORX_PLATFORM_DIR}/rexx32.exe.manifest)
      set (NSIS_CPU "x86_32")
   endif ()
   file(TO_NATIVE_PATH ${EXE_MANIFEST_FILE} EXE_MANIFEST_FILE)
   set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:524288")
else ()
   add_definitions(-DORX_VER=${ORX_MAJOR} -DORX_REL=${ORX_MINOR} -DORX_MOD=${ORX_MOD_LVL} -DORX_FIX=0
            -DOOREXX_COPY_YEAR="${ORX_COPY_YEAR}"
            -DORX_SYS_STR="${CMAKE_SYSTEM_NAME}"
            -DORX_CATDIR="${catdir}" -D_GNU_SOURCE
            -DORX_SHARED_LIBRARY_EXT="${CMAKE_SHARED_LIBRARY_SUFFIX}"
            -D${ORX_SYS_STR} -DOPSYS_${ORX_SYS_STR} -DHAVE_CONFIG_H)
    set (ORX_SYSLIB_DL dl)
    set (ORX_SYSLIB_PTHREAD pthread)
endif ()

# Check header files and functions. These are all defined to the config.h file.
check_include_file(ctype.h HAVE_CTYPE_H)
check_include_file(float.h HAVE_FLOAT_H)
check_include_file(limits.h HAVE_LIMITS_H)
check_include_file(locale.h HAVE_LOCALE_H)
check_include_file(malloc.h HAVE_MALLOC_H)
check_include_file(memory.h HAVE_MEMORY_H)
check_function_exists(memset HAVE_MEMSET)
check_function_exists(nsleep HAVE_NSLEEP)
check_function_exists(setlocale HAVE_SETLOCALE)
check_include_file(signal.h HAVE_SIGNAL_H)
check_include_file(stdarg.h HAVE_STDARG_H)
check_include_file(stddef.h HAVE_STDDEF_H)
check_include_file(stdio.h HAVE_STDIO_H)
check_include_file(stdlib.h HAVE_STDLIB_H)
check_include_file(string.h HAVE_STRING_H)
check_include_file(time.h HAVE_TIME_H)
check_function_exists(vprintf HAVE_VPRINTF)
check_include_file(fcntl.h HAVE_FCNTL_H)
check_function_exists(nanosleep HAVE_NANOSLEEP)
check_include_file(inttypes.h HAVE_INTTYPES_H)
check_include_file(stdint.h HAVE_STDINT_H)
if (NOT WIN32)
  check_include_file(attr/xattr.h HAVE_ATTR_XATTR_H)
  check_function_exists(catopen HAVE_CATOPEN)
  check_include_file(dlfcn.h HAVE_DLFCN_H)
  check_include_file(features.h HAVE_FEATURES_H)
  check_include_file(filehdr.h HAVE_FILEHDR_H)
  check_function_exists(fstat HAVE_FSTAT)
  check_function_exists(gcvt HAVE_GCVT)
  check_function_exists(geteuid HAVE_GETEUID)
  check_function_exists(getpgrp HAVE_GETPGRP)
  check_function_exists(getpwuid HAVE_GETPWUID)
  check_function_exists(gettimeofday HAVE_GETTIMEOFDAY)
  check_function_exists(getuserattr HAVE_GETUSERATTR)
  check_function_exists(IDtouser HAVE_IDTOUSER)
  find_library(HAVE_LIBPTHREAD pthread)
  find_library(HAVE_LIBRT rt)
  check_include_file(mesg.h HAVE_MESG_H)
  check_include_file(netinet/in.h HAVE_NETINET_IN_H)
  check_include_file(nl_types.h HAVE_NL_TYPES_H)
  check_include_file(pthread.h HAVE_PTHREAD_H)
  set(CMAKE_REQUIRED_LIBRARIES "pthread")
  set(CMAKE_REQUIRED_DEFINITIONS "-D_GNU_SOURCE=1")
  check_function_exists(pthread_mutexattr_settype HAVE_PTHREAD_MUTEXATTR_SETTYPE)
  check_c_source_compiles("#include <pthread.h>
                           int main(int arg, char **argv) {
                           int tryme;
                           tryme = PTHREAD_MUTEX_ERRORCHECK;
                           return 0;}"
                          HAVE_PTHREAD_MUTEX_ERRORCHECK)
  check_c_source_compiles("#include <pthread.h>
                           int main(int arg, char **argv) {
                           int tryme;
                           tryme = PTHREAD_MUTEX_RECURSIVE;
                           return 0;}"
                          HAVE_PTHREAD_MUTEX_RECURSIVE)
  check_include_file(pwd.h HAVE_PWD_H)
  check_include_file(sched.h HAVE_SCHED_H)
  check_function_exists(sighold HAVE_SIGHOLD)
  check_function_exists(strdup HAVE_STRDUP)
  check_include_file(strings.h HAVE_STRINGS_H)
  check_include_file(stropts.h HAVE_STROPTS_H)
  check_include_file(sys/filio.h HAVE_SYS_FILIO_H)
  check_include_file(sys/ldr.h HAVE_SYS_LDR_H)
  check_include_file(sys/resource.h HAVE_SYS_RESOURCE_H)
  check_include_file(sys/select.h HAVE_SYS_SELECT_H)
  check_include_file(sys/sem.h HAVE_SYS_SEM_H)
  check_include_file(sys/signal.h HAVE_SYS_SIGNAL_H)
  check_include_file(sys/socket.h HAVE_SYS_SOCKET_H)
  check_include_file(sys/stat.h HAVE_SYS_STAT_H)
  check_include_file(sys/syscall.h HAVE_SYS_SYSCALL_H)
  check_include_file(sys/time.h HAVE_SYS_TIME_H)
  check_include_file(sys/types.h HAVE_SYS_TYPES_H)
  check_include_file(sys/utsname.h HAVE_SYS_UTSNAME_H)
  check_include_file(sys/wait.h HAVE_SYS_WAIT_H)
  check_c_source_compiles("#include <sys/types.h>
                           #include <sys/sem.h>
                           int main(int arg, char **argv) {
                           union semun semopts;
                           return 0;}"
                          HAVE_UNION_SEMUN)
  check_include_file(unistd.h HAVE_UNISTD_H)
  check_include_file(usersec.h HAVE_USERSEC_H)
  check_include_file(ncurses.h HAVE_NCURSES_H)
endif ()
set (PACKAGE ${PROJECT_NAME})
set (PACKAGE_BUGREPORT "http://sourceforge.net/projects/oorexx/")
set (PACKAGE_NAME ${PROJECT_NAME})
set (PACKAGE_STRING ${PROJECT_NAME})
set (PACKAGE_TARNAME "ooRexx.tar")
set (PACKAGE_URL "http://www.oorexx.org/")
set (PACKAGE_VERSION ${ORX_VER_STR})
set (VERSION ${ORX_VER_STR})
set (STDC_HEADERS 1)


# Configure files config.h
configure_file(config.h.in.cmake ${PROJECT_BINARY_DIR}/config.h)



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
set (build_parser_dir ${build_interpreter_dir}/parser)
set (build_interpreter_common_dir ${build_interpreter_dir}/platform/common)
set (build_rexx_classes_dir ${build_interpreter_dir}/RexxClasses)
set (build_runtime_dir ${build_interpreter_dir}/runtime)
set (build_streamlibrary_dir ${build_interpreter_dir}/streamLibrary)
set (build_extensions_dir extensions)
set (build_extensions_platform_dir ${build_extensions_dir}/${ORX_PLATFORM_DIR})
set (build_extensions_rexxutil_dir ${build_extensions_dir}/rexxutil)
set (build_extensions_rexxutil_platform_dir ${build_extensions_rexxutil_dir}/${ORX_PLATFORM_DIR})
set (build_extensions_rxftp_dir ${build_extensions_dir}/rxftp)
set (build_extensions_csvstream_dir ${build_extensions_dir}/csvStream)
set (build_extensions_rxmath_dir ${build_extensions_dir}/rxmath)
set (build_extensions_rxregexp_dir ${build_extensions_dir}/rxregexp)
set (build_extensions_rxsock_dir ${build_extensions_dir}/rxsock)
set (build_extensions_orxncurses_dir ${build_extensions_dir}/orxncurses)
set (build_extensions_hostemu_dir ${build_extensions_dir}/hostemu)
set (build_extensions_hostemu_platform_dir ${build_extensions_hostemu_dir}/${ORX_PLATFORM_DIR})
set (build_utilities_dir utilities)
set (build_utilities_rexx_dir ${build_utilities_dir}/rexx/${ORX_PLATFORM_DIR})
set (build_utilities_rexxc_dir ${build_utilities_dir}/rexxc/${ORX_PLATFORM_DIR})
set (build_utilities_rexximage_dir ${build_utilities_dir}/rexximage)
set (build_utilities_rxqueue_dir ${build_utilities_dir}/rxqueue/${ORX_PLATFORM_DIR})
set (build_utilities_rxsubcom_dir ${build_utilities_dir}/rxsubcom/${ORX_PLATFORM_DIR})
set (build_api_dir api)
set (build_api_platform_dir api/${ORX_PLATFORM_DIR})
set (build_samples_dir samples)
set (api_dir ${build_api_dir}/${ORX_PLATFORM_DIR})

set(SAMPLES_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/${build_samples_dir})

if (WIN32)
   set (platform_samples_dir ${build_samples_dir}/windows)
else ()
   set (platform_samples_dir ${build_samples_dir}/unix)
endif ()

# The following are windows only
set (build_extensions_ole_dir ${build_extensions_dir}/platform/windows/ole)
set (build_extensions_oodialog_dir ${build_extensions_dir}/platform/windows/oodialog)
set (build_extensions_rxwinsys_dir ${build_extensions_dir}/platform/windows/rxwinsys)
set (build_utilities_rexxpaws_dir ${build_utilities_dir}/platform/windows/rexxpaws)
set (build_utilities_rexxhide_dir ${build_utilities_dir}/platform/windows/rexxhide)

if (WIN32)
# This replaces the /MD compile flag with /MT.
set(CompilerFlags
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
        )
foreach(CompilerFlag ${CompilerFlags})
  string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
endforeach()
endif ()

#/*----------------------------------------------------------------------------*/
#/* Build targets                                                              */
#/*----------------------------------------------------------------------------*/

#################### librexxapi.so ##########################
# Full set of files used in all platforms.
set (common_rexxapi_sources
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
# additional source files required by specific platforms
if (WIN32)
   set (platform_rexxapi_sources
            ${build_rexxapi_client_platform_dir}/rexxapi.def
            ${build_platform_dir}/verinfo.rc)
   set (platform_rexxapi_libs wsock32)
else ()
   set (platform_rexxapi_libs ${ORX_SYSLIB_DL} ${ORX_SYSLIB_PTHREAD})
endif ()
# Library definition
add_library(rexxapi SHARED
            ${common_rexxapi_sources}
            ${platform_rexxapi_sources})
# Include file definition
target_include_directories(rexxapi PUBLIC
            ${build_rexxapi_client_dir}
            ${build_rexxapi_client_platform_dir}
            ${build_rexxapi_common_dir}
            ${build_rexxapi_common_platform_dir}
            ${build_common_dir}
            ${build_common_platform_dir}
            ${build_lib_dir})
# Extra link library definitions
target_link_libraries(rexxapi ${platform_rexxapi_libs})

install(TARGETS rexxapi RUNTIME DESTINATION ${INSTALL_LIB_DIR} COMPONENT Core
                        LIBRARY DESTINATION ${INSTALL_LIB_DIR} COMPONENT Core
                        ARCHIVE DESTINATION ${INSTALL_INCLUDE_DIR} COMPONENT DevLib)
set_target_properties(rexxapi PROPERTIES VERSION ${ORX_VERSION})

#################### librexx.so ##########################
# Sources for librexx.so
set (interpreter_api_sources ${build_interpreter_api_dir}/CallContextStubs.cpp
            ${build_interpreter_api_dir}/InterpreterAPI.cpp
            ${build_interpreter_api_dir}/InterpreterInstanceStubs.cpp
            ${build_interpreter_api_dir}/MethodContextStubs.cpp
            ${build_interpreter_api_dir}/ThreadContextStubs.cpp)
set (classes_support_sources ${build_classes_support_dir}/HashCollection.cpp
            ${build_classes_support_dir}/HashContents.cpp
            ${build_classes_support_dir}/ListContents.cpp
            ${build_classes_support_dir}/ProgramMetaData.cpp
            ${build_classes_support_dir}/CompoundTableElement.cpp
            ${build_classes_support_dir}/CompoundVariableTable.cpp
            ${build_classes_support_dir}/CompoundVariableTail.cpp
            ${build_classes_support_dir}/RexxDateTime.cpp
            ${build_classes_support_dir}/StringUtil.cpp)
set (classes_sources ${build_classes_dir}/ArrayClass.cpp
            ${build_classes_dir}/BagClass.cpp
            ${build_classes_dir}/BufferClass.cpp
            ${build_classes_dir}/ClassClass.cpp
            ${build_classes_dir}/ContextClass.cpp
            ${build_classes_dir}/DirectoryClass.cpp
            ${build_classes_dir}/IntegerClass.cpp
            ${build_classes_dir}/ListClass.cpp
            ${build_classes_dir}/MessageClass.cpp
            ${build_classes_dir}/MethodClass.cpp
            ${build_classes_dir}/MutableBufferClass.cpp
            ${build_classes_dir}/NumberStringClass.cpp
            ${build_classes_dir}/NumberStringMath.cpp
            ${build_classes_dir}/NumberStringMath2.cpp
            ${build_classes_dir}/ObjectClass.cpp
            ${build_classes_dir}/PackageClass.cpp
            ${build_classes_dir}/PointerClass.cpp
            ${build_classes_dir}/QueueClass.cpp
            ${build_classes_dir}/RelationClass.cpp
            ${build_classes_dir}/RexxInfoClass.cpp
            ${build_classes_dir}/RoutineClass.cpp
            ${build_classes_dir}/RexxQueueMethods.cpp
            ${build_classes_dir}/SetClass.cpp
            ${build_classes_dir}/StemClass.cpp
            ${build_classes_dir}/StringClass.cpp
            ${build_classes_dir}/StringClassBit.cpp
            ${build_classes_dir}/StringClassConversion.cpp
            ${build_classes_dir}/StringClassMisc.cpp
            ${build_classes_dir}/StringClassSub.cpp
            ${build_classes_dir}/StringClassUtil.cpp
            ${build_classes_dir}/StringClassWord.cpp
            ${build_classes_dir}/StringTableClass.cpp
            ${build_classes_dir}/SupplierClass.cpp
            ${build_classes_dir}/TableClass.cpp
            ${build_classes_dir}/IdentityTableClass.cpp
            ${build_classes_dir}/WeakReferenceClass.cpp
            ${build_classes_dir}/StackFrameClass.cpp)
set (package_sources ${build_package_dir}/LibraryPackage.cpp
            ${build_package_dir}/PackageManager.cpp)
set (memory_sources ${build_memory_dir}/DeadObject.cpp
            ${build_memory_dir}/GlobalNames.cpp
            ${build_memory_dir}/MapBucket.cpp
            ${build_memory_dir}/MapTable.cpp
            ${build_memory_dir}/MemorySegment.cpp
            ${build_memory_dir}/MemoryStack.cpp
            ${build_memory_dir}/MemoryStats.cpp
            ${build_memory_dir}/NumberArray.cpp
            ${build_memory_dir}/PointerBucket.cpp
            ${build_memory_dir}/PointerTable.cpp
            ${build_memory_dir}/ProtectedObject.cpp
            ${build_memory_dir}/Envelope.cpp
            ${build_memory_dir}/InternalStack.cpp
            ${build_memory_dir}/SmartBuffer.cpp
            ${build_memory_dir}/UninitDispatcher.cpp
            ${build_memory_dir}/Setup.cpp
            ${build_memory_dir}/RexxMemory.cpp)
set (execution_sources ${build_execution_dir}/CPPCode.cpp
            ${build_execution_dir}/ActivationStack.cpp
            ${build_execution_dir}/RexxActivation.cpp
            ${build_execution_dir}/BaseCode.cpp
            ${build_execution_dir}/RexxCode.cpp
            ${build_execution_dir}/BaseExecutable.cpp
            ${build_execution_dir}/RexxLocalVariables.cpp
            ${build_execution_dir}/NativeActivation.cpp
            ${build_execution_dir}/NativeCode.cpp
            ${build_execution_dir}/RexxVariable.cpp
            ${build_execution_dir}/TrapHandler.cpp
            ${build_execution_dir}/VariableDictionary.cpp
            ${build_execution_dir}/SecurityManager.cpp
            ${build_execution_dir}/TraceSetting.cpp)
set (behaviour_sources ${build_behaviour_dir}/PrimitiveBehaviours.cpp
            ${build_behaviour_dir}/RexxBehaviour.cpp
            ${build_behaviour_dir}/MethodDictionary.cpp
            ${build_behaviour_dir}/VirtualFunctionTable.cpp)
set (concurrency_sources ${build_concurrency_dir}/ActivityDispatcher.cpp
            ${build_concurrency_dir}/ActivityManager.cpp
            ${build_concurrency_dir}/ActivationFrame.cpp
            ${build_concurrency_dir}/CallbackDispatcher.cpp
            ${build_concurrency_dir}/TrappingDispatcher.cpp
            ${build_concurrency_dir}/ExitHandler.cpp
            ${build_concurrency_dir}/CommandHandler.cpp
            ${build_concurrency_dir}/MessageDispatcher.cpp
            ${build_concurrency_dir}/Activity.cpp
            ${build_concurrency_dir}/RexxStartDispatcher.cpp
            ${build_concurrency_dir}/TranslateDispatcher.cpp)
set (expression_sources ${build_expression_dir}/BuiltinFunctions.cpp
            ${build_expression_dir}/CommonExternalFunctions.cpp
            ${build_expression_dir}/ExpressionClassResolver.cpp
            ${build_expression_dir}/ExpressionCompoundVariable.cpp
            ${build_expression_dir}/ExpressionDotVariable.cpp
            ${build_expression_dir}/ExpressionFunction.cpp
            ${build_expression_dir}/ExpressionMessage.cpp
            ${build_expression_dir}/ExpressionOperator.cpp
            ${build_expression_dir}/ExpressionQualifiedFunction.cpp
            ${build_expression_dir}/ExpressionStack.cpp
            ${build_expression_dir}/ExpressionStem.cpp
            ${build_expression_dir}/ExpressionVariable.cpp
            ${build_expression_dir}/IndirectVariableReference.cpp
            ${build_expression_dir}/ExpressionList.cpp
            ${build_expression_dir}/ExpressionLogical.cpp)
set (instructions_sources  ${build_instructions_dir}/AddressInstruction.cpp
            ${build_instructions_dir}/AssignmentInstruction.cpp
            ${build_instructions_dir}/BaseDoInstruction.cpp
            ${build_instructions_dir}/CallInstruction.cpp
            ${build_instructions_dir}/ClassDirective.cpp
            ${build_instructions_dir}/CommandInstruction.cpp
            ${build_instructions_dir}/DoBlock.cpp
            ${build_instructions_dir}/DoBlockComponents.cpp
            ${build_instructions_dir}/DoCountInstruction.cpp
            ${build_instructions_dir}/SimpleDoInstruction.cpp
            ${build_instructions_dir}/ControlledDoInstruction.cpp
            ${build_instructions_dir}/DoForeverInstruction.cpp
            ${build_instructions_dir}/DoOverInstruction.cpp
            ${build_instructions_dir}/DoWithInstruction.cpp
            ${build_instructions_dir}/DoWhileInstruction.cpp
            ${build_instructions_dir}/DropInstruction.cpp
            ${build_instructions_dir}/ElseInstruction.cpp
            ${build_instructions_dir}/EndIf.cpp
            ${build_instructions_dir}/EndInstruction.cpp
            ${build_instructions_dir}/ExitInstruction.cpp
            ${build_instructions_dir}/ExposeInstruction.cpp
            ${build_instructions_dir}/ForwardInstruction.cpp
            ${build_instructions_dir}/GuardInstruction.cpp
            ${build_instructions_dir}/IfInstruction.cpp
            ${build_instructions_dir}/InterpretInstruction.cpp
            ${build_instructions_dir}/LabelInstruction.cpp
            ${build_instructions_dir}/LeaveInstruction.cpp
            ${build_instructions_dir}/LibraryDirective.cpp
            ${build_instructions_dir}/MessageInstruction.cpp
            ${build_instructions_dir}/NopInstruction.cpp
            ${build_instructions_dir}/NumericInstruction.cpp
            ${build_instructions_dir}/OptionsInstruction.cpp
            ${build_instructions_dir}/OtherwiseInstruction.cpp
            ${build_instructions_dir}/ParseInstruction.cpp
            ${build_instructions_dir}/ParseTarget.cpp
            ${build_instructions_dir}/ParseTrigger.cpp
            ${build_instructions_dir}/ProcedureInstruction.cpp
            ${build_instructions_dir}/QueueInstruction.cpp
            ${build_instructions_dir}/RaiseInstruction.cpp
            ${build_instructions_dir}/ReplyInstruction.cpp
            ${build_instructions_dir}/RequiresDirective.cpp
            ${build_instructions_dir}/ReturnInstruction.cpp
            ${build_instructions_dir}/RexxInstruction.cpp
            ${build_instructions_dir}/SayInstruction.cpp
            ${build_instructions_dir}/SelectInstruction.cpp
            ${build_instructions_dir}/SignalInstruction.cpp
            ${build_instructions_dir}/ThenInstruction.cpp
            ${build_instructions_dir}/TraceInstruction.cpp
            ${build_instructions_dir}/UseInstruction.cpp
            ${build_instructions_dir}/UseLocalInstruction.cpp
            ${build_instructions_dir}/WhenCaseInstruction.cpp)
set (parser_sources ${build_parser_dir}/Clause.cpp
            ${build_parser_dir}/KeywordConstants.cpp
            ${build_parser_dir}/DirectiveParser.cpp
            ${build_parser_dir}/InstructionParser.cpp
            ${build_parser_dir}/LanguageParser.cpp
            ${build_parser_dir}/ProgramSource.cpp
            ${build_parser_dir}/Scanner.cpp
            ${build_parser_dir}/Token.cpp)
set (platform_sources ${build_interpreter_platform_dir}/ErrorMessages.cpp
            ${build_interpreter_platform_dir}/ExternalFunctions.cpp
            ${build_interpreter_platform_dir}/FileSystem.cpp
            ${build_interpreter_platform_dir}/MemorySupport.cpp
            ${build_interpreter_platform_dir}/MiscSystem.cpp
            ${build_interpreter_platform_dir}/SysActivity.cpp
            ${build_interpreter_platform_dir}/SysFileSystem.cpp
            ${build_interpreter_platform_dir}/SysInterpreterInstance.cpp
            ${build_interpreter_platform_dir}/SystemCommands.cpp
            ${build_interpreter_platform_dir}/SystemInitialization.cpp
            ${build_interpreter_platform_dir}/SystemInterpreter.cpp
            ${build_interpreter_platform_dir}/TimeSupport.cpp
            ${build_interpreter_platform_dir}/UseridFunction.cpp
            ${build_interpreter_platform_dir}/ValueFunction.cpp)
set (common_sources ${build_common_dir}/Utilities.cpp
            ${build_common_platform_dir}/SysFile.cpp
            ${build_common_platform_dir}/SysLibrary.cpp
            ${build_common_platform_dir}/SysSemaphore.cpp
            ${build_common_platform_dir}/SysThread.cpp)
set (runtime_sources ${build_runtime_dir}/InternalPackage.cpp
            ${build_runtime_dir}/Interpreter.cpp
            ${build_runtime_dir}/InterpreterInstance.cpp
            ${build_runtime_dir}/Numerics.cpp
            ${build_runtime_dir}/Version.cpp)
set (streamlibrary_sources ${build_streamlibrary_dir}/StreamCommandParser.cpp
            ${build_streamlibrary_dir}/StreamNative.cpp
            ${build_streamlibrary_dir}/FileNative.cpp)

# Full set of files used in all platforms.
set (common_interpreter_sources ${classes_sources}
            ${classes_support_sources}
            ${interpreter_api_sources}
            ${behaviour_sources}
            ${execution_sources}
            ${memory_sources}
            ${package_sources}
            ${concurrency_sources}
            ${expression_sources}
            ${instructions_sources}
            ${parser_sources}
            ${platform_sources}
            ${common_sources}
            ${runtime_sources}
            ${streamlibrary_sources})

# additional source files required by specific platforms
if (WIN32)
   set (platform_interpreter_sources ${build_interpreter_platform_dir}/rexx.def
            ${build_interpreter_platform_dir}/winmsgtb.rc)
   set (platform_interpreter_libs comctl32)
else ()
   set (platform_interpreter_sources ${build_interpreter_platform_dir}/RexxMain.cpp)
   set (platform_rexx_libs ${ORX_SYSLIB_DL} ${ORX_SYSLIB_PTHREAD})
endif ()

# Library definition
add_library(rexx SHARED
            ${common_interpreter_sources}
            ${platform_interpreter_sources})

set (core_platform_classes ${build_interpreter_platform_dir}/PlatformObjects.orx)
# Include file definition
target_include_directories(rexx PUBLIC
            ${build_lib_dir}
            ${build_api_dir}
            ${build_api_platform_dir}
            ${build_common_dir}
            ${build_common_platform_dir}
            ${build_interpreter_dir}
            ${build_behaviour_dir}
            ${build_execution_dir}
            ${build_memory_dir}
            ${build_package_dir}
            ${build_concurrency_dir}
            ${build_expression_dir}
            ${build_instructions_dir}
            ${build_classes_dir}
            ${build_classes_support_dir}
            ${build_runtime_dir}
            ${build_parser_dir}
            ${build_messages_dir}
            ${build_streamlibrary_dir}
            ${build_interpreter_common_dir}
            ${build_interpreter_platform_dir})

# Extra link library definitions
target_link_libraries(rexx rexxapi ${platform_interpreter_libs})
install(TARGETS rexx RUNTIME DESTINATION ${INSTALL_LIB_DIR} COMPONENT Core
                     LIBRARY DESTINATION ${INSTALL_LIB_DIR} COMPONENT Core
                     ARCHIVE DESTINATION ${INSTALL_INCLUDE_DIR} COMPONENT DevLib)
set_target_properties(rexx PROPERTIES VERSION ${ORX_VERSION})

#################### orexx (executable) ##########################
if (WIN32)
   set (platform_rexx_exe_sources
            ${build_platform_dir}/rexx.rc)
else ()
   set (platform_rexx_exe_libs ${ORX_SYSLIB_DL} ${ORX_SYSLIB_PTHREAD})
endif ()
# Sources for rexx.  Note, the target must not be rexx, since we already
# have a rexx library target
add_executable(rexx_exe ${build_utilities_rexx_dir}/rexx.cpp
                        ${platform_rexx_exe_sources})
# This allows the executable to have the same name as the library.
SET_TARGET_PROPERTIES(rexx_exe
  PROPERTIES OUTPUT_NAME rexx)

if (WIN32)
# We need to override the PDB output file name for rexx.exe so it doesn't overwrite
# the file for rexx.dll, which essentially makes the interpreter undebuggable on Windows
# we want to append, which is a little bit of a pain.
   get_property(link_flags TARGET rexx_exe PROPERTY LINK_FLAGS_DEBUG)
   set(link_flags "${link_flags} /PDB:bin\\orexx.pdb")
   set_target_properties(rexx_exe PROPERTIES LINK_FLAGS_DEBUG ${link_flags})

   get_property(link_flags TARGET rexx_exe PROPERTY LINK_FLAGS_RELWITHDEBINFO)
   set(link_flags "${link_flags} /PDB:bin\\orexx.pdb")
   set_target_properties(rexx_exe PROPERTIES LINK_FLAGS_RELWITHDEBINFO ${link_flags})
endif ()

# Include file definition
target_include_directories(rexx_exe PUBLIC
            ${build_lib_dir}
            ${build_api_dir}
            ${build_api_platform_dir}
            ${build_messages_dir})
# Extra link library definitions
target_link_libraries(rexx_exe rexx rexxapi ${platform_rexx_exe_libs})
# Merge in extra manifest information on Windows
if (WIN32)
add_custom_command(TARGET rexx_exe POST_BUILD
   COMMAND "mt.exe" -manifest \"${EXE_MANIFEST_FILE}\" "-inputresource:bin\\rexx.exe;#1" "-outputresource:bin\\rexx.exe;#1"
   COMMENT "Adding custom manifest to rexx.exe...")
endif ()

install(TARGETS rexx_exe RUNTIME COMPONENT Core DESTINATION ${INSTALL_EXECUTABLE_DIR})

#################### rexximage (bin file) ##########################
# These are the core classes built into the .img file.
set (core_classes ${build_rexx_classes_dir}/CoreClasses.orx
            ${build_rexx_classes_dir}/StreamClasses.orx)
# additional source files required by specific platforms
if (WIN32)
   set (platform_rexximage_sources
            ${build_platform_dir}/rexx.rc)
endif ()
# Sources for rexximg
add_executable(rexximage ${build_utilities_rexximage_dir}/rexximage.cpp
            ${core_classes}
            ${platform_rexximage_sources})
# Include file definition
target_include_directories(rexximage PUBLIC
            ${build_lib_dir}
            ${build_api_dir}
            ${build_api_platform_dir})
# Extra link library definitions
target_link_libraries(rexximage rexx rexxapi ${platform_rexximg_libs})

# handy macro for copying files from the source into the build directory.
macro (copy_build_file name source)
    ADD_CUSTOM_COMMAND(
       OUTPUT ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${name}
       COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/${source}/${name} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${name}
       DEPENDS ${source}/${name}
       )
endmacro (copy_build_file)

# Copy the class files to the build directory
copy_build_file(CoreClasses.orx ${build_rexx_classes_dir})
copy_build_file(StreamClasses.orx ${build_rexx_classes_dir})
copy_build_file(PlatformObjects.orx ${build_interpreter_platform_dir})

# These are the portable class files the image requires
set (image_class_files
   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/CoreClasses.orx
   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/StreamClasses.orx
   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/PlatformObjects.orx)

# On Windows, the ole support is also included in the image
if (WIN32)
  copy_build_file(orexxole.cls ${build_extensions_ole_dir})
  set (platform_rexx_img_depends orexxole
       ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/orexxole.cls)
endif ()

# Build the rexx.img file
add_custom_command(OUTPUT ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rexx.img
           COMMAND $<TARGET_FILE:rexximage>
           DEPENDS rexximage rxapi rexxutil ${image_class_files} ${platform_rexx_img_depends}
           WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_custom_target(rexx_img ALL DEPENDS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rexx.img)
install(PROGRAMS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rexx.img COMPONENT Core DESTINATION ${INSTALL_EXECUTABLE_DIR} PERMISSIONS OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE OWNER_READ GROUP_READ WORLD_READ)

#################### rxapi (executable) #########################
# additional source files required by specific platforms
if (WIN32)
   set (platform_rxapi_sources
            ${build_rexxapi_server_platform_dir}/APIService.cpp
            ${build_rexxapi_server_platform_dir}/rxapi.rc
            ${build_rexxapi_server_platform_dir}/rxapi.ico)
   set (rxapi_executable_type WIN32)
   set (platform_rxapi_libs wsock32 shlwapi)
else ()
   set (platform_rxapi_sources
            ${build_rexxapi_server_platform_dir}/linux/APIService.cpp)
   set (platform_rxapi_libs ${ORX_SYSLIB_DL} ${ORX_SYSLIB_PTHREAD})
endif ()
# Sources for rxapi
add_executable(rxapi ${rxapi_executable_type}
         ${build_rexxapi_server_dir}/APIServer.cpp
         ${build_rexxapi_server_dir}/APIServerInstance.cpp
         ${build_rexxapi_server_dir}/APIServerThread.cpp
         ${build_rexxapi_server_dir}/MacroSpaceManager.cpp
         ${build_rexxapi_server_dir}/QueueManager.cpp
         ${build_rexxapi_server_dir}/RegistrationManager.cpp
         ${build_rexxapi_common_dir}/RegistrationTable.cpp
         ${build_rexxapi_common_dir}/ServiceMessage.cpp
         ${build_rexxapi_common_platform_dir}/SysAPIManager.cpp
         ${build_rexxapi_common_platform_dir}/SysCSStream.cpp
         ${build_rexxapi_common_platform_dir}/SysProcess.cpp
         ${build_common_dir}/Utilities.cpp
         ${build_common_platform_dir}/SysSemaphore.cpp
         ${build_common_platform_dir}/SysThread.cpp
         ${platform_rxapi_sources})
# Include file definition
target_include_directories(rxapi PUBLIC
         ${build_rexxapi_server_dir})
# Extra link library definitions
target_link_libraries(rxapi rexx rexxapi ${platform_rxapi_libs})

# Kill the rxapi daemon whenever we have to build, otherwise the build
# will fail.  Unfortunately, the killing of the process is not immediate,
# so we attach this as a dependency to multiple early pieces of the project
# to give it a chance to free up the file.
if (WIN32)
   add_custom_target(kill_rexxapi
        ${CMAKE_SOURCE_DIR}/${ORX_PLATFORM_DIR}/killapi.bat
        COMMENT "Killing rxapi daemon...")
   # add this as a dependency for major bits to give it a chance to
   # execute early.
   add_dependencies(rexx kill_rexxapi)
   add_dependencies(rexxapi kill_rexxapi)
   add_dependencies(rxapi kill_rexxapi)
endif ()
install(TARGETS rxapi RUNTIME COMPONENT Core DESTINATION ${INSTALL_EXECUTABLE_DIR})

#################### message_headers #########################
# Autogenerate the message header files and the primitive classes files, but
# only if the xalan executable is available.
if (XALAN_EXECUTABLE)

    # handy macro for generating a file from the rexxmsg.xml file
    macro (generate_message_file target xsl)
        ADD_CUSTOM_COMMAND(
           OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${target}
           COMMAND ${XALAN_EXECUTABLE} -o ${CMAKE_CURRENT_SOURCE_DIR}/${target} ${build_messages_dir}/rexxmsg.xml ${xsl}
           DEPENDS ${build_messages_dir}/rexxmsg.xml ${xsl}
           COMMENT "Generating ${CMAKE_CURRENT_SOURCE_DIR}/${target}"
           WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
           )
    endmacro (generate_message_file)

    # handy macro for generating a file from the PrimitiveClasses.xml file
    macro (generate_class_header target xsl)
        ADD_CUSTOM_COMMAND(
           OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${build_behaviour_dir}/${target}
           COMMAND ${XALAN_EXECUTABLE} -o ${build_behaviour_dir}/${target} ${build_behaviour_dir}/PrimitiveClasses.xml ${build_behaviour_dir}/${xsl}
           DEPENDS ${build_behaviour_dir}/PrimitiveClasses.xml ${build_behaviour_dir}/${xsl}
           COMMENT "Generating ${CMAKE_CURRENT_SOURCE_DIR}/${build_behaviour_dir}/${target}"
           WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
           )
    endmacro (generate_class_header)

    generate_message_file(${build_messages_dir}/RexxErrorCodes.h ${build_messages_dir}/RexxErrorCodes.xsl)
    generate_message_file(${build_messages_dir}/RexxMessageNumbers.h ${build_messages_dir}/RexxMessageNumbers.xsl)
    generate_message_file(${build_messages_dir}/RexxMessageTable.h ${build_messages_dir}/RexxMessageTable.xsl)
    generate_message_file(${build_api_dir}/oorexxerrors.h ${build_messages_dir}/ApiErrorCodes.xsl)
    generate_message_file(${build_messages_dir}/errnums.xml ${build_messages_dir}/errnums.xsl)
    generate_message_file(${build_messages_dir}/errnumsrxqueue.xml ${build_messages_dir}/errnumsrxqueue.xsl)
    generate_message_file(${build_messages_dir}/errnumssubcom.xml ${build_messages_dir}/errnumssubcom.xsl)
    generate_message_file(${build_messages_dir}/errnumsrexxc.xml ${build_messages_dir}/errnumsrexxc.xsl)
    generate_message_file(${build_interpreter_dir}/platform/windows/winmsgtb.rc ${build_interpreter_dir}/platform/windows/WinMessageResource.xsl)
    generate_message_file(${build_interpreter_dir}/platform/unix/rexx.cat ${build_interpreter_dir}/platform/unix/UnixGencat.xsl)

    generate_class_header(PrimitiveBehaviourNames.h PrimitiveBehaviourNames.xsl)
    generate_class_header(PrimitiveBehaviours.cpp PrimitiveBehaviours.xsl)
    generate_class_header(VirtualFunctionTable.cpp VirtualFunctionTable.xsl)
    generate_class_header(ClassTypeCodes.h ClassTypeCodes.xsl)

    set (REXXMSG_FILES ${build_messages_dir}/RexxErrorCodes.h
                       ${build_messages_dir}/RexxMessageNumbers.h
                       ${build_messages_dir}/RexxMessageTable.h
                       ${build_api_dir}/oorexxerrors.h
                       ${build_messages_dir}/errnums.xml
                       ${build_messages_dir}/errnumsrxqueue.xml
                       ${build_messages_dir}/errnumssubcom.xml
                       ${build_messages_dir}/errnumsrexxc.xml
                       ${build_interpreter_dir}/platform/windows/winmsgtb.rc
                       ${build_interpreter_dir}/platform/unix/rexx.cat
                       ${build_behaviour_dir}/PrimitiveBehaviourNames.h
                       ${build_behaviour_dir}/PrimitiveBehaviours.cpp
                       ${build_behaviour_dir}/VirtualFunctionTable.cpp
                       ${build_behaviour_dir}/ClassTypeCodes.h)

    add_custom_target(message_headers ALL DEPENDS ${REXXMSG_FILES})

    # add dependencies to the main build pieces
    add_dependencies(rexx message_headers)
    add_dependencies(rexxapi message_headers)

endif()


#################### rexxc (executable) #########################
# additional source files required by specific platforms
if (WIN32)
   set (platform_rexxc_sources
            ${build_platform_dir}/rexx.rc)
else ()
   set (platform_rexxc_libs ${ORX_SYSLIB_DL} ${ORX_SYSLIB_PTHREAD})
endif ()
# Sources for rexxc
add_executable(rexxc ${build_utilities_rexxc_dir}/RexxCompiler.cpp
                     ${platform_rexxc_sources})
# Include file definition
target_include_directories(rexxc PUBLIC
            ${build_lib_dir}
            ${build_api_dir}
            ${build_api_platform_dir}
            ${build_messages_dir})
# Extra link library definitions
target_link_libraries(rexxc rexx rexxapi ${platform_rexxc_libs})
install(TARGETS rexxc RUNTIME COMPONENT Core DESTINATION ${INSTALL_EXECUTABLE_DIR})

#################### rxqueue (executable) #########################
# additional source files required by specific platforms
if (WIN32)
   set (platform_rxqueue_sources
            ${build_platform_dir}/verinfo.rc)
else ()
   set (platform_rxqueue_libs ${ORX_SYSLIB_DL} ${ORX_SYSLIB_PTHREAD})
endif ()
# Sources for rxqueue
add_executable(rxqueue ${build_utilities_rxqueue_dir}/rxqueue.cpp
                     ${platform_rxqueue_sources})
# Include file definition
target_include_directories(rxqueue PUBLIC
            ${build_lib_dir}
            ${build_api_dir}
            ${build_api_platform_dir}
            ${build_interpreter_platform_dir}
            ${build_messages_dir})
# Extra link library definitions
target_link_libraries(rxqueue rexx rexxapi ${platform_rxqueue_libs})
install(TARGETS rxqueue RUNTIME COMPONENT Core DESTINATION ${INSTALL_EXECUTABLE_DIR})

#################### rxsubcom (executable) #########################
# additional source files required by specific platforms
if (WIN32)
   set (platform_subcom_sources
            ${build_platform_dir}/verinfo.rc)
else ()
   set (platform_subcom_libs ${ORX_SYSLIB_DL} ${ORX_SYSLIB_PTHREAD})
endif ()
# Sources for rxsubcom
add_executable(rxsubcom ${build_utilities_rxsubcom_dir}/rxsubcom.cpp
                     ${platform_subcom_sources})
# Include file definition
target_include_directories(rxsubcom PUBLIC
             ${build_lib_dir}
             ${build_api_dir}
             ${build_api_platform_dir}
             ${build_interpreter_platform_dir}
             ${build_messages_dir})
# Extra link library definitions
target_link_libraries(rxsubcom rexx rexxapi ${platform_subcom_libs})
install(TARGETS rxsubcom RUNTIME COMPONENT Core DESTINATION ${INSTALL_EXECUTABLE_DIR})

#################### librexxutil.so ################
# additional source files required by specific platforms
if (WIN32)
   set (platform_rexxutil_sources
            ${build_extensions_rexxutil_platform_dir}/rexxutil.def
            ${build_platform_dir}/verinfo.rc)
   set (platform_rexxutil_libs shlwapi)
else ()
   set (platform_rexxutil_libs ${ORX_SYSLIB_DL} ${ORX_SYSLIB_PTHREAD})
endif ()
# Sources for librexxutil.so
add_library(rexxutil SHARED ${build_extensions_rexxutil_platform_dir}/rexxutil.cpp
                            ${platform_rexxutil_sources})
# Include file definition
target_include_directories(rexxutil PUBLIC
             ${build_lib_dir}
             ${build_api_dir}
             ${build_api_platform_dir}
             ${build_messages_dir})
# Extra link library definitions
target_link_libraries(rexxutil rexx rexxapi ${platform_rexxutil_libs})
install(TARGETS rexxutil RUNTIME COMPONENT Core DESTINATION ${INSTALL_LIB_DIR}
                         LIBRARY DESTINATION ${INSTALL_LIB_DIR} COMPONENT Core)
set_target_properties(rexxutil PROPERTIES VERSION ${ORX_VERSION})

#################### librxmath.so ################
# additional source files required by specific platforms
if (WIN32)
   set (platform_rxmath_sources
            ${build_platform_dir}/rxmath.def
            ${build_platform_dir}/verinfo.rc)
endif ()
# Sources for librxmath.so
add_library(rxmath SHARED ${build_extensions_rxmath_dir}/rxmath.cpp
                          ${platform_rxmath_sources})

# Include file definition
target_include_directories(rxmath PUBLIC
             ${build_lib_dir}
             ${build_api_dir}
             ${build_api_platform_dir})
# Extra link library definitions
target_link_libraries(rxmath rexx rexxapi ${platform_rxmath_libs})
install(TARGETS rxmath RUNTIME COMPONENT Core DESTINATION ${INSTALL_LIB_DIR}
                       LIBRARY DESTINATION ${INSTALL_LIB_DIR} COMPONENT Core)
set_target_properties(rxmath PROPERTIES VERSION ${ORX_VERSION})

#################### librxsock.so ################
# additional source files required by specific platforms
if (WIN32)
   set (platform_rxsock_sources
            ${build_platform_dir}/rxsock.def
            ${build_platform_dir}/verinfo.rc)
endif ()

# Sources for librxsock.so
add_library(rxsock SHARED ${build_extensions_rxsock_dir}/rxsock.cpp
             ${build_extensions_rxsock_dir}/rxsockfn.cpp
             ${platform_rxsock_sources})
# Extra link library definitions
target_link_libraries(rxsock rexx rexxapi ${platform_rxsock_libs})
install(TARGETS rxsock RUNTIME COMPONENT Core DESTINATION ${INSTALL_LIB_DIR}
                       LIBRARY DESTINATION ${INSTALL_LIB_DIR} COMPONENT Core)
set_target_properties(rxsock PROPERTIES VERSION ${ORX_VERSION})

#################### librxsock6.so ################
# Sources for librxsock.so
add_library(rxsock6 SHARED ${build_extensions_rxsock_dir}/orxsock6.cpp)
# additional source files required by specific platforms
if (WIN32)
   set (platform_rxsock_sources
            ${build_platform_dir}/rxsock6.def
            ${build_platform_dir}/verinfo.rc)
   # Extra link library definitions
   target_link_libraries(rxsock6 rexx rexxapi ${platform_rxsock_libs} ws2_32)
else ()
   # Extra link library definitions
   target_link_libraries(rxsock6 rexx rexxapi ${platform_rxsock_libs})
endif ()

install(TARGETS rxsock6 RUNTIME COMPONENT Core DESTINATION ${INSTALL_LIB_DIR}
                       LIBRARY DESTINATION ${INSTALL_LIB_DIR} COMPONENT Core)
set_target_properties(rxsock6 PROPERTIES VERSION ${ORX_VERSION})

#################### librxregexp.so ################
# additional source files required by specific platforms
if (WIN32)
   set (platform_rxregexp_sources
            ${build_platform_dir}/rxregexp.def
            ${build_platform_dir}/verinfo.rc)
endif ()

# Sources for librxregexp.so
add_library(rxregexp SHARED ${build_extensions_rxregexp_dir}/automaton.cpp
             ${build_extensions_rxregexp_dir}/dblqueue.cpp
             ${build_extensions_rxregexp_dir}/rxregexp.cpp
             ${platform_rxregexp_sources})
# Include file definition
target_include_directories(rxregexp PUBLIC
             ${build_lib_dir}
             ${build_api_dir}
             ${build_api_platform_dir}
             ${build_messages_dir})
# Extra link library definitions
target_link_libraries(rxregexp rexx rexxapi ${platform_rxregexp_libs})
install(TARGETS rxregexp RUNTIME COMPONENT Core DESTINATION ${INSTALL_LIB_DIR}
                         LIBRARY DESTINATION ${INSTALL_LIB_DIR} COMPONENT Core)
set_target_properties(rxregexp PROPERTIES VERSION ${ORX_VERSION})

#################### libhostemu.so ################
# additional source files required by specific platforms
if (WIN32)
   set (platform_hostemu_sources
            ${build_platform_dir}/hostemu.def
            ${build_platform_dir}/verinfo.rc)
else ()
   set (platform_hostemu_libs ${ORX_SYSLIB_DL} ${ORX_SYSLIB_PTHREAD})
endif ()
# Sources for libhostemu.so
# Note that we ignore the dependency on hostemu.ypp for now.
add_library(hostemu SHARED ${build_extensions_hostemu_platform_dir}/hostemu.cpp
             ${build_extensions_hostemu_dir}/cmdparse.cpp
             ${platform_hostemu_sources})
# Include file definition
target_include_directories(hostemu PUBLIC
             ${build_api_dir}
             ${build_api_platform_dir}
             ${build_extensions_hostemu_dir})
# Extra link library definitions
target_link_libraries(hostemu rexx rexxapi ${platform_hostemu_libs})
install(TARGETS hostemu RUNTIME COMPONENT Core DESTINATION ${INSTALL_LIB_DIR}
                        LIBRARY DESTINATION ${INSTALL_LIB_DIR} COMPONENT Core)
set_target_properties(hostemu PROPERTIES VERSION ${ORX_VERSION})

#################### librxunixsys.so ################
if (NOT WIN32)
# Sources for librxunixsys.so
add_library(rxunixsys SHARED ${build_extensions_platform_dir}/rxunixsys/rxunixsys.cpp)
# Include file definition
target_include_directories(rxunixsys PUBLIC
             ${build_api_dir}
             ${build_api_platform_dir}
             ${extensions_unix_dir}/rxunixsys)
# Extra link library definitions
target_link_libraries(rxunixsys rexx rexxapi)
set_target_properties(rxunixsys PROPERTIES VERSION ${ORX_VERSION})
install(TARGETS rxunixsys RUNTIME DESTINATION ${INSTALL_LIB_DIR}
                          LIBRARY DESTINATION ${INSTALL_LIB_DIR} COMPONENT Core)
endif ()

#################### liborxncurses.so ################
if (NOT WIN32)
# Sources for liborxncurses.so
add_library(orxncurses SHARED ${build_extensions_dir}/orxncurses/orxncurses.cpp)
# Include file definition
target_include_directories(orxncurses PUBLIC
             ${build_api_dir}
             ${build_api_platform_dir}
             ${build_extensions_dir}/orxncurses)
# Extra link library definitions
target_link_libraries(orxncurses rexx rexxapi ncurses)
set_target_properties(orxncurses PROPERTIES VERSION ${ORX_VERSION})
install(TARGETS orxncurses RUNTIME DESTINATION ${INSTALL_LIB_DIR}
                          LIBRARY DESTINATION ${INSTALL_LIB_DIR} COMPONENT Core)
endif ()

#################### orexxole.dll ################
if (WIN32)
# Sources for orexxole.dll
add_library(orexxole SHARED ${build_extensions_ole_dir}/orexxole.cpp
             ${build_extensions_ole_dir}/OLEVariant.cpp
             ${build_extensions_ole_dir}/events.cpp
             ${build_extensions_ole_dir}/orexxole.def
             ${build_platform_dir}/verinfo.rc)
# Include file definition
target_include_directories(orexxole PUBLIC
             ${build_extensions_ole_dir})
# Extra link library definitions
target_link_libraries(orexxole rexx rexxapi)
install(TARGETS orexxole RUNTIME COMPONENT Core DESTINATION ${INSTALL_LIB_DIR})
endif ()

#################### rxwinsys.dll ################
if (WIN32)
   # need to copy this to the binary dir
   copy_build_file(winsystm.cls ${build_extensions_rxwinsys_dir})
   # Sources for rxwinwys.dll
   add_library(rxwinsys SHARED ${build_extensions_rxwinsys_dir}/rxwinsys.cpp
                ${build_extensions_rxwinsys_dir}/rxwinsys.def
                ${build_platform_dir}/verinfo.rc)
   # Include file definition
   target_include_directories(rxwinsys PUBLIC
                ${build_extensions_rxwinsys_dir})
   # Extra link library definitions
   target_link_libraries(rxwinsys rexx rexxapi shlwapi)
   # Make sure winsystm.cls gets copied
   add_custom_target(winsystm_cls ALL DEPENDS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/winsystm.cls)
   #add_dependencies(rxwinsys ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/winsystm.cls)
   install(TARGETS rxwinsys RUNTIME COMPONENT Core DESTINATION ${INSTALL_LIB_DIR})
   install(PROGRAMS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/winsystm.cls COMPONENT Core DESTINATION ${INSTALL_EXECUTABLE_DIR})
endif ()

#################### oodialog.dll ################
if (WIN32)
   set (OOD_MAJOR 4)
   set (OOD_MINOR 2)
   set (OOD_MOD_LVL 3)
   set (OOD_BLD_LVL 9606)
   set (OOD_VER_STR "4.2.3.9606")
   set (OOD_COPY_YEAR "2005-2013")
   set (OOD_SVN_REVSION 9150)

   # Sources for oodialog.dll
   add_library(oodialog SHARED
                ${build_extensions_oodialog_dir}/APICommon.cpp
                ${build_extensions_oodialog_dir}/oodBarControls.cpp
                ${build_extensions_oodialog_dir}/oodBaseDialog.cpp
                ${build_extensions_oodialog_dir}/oodBasicControls.cpp
                ${build_extensions_oodialog_dir}/oodCommon.cpp
                ${build_extensions_oodialog_dir}/oodControl.cpp
                ${build_extensions_oodialog_dir}/oodData.cpp
                ${build_extensions_oodialog_dir}/oodDeviceGraphics.cpp
                ${build_extensions_oodialog_dir}/ooDialog.cpp
                ${build_extensions_oodialog_dir}/oodKeyboard.cpp
                ${build_extensions_oodialog_dir}/oodListView.cpp
                ${build_extensions_oodialog_dir}/oodMenu.cpp
                ${build_extensions_oodialog_dir}/oodMessaging.cpp
                ${build_extensions_oodialog_dir}/oodMouse.cpp
                ${build_extensions_oodialog_dir}/oodPackageEntry.cpp
                ${build_extensions_oodialog_dir}/oodPropertySheetDialog.cpp
                ${build_extensions_oodialog_dir}/oodResizableDialog.cpp
                ${build_extensions_oodialog_dir}/oodResources.cpp
                ${build_extensions_oodialog_dir}/oodRoutines.cpp
                ${build_extensions_oodialog_dir}/oodShared.cpp
                ${build_extensions_oodialog_dir}/oodShellObjects.cpp
                ${build_extensions_oodialog_dir}/oodToolTip.cpp
                ${build_extensions_oodialog_dir}/oodTreeView.cpp
                ${build_extensions_oodialog_dir}/oodUser.cpp
                ${build_extensions_oodialog_dir}/oodUtilities.cpp
                ${build_extensions_oodialog_dir}/oodViewControls.cpp
                ${build_extensions_oodialog_dir}/ooDialog.def
                ${build_extensions_oodialog_dir}/ooDialog.rc)
   # Override the compile definitions with ooDialog-exclusive defines
   set_property(TARGET oodialog
          PROPERTY COMPILE_DEFINITIONS
          OOD_VER=${OOD_MAJOR}
          OOD_REL=${OOD_MINOR}
          OOD_MOD=${OOD_MOD_LVL}
          OOD_BLD=${OOD_BLD_LVL}
          OOD_COPY_YEAR=${OOD_COPY_YEAR}
          OOD_VER_STR="ooDialog ${OOD_MAJOR}.${OOD_MINOR}.${OOD_MOD_LVL}.${OOD_BLD_LVL}"
          OOD_BIN_NAME="oodialog.dll")

   # Include file definition
   target_include_directories(oodialog PUBLIC
                ${build_extensions_oodialog_dir})
   # Extra link library definitions
   target_link_libraries(oodialog rexx rexxapi winmm comdlg32 comctl32 shlwapi oleacc uxtheme rpcrt4 propsys delayimp)
   install(TARGETS oodialog RUNTIME COMPONENT Core DESTINATION ${INSTALL_LIB_DIR})

   #################### ooDialog.exe (executable) ##########################
   # Sources for ooDialog.exe
   add_executable(oodialog_exe WIN32 ${build_extensions_oodialog_dir}/oodWinMain.cpp
                                     ${build_extensions_oodialog_dir}/oodShared.cpp
                                     ${build_extensions_oodialog_dir}/ooDialogWinMain.rc)
# We need to override the PDB output file name for oodialog.exe so it doesn't overwrite
# the file for oodialog.dll, which essentially makes oodialog undebuggable on Windows
# we want to append, which is a little bit of a pain.
   get_property(link_flags TARGET oodialog_exe PROPERTY LINK_FLAGS_DEBUG)
   set(link_flags "${link_flags} /PDB:bin\\ooDialog_exe.pdb")
   set_target_properties(oodialog_exe PROPERTIES LINK_FLAGS_DEBUG ${link_flags})

   get_property(link_flags TARGET oodialog_exe PROPERTY LINK_FLAGS_RELWITHDEBINFO)
   set(link_flags "${link_flags} /PDB:bin\\ooDialog_exe.pdb")
   set_target_properties(oodialog_exe PROPERTIES LINK_FLAGS_RELWITHDEBINFO ${link_flags})

   set_property(TARGET oodialog_exe
          PROPERTY COMPILE_DEFINITIONS
          OOD_VER=${OOD_MAJOR}
          OOD_REL=${OOD_MINOR}
          OOD_MOD=${OOD_MOD_LVL}
          OOD_BLD=${OOD_BLD_LVL}
          OOD_COPY_YEAR=${OOD_COPY_YEAR}
          OOD_VER_STR="ooDialog ${OOD_MAJOR}.${OOD_MINOR}.${OOD_MOD_LVL}.${OOD_BLD_LVL}"
          OOD_BIN_NAME="oodialog.exe")
   # This allows the executable to have the same name as the library.
   SET_TARGET_PROPERTIES(oodialog_exe PROPERTIES OUTPUT_NAME ooDialog)
   # Include file definition
   target_include_directories(oodialog_exe PUBLIC
                ${build_extensions_oodialog_dir})
   target_link_libraries(oodialog_exe rexx rexxapi shlwapi)
   add_custom_command(TARGET oodialog_exe POST_BUILD
      COMMAND "mt.exe" -manifest \"${EXE_MANIFEST_FILE}\" "-inputresource:bin\\ooDialog.exe;#1" "-outputresource:bin\\ooDialog.exe;#1"
      COMMENT "Adding custom manifest to ooDialog.exe...")
   install(TARGETS oodialog_exe RUNTIME COMPONENT Core DESTINATION ${INSTALL_EXECUTABLE_DIR})

   #################### ooDialog.com (executable) ##########################
   # Sources for ooDialog.com
   add_executable(oodialog_com ${build_extensions_oodialog_dir}/oodMain.cpp
                               ${build_extensions_oodialog_dir}/oodShared.cpp
                               ${build_extensions_oodialog_dir}/ooDialogMain.rc)
# We need to override the PDB output file name for oodialog.com so it doesn't overwrite
# the file for oodialog.dll, which essentially makes oodialog undebuggable on Windows
# we want to append, which is a little bit of a pain.
   get_property(link_flags TARGET oodialog_com PROPERTY LINK_FLAGS_DEBUG)
   set(link_flags "${link_flags} /PDB:bin\\ooDialog_com.pdb")
   set_target_properties(oodialog_com PROPERTIES LINK_FLAGS_DEBUG ${link_flags})

   get_property(link_flags TARGET oodialog_com PROPERTY LINK_FLAGS_RELWITHDEBINFO)
   set(link_flags "${link_flags} /PDB:bin\\ooDialog_com.pdb")
   set_target_properties(oodialog_com PROPERTIES LINK_FLAGS_RELWITHDEBINFO ${link_flags})

   set_property(TARGET oodialog_com
          PROPERTY COMPILE_DEFINITIONS
          OOD_VER=${OOD_MAJOR}
          OOD_REL=${OOD_MINOR}
          OOD_MOD=${OOD_MOD_LVL}
          OOD_BLD=${OOD_BLD_LVL}
          OOD_COPY_YEAR=${OOD_COPY_YEAR}
          OOD_VER_STR="ooDialog ${OOD_MAJOR}.${OOD_MINOR}.${OOD_MOD_LVL}.${OOD_BLD_LVL}"
          OOD_BIN_NAME="oodialog.com")
   # This allows the executable to have the same name as the library.
   SET_TARGET_PROPERTIES(oodialog_com PROPERTIES OUTPUT_NAME ooDialog)
   SET_TARGET_PROPERTIES(oodialog_com PROPERTIES SUFFIX .com)
   # Include file definition
   target_include_directories(oodialog_com PUBLIC
                ${build_extensions_oodialog_dir})
   target_link_libraries(oodialog_com rexx rexxapi shell32 Advapi32)
   add_custom_command(TARGET oodialog_com POST_BUILD
      COMMAND "mt.exe" -manifest \"${EXE_MANIFEST_FILE}\" "-inputresource:bin\\ooDialog.com;#1" "-outputresource:bin\\ooDialog.com;#1"
      COMMENT "Adding custom manifest to ooDialog.com...")
   install(TARGETS oodialog_com RUNTIME COMPONENT Core DESTINATION ${INSTALL_EXECUTABLE_DIR})


   # Generate the oodialog .cls files used for the ::requires.  These are generated from
   # from a set of individual source files
   set (oodialog_source_classFiles
      ${build_extensions_oodialog_dir}/UtilityClasses.cls
      ${build_extensions_oodialog_dir}/AnimatedButton.cls
      ${build_extensions_oodialog_dir}/BaseDialog.cls
      ${build_extensions_oodialog_dir}/ControlDialog.cls
      ${build_extensions_oodialog_dir}/DialogControls.cls
      ${build_extensions_oodialog_dir}/ListView.cls
      ${build_extensions_oodialog_dir}/TreeView.cls
      ${build_extensions_oodialog_dir}/ToolTip.cls
      ${build_extensions_oodialog_dir}/DialogExtensions.cls
      ${build_extensions_oodialog_dir}/DynamicDialog.cls
      ${build_extensions_oodialog_dir}/EventNotification.cls
      ${build_extensions_oodialog_dir}/PlainBaseDialog.cls
      ${build_extensions_oodialog_dir}/RcDialog.cls
      ${build_extensions_oodialog_dir}/Menu.cls
      ${build_extensions_oodialog_dir}/ResDialog.cls
      ${build_extensions_oodialog_dir}/UserDialog.cls
      ${build_extensions_oodialog_dir}/DeprecatedClasses.cls
      ${build_extensions_oodialog_dir}/ShellObjects.cls)


   # We run a rexx program to generate these files, so we need to have the interpreter
   # fully built first.  The files are generated into the output directory.
   add_custom_command(OUTPUT ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/oodPlain.cls ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/oodWin32.cls ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ooDialog.cls
              COMMAND $<TARGET_FILE:rexx_exe> build_ooDialog_cls.rex ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
              DEPENDS rexx_exe rexx_img ${build_extensions_oodialog_dir}/build_ooDialog_cls.rex ${oodialog_source_classfiles}
              WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/${build_extensions_oodialog_dir})
   add_custom_target(oodialog_cls ALL DEPENDS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/oodPlain.cls ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/oodWin32.cls ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ooDialog.cls)

   install(PROGRAMS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/oodPlain.cls COMPONENT Core DESTINATION ${INSTALL_EXECUTABLE_DIR})
   install(PROGRAMS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/oodWin32.cls COMPONENT Core DESTINATION ${INSTALL_EXECUTABLE_DIR})
   install(PROGRAMS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ooDialog.cls COMPONENT Core DESTINATION ${INSTALL_EXECUTABLE_DIR})
endif ()


#################### rexxhide (executable) ##########################
if (WIN32)
   # Sources for rexxhide.
   add_executable(rexxhide WIN32 ${build_utilities_rexxhide_dir}/rexxhide.cpp
                           ${build_platform_dir}/rexx.rc)
   # Include file definition
   target_include_directories(rexxhide PUBLIC
               ${build_lib_dir}
               ${build_api_dir}
               ${build_api_platform_dir}
               ${build_messages_dir})
   target_link_libraries(rexxhide rexx rexxapi)
   add_custom_command(TARGET rexxhide POST_BUILD
      COMMAND "mt.exe" -manifest \"${EXE_MANIFEST_FILE}\" "-inputresource:bin\\rexxhide.exe;#1" "-outputresource:bin\\rexxhide.exe;#1"
      COMMENT "Adding custom manifest to rexxhide.exe...")
   install(TARGETS rexxhide RUNTIME COMPONENT Core DESTINATION ${INSTALL_EXECUTABLE_DIR})
endif ()


#################### rexxpaws (executable) ##########################
if (WIN32)
   # Sources for rexxpaws.
   add_executable(rexxpaws ${build_utilities_rexxpaws_dir}/rexxpaws.cpp
                           ${build_platform_dir}/rexx.rc)
   # Include file definition
   target_include_directories(rexxhide PUBLIC
               ${build_lib_dir}
               ${build_api_dir}
               ${build_api_platform_dir}
               ${build_messages_dir})
   target_link_libraries(rexxpaws rexx rexxapi)
   add_custom_command(TARGET rexxpaws POST_BUILD
      COMMAND "mt.exe" -manifest \"${EXE_MANIFEST_FILE}\" "-inputresource:bin\\rexxpaws.exe;#1" "-outputresource:bin\\rexxpaws.exe;#1"
      COMMENT "Adding custom manifest to rexxpaws.exe...")
   install(TARGETS rexxpaws RUNTIME COMPONENT Core DESTINATION ${INSTALL_EXECUTABLE_DIR})
endif ()


#################### misc files ##########################

# These artifacts apply to all platforms
# For the .cls class files, we copy these to the build directory so that the build has a full
# working copy of the class files for testing/development purposes
copy_build_file(rxregexp.cls ${build_extensions_rxregexp_dir})
copy_build_file(rxftp.cls ${build_extensions_rxftp_dir})
copy_build_file(csvStream.cls ${build_extensions_csvstream_dir})
copy_build_file(socket.cls ${build_extensions_rxsock_dir})
copy_build_file(socket6.cls ${build_extensions_rxsock_dir})
copy_build_file(streamsocket.cls ${build_extensions_rxsock_dir})
copy_build_file(streamsocket6.cls ${build_extensions_rxsock_dir})
copy_build_file(mime.cls ${build_extensions_rxsock_dir})
copy_build_file(smtp.cls ${build_extensions_rxsock_dir})
copy_build_file(smtp6.cls ${build_extensions_rxsock_dir})
copy_build_file(ncurses.cls ${build_extensions_orxncurses_dir})
copy_build_file(rexxtry.rex ${build_samples_dir})
add_custom_target(class_files ALL DEPENDS
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rxregexp.cls
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rxftp.cls
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/csvStream.cls
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/socket.cls
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/streamsocket.cls
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mime.cls
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/smtp.cls
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ncurses.cls
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rexxtry.rex)

install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rxregexp.cls COMPONENT Core DESTINATION ${INSTALL_CLS_DIR})

install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rxftp.cls COMPONENT Core DESTINATION ${INSTALL_CLS_DIR})

install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/csvStream.cls COMPONENT Core DESTINATION ${INSTALL_CLS_DIR})

install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/socket.cls COMPONENT Core DESTINATION ${INSTALL_CLS_DIR})
install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/streamsocket.cls COMPONENT Core DESTINATION ${INSTALL_CLS_DIR})
install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mime.cls COMPONENT Core DESTINATION ${INSTALL_CLS_DIR})
install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/smtp.cls COMPONENT Core DESTINATION ${INSTALL_CLS_DIR})
install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ncurses.cls COMPONENT Core DESTINATION ${INSTALL_CLS_DIR})
# technically a sample, but this is so useful we add it to the binary dir
install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/${build_samples_dir}/rexxtry.rex COMPONENT Core DESTINATION ${INSTALL_EXECUTABLE_DIR})

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${build_api_dir}/rexx.h COMPONENT DevLib DESTINATION ${INSTALL_INCLUDE_DIR})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${build_api_dir}/rexxapidefs.h COMPONENT DevLib DESTINATION ${INSTALL_INCLUDE_DIR})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${build_api_platform_dir}/rexxapitypes.h COMPONENT DevLib DESTINATION ${INSTALL_INCLUDE_DIR})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${build_api_platform_dir}/rexxplatformapis.h COMPONENT DevLib DESTINATION ${INSTALL_INCLUDE_DIR})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${build_api_platform_dir}/rexxplatformdefs.h COMPONENT DevLib DESTINATION ${INSTALL_INCLUDE_DIR})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${build_api_dir}/oorexxapi.h COMPONENT DevLib DESTINATION ${INSTALL_INCLUDE_DIR})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${build_api_dir}/oorexxerrors.h COMPONENT DevLib DESTINATION ${INSTALL_INCLUDE_DIR})

# common sample programs
install(PROGRAMS ${SAMPLES_SOURCE}/rexxcps.rex COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR})
install(PROGRAMS ${SAMPLES_SOURCE}/ccreply.rex COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR})
install(PROGRAMS ${SAMPLES_SOURCE}/complex.rex COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR})
install(PROGRAMS ${SAMPLES_SOURCE}/greply.rex COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR})
install(PROGRAMS ${SAMPLES_SOURCE}/guess.rex COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR})
install(PROGRAMS ${SAMPLES_SOURCE}/ktguard.rex COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR})
install(PROGRAMS ${SAMPLES_SOURCE}/makestring.rex COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR})
install(PROGRAMS ${SAMPLES_SOURCE}/month.rex COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR})
install(PROGRAMS ${SAMPLES_SOURCE}/philfork.rex COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR})
install(PROGRAMS ${SAMPLES_SOURCE}/pipe.rex COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR})
install(PROGRAMS ${SAMPLES_SOURCE}/properties.rex COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR})
install(PROGRAMS ${SAMPLES_SOURCE}/qdate.rex COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR})
install(PROGRAMS ${SAMPLES_SOURCE}/qtime.rex COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR})
install(PROGRAMS ${SAMPLES_SOURCE}/scclient.rex COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR})
install(PROGRAMS ${SAMPLES_SOURCE}/scserver.rex COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR})
install(PROGRAMS ${SAMPLES_SOURCE}/semcls.rex COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR})
install(PROGRAMS ${SAMPLES_SOURCE}/sfclient.rex COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR})
install(PROGRAMS ${SAMPLES_SOURCE}/sfserver.rex COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR})
install(PROGRAMS ${SAMPLES_SOURCE}/stack.rex COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR})
install(PROGRAMS ${SAMPLES_SOURCE}/usecomp.rex COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR})
install(PROGRAMS ${SAMPLES_SOURCE}/usepipe.rex COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR})
install(PROGRAMS ${SAMPLES_SOURCE}/arithmeticEvaluation.rex COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR})
install(PROGRAMS ${SAMPLES_SOURCE}/arrayCallback.rex COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR})
install(PROGRAMS ${SAMPLES_SOURCE}/concurrency.rex COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR})
install(PROGRAMS ${SAMPLES_SOURCE}/constrained.rex COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR})
install(PROGRAMS ${SAMPLES_SOURCE}/timezone.rex COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR})
install(PROGRAMS ${SAMPLES_SOURCE}/delegation.rex COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR})
install(PROGRAMS ${SAMPLES_SOURCE}/dynamicMethod.rex COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR})
install(PROGRAMS ${SAMPLES_SOURCE}/interface.rex COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR})
install(PROGRAMS ${SAMPLES_SOURCE}/singleLinkedList.rex COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR})
install(PROGRAMS ${SAMPLES_SOURCE}/sortComposite.rex COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR})
install(PROGRAMS ${SAMPLES_SOURCE}/synchronousConcurrency.rex COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR})
install(FILES ${SAMPLES_SOURCE}/jabberwocky.txt COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR})
install(PROGRAMS ${SAMPLES_SOURCE}/treeTraversal.rex COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR})
install(PROGRAMS ${SAMPLES_SOURCE}/treeDirectory.cls COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR})
install(PROGRAMS ${SAMPLES_SOURCE}/usetree.rex COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR})

if (NOT WIN32)
   add_custom_command(OUTPUT rexx.cat
              COMMAND gencat rexx.cat ${CMAKE_CURRENT_SOURCE_DIR}/${build_messages_dir}/gencat.inp
              DEPENDS ${image_class_files} ${platform_rexx_img_depends}
              WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
   add_custom_target(rexx_cat ALL DEPENDS rexx.cat)
   install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rexx.cat DESTINATION ${catdir} PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE)

   macro (add_manpage name cat)
      add_custom_command(OUTPUT ${name}.${cat}.gz
              COMMAND gzip -c
              ${CMAKE_CURRENT_SOURCE_DIR}/${build_platform_dir}/${name}.${cat}
              > ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${name}.${cat}.gz
              DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${build_platform_dir}/${name}.${cat})
      add_custom_target(${name}_man ALL DEPENDS ${name}.${cat}.gz)
      install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${name}.${cat}.gz
          DESTINATION ${INSTALL_MAN_DIR}/man${cat}
          PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
   endmacro (add_manpage)

   add_manpage (rexx 1)
   add_manpage (rexxc 1)
   add_manpage (rxqueue 1)
   add_manpage (rxsubcom 1)

   install(FILES rexxapi/server/platform/unix/rxapid DESTINATION ${INSTALL_EXECUTABLE_DIR} PERMISSIONS OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE OWNER_READ GROUP_READ WORLD_READ)
   install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${build_platform_dir}/rexx.sh DESTINATION ${CMAKE_INSTALL_PREFIX}/share/${CMAKE_PROJECT_NAME} PERMISSIONS OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE OWNER_READ GROUP_READ WORLD_READ)
   install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${build_platform_dir}/rexx.csh DESTINATION ${CMAKE_INSTALL_PREFIX}/share/${CMAKE_PROJECT_NAME} PERMISSIONS OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE OWNER_READ GROUP_READ WORLD_READ)
   install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${build_platform_dir}/rxapid.service DESTINATION ${CMAKE_INSTALL_PREFIX}/share/${CMAKE_PROJECT_NAME})

   install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${build_samples_dir}/readme DESTINATION ${CMAKE_INSTALL_PREFIX}/share/${CMAKE_PROJECT_NAME})
endif ()


# direct output into a samples subdir
add_subdirectory (samples)

# build the binaries used for API tests.
add_subdirectory (testbinaries)


#################### NSIS Package Targets ##########################
#################### NSIS Package Targets ##########################
#################### NSIS Package Targets ##########################
#################### NSIS Package Targets ##########################
#################### NSIS Package Targets ##########################

# The Windows installer assumes we have the docs available.  Don't try building
# any of the installer stuff if this is not available.
if (WIN32 AND DOC_SOURCE_DIR)
   # Add the install directory to the module path so the custom template can be located.
   set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${build_platform_dir}/install ${CMAKE_MODULE_PATH})

   # We do not use the NSIS generator directly, but rather configure our
   # own NSIS installation from a template
   set(CPACK_PACKAGE_INSTALL_DIRECTORY "Open Object Rexx")
   set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "Open Object Rexx")
   # ask for a modify the pack dialg
   set(CPACK_NSIS_INSTALLED_ICON_NAME "rexx.exe,0")
   set(CPACK_NSIS_URL_INFO_ABOUT "http://www.oorexx.org")
   set(CPACK_NSIS_MUI_ICON ${CMAKE_CURRENT_SOURCE_DIR}/${build_platform_dir}/rexx.ico)
   set(CPACK_NSIS_MUI_UNIICON ${CMAKE_CURRENT_SOURCE_DIR}/${build_platform_dir}/install/uninstall.ico)

   # Not sure why this is not working.  Claims it cannoth file the file, but it exists.
   # It might need to be an installed file...not sure
   # set(CPACK_PACKAGE_ICON ${CMAKE_CURRENT_SOURCE_DIR}/${build_platform_dir}/rexx.ico)

   # Component setups
   set(CPACK_COMPONENT_CORE_DISPLAY_NAME "Open Object Rexx Core (required)")
   set(CPACK_COMPONENT_CORE_DESCRIPTION "The Open Object Rexx interpreter and extension libraries.")
   set(CPACK_COMPONENT_CORE_REQUIRED 1)
   set(CPACK_COMPONENT_SAMPLES_DISPLAY_NAME "Open Object Rexx Samples")
   set(CPACK_COMPONENT_SAMPLES_DESCRIPTION "Sample programs that demonstrate Open Object Rexx features.")
   set(CPACK_COMPONENT_DEVLIB_DISPLAY_NAME "Open Object Rexx Development Kit")
   set(CPACK_COMPONENT_DEVLIB_DESCRIPTION "Header files and libraries for using the Open Object Rexx native programming interfaces.")
   set(CPACK_COMPONENT_DOCS_DISPLAY_NAME "Open Object Rexx Documentation")
   set(CPACK_COMPONENT_DOCS_DESCRIPTION "Complete documentation for Open Object Rexx in PDF form.")

   # The Docs are an optional build step, so we only add them if we have
   # been configured with a doc source directory
   set(NSIS_INSTALL_COMPONENTS Core Docs DevLib Samples)


   # Add addititional Windows-only install files
   install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${build_platform_dir}/rexx.ico COMPONENT Core DESTINATION ${INSTALL_EXECUTABLE_DIR})
   install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/CPLv1.0.txt COMPONENT Core DESTINATION ${INSTALL_EXECUTABLE_DIR})

   # if this is a Debug or RelWithDebInfo configuration, also install the .pdb files in the binary locations
   install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/hostemu.pdb COMPONENT Core DESTINATION ${INSTALL_EXECUTABLE_DIR}
       CONFIGURATIONS Debug RelWithDebInfo)
   install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/oodialog.pdb COMPONENT Core DESTINATION ${INSTALL_EXECUTABLE_DIR}
       CONFIGURATIONS Debug RelWithDebInfo)
   install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/oodialog_com.pdb COMPONENT Core DESTINATION ${INSTALL_EXECUTABLE_DIR}
       CONFIGURATIONS Debug RelWithDebInfo)
   install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/oodialog_exe.pdb COMPONENT Core DESTINATION ${INSTALL_EXECUTABLE_DIR}
       CONFIGURATIONS Debug RelWithDebInfo)
   install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/orexx.pdb COMPONENT Core DESTINATION ${INSTALL_EXECUTABLE_DIR}
       CONFIGURATIONS Debug RelWithDebInfo)
   install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/orexxole.pdb COMPONENT Core DESTINATION ${INSTALL_EXECUTABLE_DIR}
       CONFIGURATIONS Debug RelWithDebInfo)
   install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rexx.pdb COMPONENT Core DESTINATION ${INSTALL_EXECUTABLE_DIR}
       CONFIGURATIONS Debug RelWithDebInfo)
   install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rexxapi.pdb COMPONENT Core DESTINATION ${INSTALL_EXECUTABLE_DIR}
       CONFIGURATIONS Debug RelWithDebInfo)
   install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rexxc.pdb COMPONENT Core DESTINATION ${INSTALL_EXECUTABLE_DIR}
       CONFIGURATIONS Debug RelWithDebInfo)
   install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rexxhide.pdb COMPONENT Core DESTINATION ${INSTALL_EXECUTABLE_DIR}
       CONFIGURATIONS Debug RelWithDebInfo)
   install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rexximage.pdb COMPONENT Core DESTINATION ${INSTALL_EXECUTABLE_DIR}
       CONFIGURATIONS Debug RelWithDebInfo)
   install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rexxpaws.pdb COMPONENT Core DESTINATION ${INSTALL_EXECUTABLE_DIR}
       CONFIGURATIONS Debug RelWithDebInfo)
   install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rexxutil.pdb COMPONENT Core DESTINATION ${INSTALL_EXECUTABLE_DIR}
       CONFIGURATIONS Debug RelWithDebInfo)
   install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rxapi.pdb COMPONENT Core DESTINATION ${INSTALL_EXECUTABLE_DIR}
       CONFIGURATIONS Debug RelWithDebInfo)
   install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rxmath.pdb COMPONENT Core DESTINATION ${INSTALL_EXECUTABLE_DIR}
       CONFIGURATIONS Debug RelWithDebInfo)
   install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rxqueue.pdb COMPONENT Core DESTINATION ${INSTALL_EXECUTABLE_DIR}
       CONFIGURATIONS Debug RelWithDebInfo)
   install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rxregexp.pdb COMPONENT Core DESTINATION ${INSTALL_EXECUTABLE_DIR}
       CONFIGURATIONS Debug RelWithDebInfo)
   install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rxsock.pdb COMPONENT Core DESTINATION ${INSTALL_EXECUTABLE_DIR}
       CONFIGURATIONS Debug RelWithDebInfo)
   install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rxsubcom.pdb COMPONENT Core DESTINATION ${INSTALL_EXECUTABLE_DIR}
       CONFIGURATIONS Debug RelWithDebInfo)
   install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rxwinsys.pdb COMPONENT Core DESTINATION ${INSTALL_EXECUTABLE_DIR}
       CONFIGURATIONS Debug RelWithDebInfo)

   # technically a sample, but this is so useful we add it to the binary dir
   install(PROGRAMS ${SAMPLES_SOURCE}/windows/oodialog/ooRexxTry/ooRexxTry.rex COMPONENT Core DESTINATION ${INSTALL_EXECUTABLE_DIR})

   # add extra shortcuts (uninstall one is automatically added)
   install_rexx_shortcut(Core "Try Rexx" rexx rexxtry.rex)
   install_rexx_shortcut(Core "Try Rexx (GUI)" rexx ooRexxTry.rex)
   install_component_shortcut(Core "ooRexx License" CPLv1.0.txt "" CPLv1.0.txt 0)

   # now the samples

   install(FILES ${SAMPLES_SOURCE}/windows/0ReadMe.first COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR})
   install(PROGRAMS ${SAMPLES_SOURCE}/windows/rexutils/drives.rex COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR})

   # misc samples
   install(FILES ${SAMPLES_SOURCE}/windows/misc/fileDrop.empty COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR}/misc)
   install(FILES ${SAMPLES_SOURCE}/windows/misc/fileDrop.input COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR}/misc)
   install(FILES ${SAMPLES_SOURCE}/windows/misc/fileDrop.readMe COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR}/misc)
   install(PROGRAMS ${SAMPLES_SOURCE}/windows/misc/fileDrop.rex COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR}/misc)

   # ole samples
   install(DIRECTORY ${SAMPLES_SOURCE}/windows/ole COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR} PATTERN ".svn" EXCLUDE)

   # oodialog samples
   install(DIRECTORY ${SAMPLES_SOURCE}/windows/oodialog COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR} PATTERN ".svn" EXCLUDE)
   install(FILES ${build_extensions_oodialog_dir}/oodialog.ico COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR}/oodialog)

   # sample shortcuts
   # NOTE:  because of multiple levels of escaping going on, the subdirectory "\" require FOUR backslashes
   install_rexx_shortcut(Samples "ooRexx Samples\\\\RexxCPS" rexxpaws "samples\\\\rexxcps.rex")
   install_rexx_shortcut(Samples "ooRexx Samples\\\\Query Date" rexxpaws "samples\\\\qdate.rex")
   install_rexx_shortcut(Samples "ooRexx Samples\\\\Display Window Tree" ooDialog "samples\\\\oodialog\\\\winsystem\\\\displayWindowTree.rex")
   install_rexx_shortcut(Samples "ooRexx Samples\\\\Windows Manager" ooDialog "samples\\\\oodialog\\\\winsystem\\\\usewmgr.rex")
   install_rexx_shortcut(Samples "ooRexx Samples\\\\ooDialog\\\\Combo Box Types" ooDialog "samples\\\\oodialog\\\\Controls\\\\ComboBox\\\\comboBoxTypes.rex")
   install_rexx_shortcut(Samples "ooRexx Samples\\\\ooDialog\\\\Custom Position Tool Tips" ooDialog "samples\\\\oodialog\\\\Controls\\\\ToolTip\\\\customPositionToolTip.rex")
   install_rexx_shortcut(Samples "ooRexx Samples\\\\ooDialog\\\\List-view Views" ooDialog "samples\\\\oodialog\\\\Controls\\\\ListView\\\\columnIcons.rex")
   install_rexx_shortcut(Samples "ooRexx Samples\\\\ooDialog\\\\Samples" ooDialog "samples\\\\sample.rex")
   install_component_shortcut(Samples "ooRexx Samples\\\\API\\\\Call ooRexx in a Console" "samples\\\\api\\\\callrxnt\\\\callrxnt.exe" "" "samples\\\\api\\\\callrxnt\\\\callrxnt.ico" "")
   install_component_shortcut(Samples "ooRexx Samples\\\\API\\\\Call ooRexx in a Window" "samples\\\\api\\\\callrxwn\\\\callrxwn.exe" "" "samples\\\\api\\\\callrxwn\\\\callrxwn.ico" "")
   install_component_shortcut(Samples "ooRexx Samples\\\\API\\\\Call ooRexx with Exits" "samples\\\\api\\\\rexxexit\\\\rexxexit.exe" "" "samples\\\\api\\\\rexxexit\\\\rexxexit.ico" "")

   # if we have a doc source directory, then add the Docs component to the installer and
   # install each of the doc files
   install(FILES ${DOC_SOURCE_DIR}/readme.pdf COMPONENT Docs DESTINATION ${INSTALL_DOC_DIR})
   install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/CHANGES COMPONENT Docs DESTINATION ${INSTALL_DOC_DIR} RENAME CHANGES.txt)
   install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/ReleaseNotes COMPONENT Docs DESTINATION ${INSTALL_DOC_DIR} RENAME ReleaseNotes.txt)
   install(FILES ${DOC_SOURCE_DIR}/rexxpg.pdf COMPONENT Docs DESTINATION ${INSTALL_DOC_DIR})
   install(FILES ${DOC_SOURCE_DIR}/rexxref.pdf COMPONENT Docs DESTINATION ${INSTALL_DOC_DIR})
   install(FILES ${DOC_SOURCE_DIR}/rxmath.pdf COMPONENT Docs DESTINATION ${INSTALL_DOC_DIR})
   install(FILES ${DOC_SOURCE_DIR}/rxsock.pdf COMPONENT Docs DESTINATION ${INSTALL_DOC_DIR})
   install(FILES ${DOC_SOURCE_DIR}/rxftp.pdf COMPONENT Docs DESTINATION ${INSTALL_DOC_DIR})
   install(FILES ${DOC_SOURCE_DIR}/oodialog.pdf COMPONENT Docs DESTINATION ${INSTALL_DOC_DIR})
   install(FILES ${build_extensions_oodialog_dir}/ooDialog_ReleaseNotes.txt COMPONENT Docs DESTINATION ${INSTALL_DOC_DIR})
   install(FILES ${DOC_SOURCE_DIR}/oodguide.pdf COMPONENT Docs DESTINATION ${INSTALL_DOC_DIR})
   install(FILES ${DOC_SOURCE_DIR}/rexxextensions.pdf COMPONENT Docs DESTINATION ${INSTALL_DOC_DIR})
   install(FILES ${DOC_SOURCE_DIR}/winextensions.pdf COMPONENT Docs DESTINATION ${INSTALL_DOC_DIR})
   install(FILES ${SAMPLES_SOURCE}/windows/oodialog/ooRexxTry/doc/ooRexxTry.pdf COMPONENT Docs DESTINATION ${INSTALL_DOC_DIR})

   install_doc_shortcut("ooRexx README" "readme.pdf")
   install_doc_shortcut("ooRexx CHANGES" "CHANGES.txt")
   install_doc_shortcut("ooRexx Release Notes" "ReleaseNotes.txt")
   install_doc_shortcut("ooRexx Reference" "rexxref.pdf")
   install_doc_shortcut("ooRexx Programming Guide" "rexxpg.pdf")
   install_doc_shortcut("ooRexx Mathematical Functions Reference" "rxmath.pdf")
   install_doc_shortcut("ooRexx rxFTP Class Reference" "rxftp.pdf")
   install_doc_shortcut("ooDialog Reference" "oodialog.pdf")
   install_doc_shortcut("ooDialog Release Notes" "ooDialog_ReleaseNotes.txt")
   install_doc_shortcut("ooDialog User Guide" "oodguide.pdf")
   install_doc_shortcut("ooRexx Extensions Reference" "rexxextensions.pdf")
   install_doc_shortcut("ooRexx Windows Reference" "winextensions.pdf")
   install_doc_shortcut("ooRexxTry Reference" "ooRexxTry.pdf")

   # build the NSIS shortcut creation/deletion macros that are inserted into the template
   configure_file(${NSIS_INSTALLER_SOURCE_DIR}/Core_shortcut_manifest.nsh.in ${NSIS_INSTALLER_DIR}/Core_shortcut_manifest.nsh @ONLY)
   configure_file(${NSIS_INSTALLER_SOURCE_DIR}/Docs_shortcut_manifest.nsh.in ${NSIS_INSTALLER_DIR}/Docs_shortcut_manifest.nsh @ONLY)
   configure_file(${NSIS_INSTALLER_SOURCE_DIR}/DevLib_shortcut_manifest.nsh.in ${NSIS_INSTALLER_DIR}/DevLib_shortcut_manifest.nsh @ONLY)
   configure_file(${NSIS_INSTALLER_SOURCE_DIR}/Samples_shortcut_manifest.nsh.in ${NSIS_INSTALLER_DIR}/Samples_shortcut_manifest.nsh @ONLY)

   # Build an NSIS installer without using CPack
   find_program(NSIS_EXECUTABLE makensis DOC "NSIS Compiler")

   # only do this if we have NSIS installed
   if (NSIS_EXECUTABLE)
      # we're using verbatim for the command, so we need to get the
      # directories in native form first
      file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR} NATIVE_SOURCE_DIR)
      file(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR}/bin NATIVE_BINARY_DIR)
      file(TO_NATIVE_PATH ${DOC_SOURCE_DIR} NATIVE_DOC_DIR)

      # Name of the installer file we're going to create
      set(INSTALLER_FILE "ooRexx.${ORX_VERSION_NSIS}-${NSIS_CPU}.exe")
      set(INSTALLER_SCRIPT "ooRexx.${ORX_VERSION_NSIS}-${NSIS_CPU}.nsi")

      # create an installer file from the template
      message(STATUS "Building NSIS template ${INSTALLER_SCRIPT}")
      configure_file(${NSIS_INSTALLER_SOURCE_DIR}/cpack.nsi.template.in ${NSIS_INSTALLER_DIR}/${INSTALLER_SCRIPT} @ONLY)

   # handy macro for copying files from the source into the source directory.
   macro (copy_install_file name)
       ADD_CUSTOM_COMMAND(
          OUTPUT ${NSIS_INSTALLER_DIR}/${name}
          COMMAND ${CMAKE_COMMAND} -E copy ${NSIS_INSTALLER_SOURCE_DIR}/${name} ${NSIS_INSTALLER_DIR}/${name}
          DEPENDS ${NSIS_INSTALLER_SOURCE_DIR}/${name}
       )
   endmacro (copy_install_file)

      add_custom_target(nsis_installer
         DEPENDS ${DOC_SOURCE_DIR}/readme.pdf
                 ${DOC_SOURCE_DIR}/rexxpg.pdf
                 ${DOC_SOURCE_DIR}/rexxref.pdf
                 ${DOC_SOURCE_DIR}/rxmath.pdf
                 ${DOC_SOURCE_DIR}/rxsock.pdf
                 ${DOC_SOURCE_DIR}/rxftp.pdf
                 ${DOC_SOURCE_DIR}/oodialog.pdf
                 ${DOC_SOURCE_DIR}/oodguide.pdf
                 ${DOC_SOURCE_DIR}/rexxextensions.pdf
                 ${DOC_SOURCE_DIR}/winextensions.pdf
                 ${build_platform_dir}/install/cpack.nsi
         VERBATIM
         COMMAND ${NSIS_EXECUTABLE} /DVERSION=${ORX_VERSION_NSIS} /DSRCDIR=${NATIVE_SOURCE_DIR} /DBINDIR=${NATIVE_BINARY_DIR} /DDOCDIR=${NATIVE_DOC_DIR} /DCPU=${NSIS_CPU} cpack.nsi
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${build_platform_dir}/install
         COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${build_platform_dir}/install/${INSTALLER_FILE} ${CMAKE_CURRENT_BINARY_DIR}/${INSTALLER_FILE}
         COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_SOURCE_DIR}/${build_platform_dir}/install/${INSTALLER_FILE}
         COMMENT "Building NSIS installer")

      # copy additional include files required by the makensis command
      copy_install_file(admin.nsh)
      copy_install_file(newpath.nsh)
      copy_install_file(WriteEnv.nsh)
      copy_install_file(orange.bmp)
      copy_install_file(orange-uninstall.bmp)

      # a target to make sure these get copied (not part of ALL, but the installer will depend on it
      add_custom_target(nsis_includes DEPENDS
         ${NSIS_INSTALLER_DIR}/admin.nsh
         ${NSIS_INSTALLER_DIR}/newpath.nsh
         ${NSIS_INSTALLER_DIR}/WriteEnv.nsh
         ${NSIS_INSTALLER_DIR}/orange.bmp
         ${NSIS_INSTALLER_DIR}/orange-uninstall.bmp)

      # Target for building an NSIS installation.  This will copy all of the install
      # files into a directory structure that mimics how.  We do this in 4 stages to install each of
      # the components into a different directory, since them might install files into common directories
      # at install time.
      add_custom_target(nsis_install_dirs
         DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/cmake_install.cmake
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
         COMMAND ${CMAKE_COMMAND} -DCMAKE_INSTALL_COMPONENT=Core -DCMAKE_INSTALL_PREFIX=${NSIS_FILE_DIR}/Core -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_install.cmake
         COMMAND ${CMAKE_COMMAND} -DCMAKE_INSTALL_COMPONENT=Docs -DCMAKE_INSTALL_PREFIX=${NSIS_FILE_DIR}/Docs -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_install.cmake
         COMMAND ${CMAKE_COMMAND} -DCMAKE_INSTALL_COMPONENT=DevLib -DCMAKE_INSTALL_PREFIX=${NSIS_FILE_DIR}/DevLib -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_install.cmake
         COMMAND ${CMAKE_COMMAND} -DCMAKE_INSTALL_COMPONENT=Samples -DCMAKE_INSTALL_PREFIX=${NSIS_FILE_DIR}/Samples -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_install.cmake
         COMMENT "Building NSIS file install structure")

      # Target for building the NSIS file manifests from the install information
      add_custom_target(nsis_file_manifests
         DEPENDS nsis_install_dirs
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
         COMMAND ${CMAKE_COMMAND} -DBINARY_DIR=${CMAKE_CURRENT_BINARY_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/${build_platform_dir}/install/BuildComponentManifests.cmake
         COMMENT "Building NSIS file install manifests")

      # and finally, build the installer manifests
      add_custom_target(nsis_template_installer
         DEPENDS nsis_file_manifests
                 nsis_install_dirs
                 nsis_includes
                 ${NSIS_INSTALLER_DIR}/${INSTALLER_SCRIPT}
         VERBATIM
         COMMAND ${NSIS_EXECUTABLE} ${INSTALLER_SCRIPT}
         WORKING_DIRECTORY ${NSIS_INSTALLER_DIR}
         COMMENT "Building NSIS installer from template in ${NSIS_INSTALLER_DIR}")

      # an extra to cleanup the installer directory to start fresh
      add_custom_target(remove_nsis_installer
          COMMAND ${CMAKE_COMMAND} -E remove_directory ${NSIS_INSTALLER_DIR}
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
          COMMENT "Removing NSIS installer build files")
   endif()
endif ()


#################### Unix Package Targets ##########################
#################### Unix Package Targets ##########################
#################### Unix Package Targets ##########################
#################### Unix Package Targets ##########################
#################### Unix Package Targets ##########################

# Build an RPM
if (BUILD_RPM)
   # Set up the desktop menu
   SET(ORX_DESKTOP_DIR     ${CMAKE_INSTALL_PREFIX}/share/applications)
   SET(ORX_PIXMAPS_DIR     ${CMAKE_INSTALL_PREFIX}/share/icons)
   configure_file(${ORX_PLATFORM_DIR}/menu/rexxtry.desktop.in ${PROJECT_BINARY_DIR}/rexxtry.desktop)
   INSTALL (FILES ${PROJECT_BINARY_DIR}/rexxtry.desktop DESTINATION ${ORX_DESKTOP_DIR})
   #INSTALL (FILES ${ORX_PLATFORM_DIR}/menu/rexxtry.xml DESTINATION ${ORX_DESKTOP_DIR})
   # Copy the ooRexx pixmap
   INSTALL (FILES ${ORX_PLATFORM_DIR}/oorexx.png DESTINATION ${ORX_PIXMAPS_DIR})
   # try to set up the menu system
   find_program(XDG-MIME_EXECUTABLE xdg-mime)
   find_program(XDG-DESKTOP-MENU_EXECUTABLE xdg-desktop-menu)
   INSTALL(CODE "
     #execute_process(COMMAND ${XDG-MIME_EXECUTABLE} install --novendor ${ORX_DESKTOP_DIR}/rexxtry.xml)
     execute_process(COMMAND ${XDG-DESKTOP-MENU_EXECUTABLE} install --novendor ${ORX_DESKTOP_DIR}/rexxtry.desktop)
     execute_process(COMMAND ${XDG-MIME_EXECUTABLE} default ${ORX_DESKTOP_DIR}/rexxtry.desktop application/x-ooRexx-item)
     ")
   # This pevents the rpm from generating these subdirectories in the install.
   set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION
      ${CMAKE_INSTALL_PREFIX}
      ${CMAKE_INSTALL_PREFIX}/bin
      ${CMAKE_INSTALL_PREFIX}/lib
      ${CMAKE_INSTALL_PREFIX}/lib64
      ${CMAKE_INSTALL_PREFIX}/share
      ${CMAKE_INSTALL_PREFIX}/share/man
      ${CMAKE_INSTALL_PREFIX}/share/man/man1
      ${CMAKE_INSTALL_PREFIX}/share/applications
      ${CMAKE_INSTALL_PREFIX}/share/icons)
   set(CPACK_GENERATOR RPM)
   set(CPACK_RPM_PACKAGE_NAME ${CPACK_PACKAGE_NAME})
   if (DEFINED PROD)
      set(CPACK_PACKAGE_RELEASE 1)
   else ()
      set(CPACK_PACKAGE_RELEASE ${ORX_BLD_LVL})
   endif ()
   set(CPACK_RPM_PACKAGE_LICENSE "CPL 1.0")
   set(CPACK_RPM_PACKAGE_URL "http://www.oorexx.org/")
   set(CPACK_PACKAGE_CONTACT "wdashley@sourceforge.net")
   set(CPACK_PACKAGING_INSTALL_PREXIX "${CMAKE_INSTALL_PREFIX}")
   configure_file(${CMAKE_CURRENT_SOURCE_DIR}/platform/unix/rpm_post.in ${CMAKE_CURRENT_SOURCE_DIR}/platform/unix/rpm_post)
   set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE ${CMAKE_SOURCE_DIR}/platform/unix/rpm_post)
   set(CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE ${CMAKE_SOURCE_DIR}/platform/unix/rpm_preun)
   set(CPACK_RPM_PACKAGE_REQUIRES "csh")
   if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "s390x")
      set(orx_install_arch "s390x")
   elseif (${INSTALL_LIB_DIR} STREQUAL ${CMAKE_INSTALL_PREFIX}/lib64)
      set(orx_install_arch "x86_64")
   else ()
      set(orx_install_arch "i386")
   endif ()
   set(CPACK_RPM_PACKAGE_ARCHITECTURE ${orx_install_arch})
   if (DEFINED OS_DIST)
      set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${ORX_MAJOR}.${ORX_MINOR}.${ORX_MOD_LVL}-${CPACK_PACKAGE_RELEASE}.${orx_install_arch}.${OS_DIST}")
   else ()
      set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${ORX_MAJOR}.${ORX_MINOR}.${ORX_MOD_LVL}-${CPACK_PACKAGE_RELEASE}.${orx_install_arch}")
   endif ()
   set(CPACK_RPM_PACKAGE_DESCRIPTION
"Open Object Rexx is an object-oriented scripting language. The language
is designed for both beginners and experienced Rexx programmers. It is
easy to learn and use, and provides an excellent vehicle to enter the
world of object-oriented programming without much effort.

It extends the procedural way of Rexx programming with object-oriented
features that allow you to gradually change your programming style as
you learn more about objects.

For more information on ooRexx, visit http://www.oorexx.org/
For more information on Rexx, visit http://www.rexxla.org/")
endif ()

# Build a DEB
if (BUILD_DEB)
   # Set up the desktop menu
   SET(ORX_DESKTOP_DIR     ${CMAKE_INSTALL_PREFIX}/share/applications)
   SET(ORX_PIXMAPS_DIR     ${CMAKE_INSTALL_PREFIX}/share/icons)
   configure_file(${ORX_PLATFORM_DIR}/menu/rexxtry.desktop.in ${PROJECT_BINARY_DIR}/rexxtry.desktop)
   INSTALL (FILES ${PROJECT_BINARY_DIR}/rexxtry.desktop DESTINATION ${ORX_DESKTOP_DIR})
   #INSTALL (FILES ${ORX_PLATFORM_DIR}/menu/rexxtry.xml DESTINATION ${ORX_DESKTOP_DIR})
   # Copy the ooRexx pixmap
   INSTALL (FILES ${ORX_PLATFORM_DIR}/oorexx.png DESTINATION ${ORX_PIXMAPS_DIR})
   # try to set up the menu system
   find_program(XDG-MIME_EXECUTABLE xdg-mime)
   find_program(XDG-DESKTOP-MENU_EXECUTABLE xdg-desktop-menu)
   INSTALL(CODE "
     #execute_process(COMMAND ${XDG-MIME_EXECUTABLE} install --novendor ${ORX_DESKTOP_DIR}/rexxtry.xml)
     execute_process(COMMAND ${XDG-DESKTOP-MENU_EXECUTABLE} install --novendor ${ORX_DESKTOP_DIR}/rexxtry.desktop)
     execute_process(COMMAND ${XDG-MIME_EXECUTABLE} default ${ORX_DESKTOP_DIR}/rexxtry.desktop application/x-ooRexx-item)
     ")
   set(CPACK_GENERATOR DEB)
   # The package name MUST be in lower case.
   set(CPACK_DEBIAN_PACKAGE_NAME oorexx)
   if (DEFINED PROD)
      set(CPACK_PACKAGE_RELEASE 1)
   else ()
      set(CPACK_PACKAGE_RELEASE ${ORX_BLD_LVL})
   endif ()
   set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
   set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "http://www.oorexx.org/")
   set(CPACK_PACKAGE_CONTACT "wdashley@sourceforge.net")
   set(CPACK_DEBIAN_PACKAGE_MAINTAINER "David Ashley <wdashley@sourceforge.net>")
   set(CPACK_PACKAGING_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
   configure_file(${CMAKE_CURRENT_SOURCE_DIR}/platform/unix/debian/cmake/postinst.in ${CMAKE_CURRENT_SOURCE_DIR}/platform/unix/debian/cmake/postinst)
   set(CPACK_DEBIAN_PACKAGE_DEPENDS "csh")
   # we need to copy these files so we can change the permissions
   file(COPY ${CMAKE_SOURCE_DIR}/platform/unix/debian/cmake/postinst
           DESTINATION ${CMAKE_BINARY_DIR}
           FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ
           GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
   file(COPY ${CMAKE_SOURCE_DIR}/platform/unix/debian/cmake/prerm
           DESTINATION ${CMAKE_BINARY_DIR}
           FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ
           GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
   set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
           "${CMAKE_BINARY_DIR}/postinst;${CMAKE_BINARY_DIR}/prerm;")
   if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "s390x")
      set(orx_install_arch "s390x")
   elseif (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
      set(orx_install_arch "x86_64")
   else ()
      set(orx_install_arch "i386")
   endif ()
   if (DEFINED OS_DIST)
      set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${ORX_MAJOR}.${ORX_MINOR}.${ORX_MOD_LVL}-${CPACK_PACKAGE_RELEASE}.${orx_install_arch}.${OS_DIST}")
   else ()
      set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${ORX_MAJOR}.${ORX_MINOR}.${ORX_MOD_LVL}-${CPACK_PACKAGE_RELEASE}.${orx_install_arch}")
   endif ()
   # Be sure to do this AFTER you set the CPACK_PACKAGE_FILE_NAME
   configure_file(${CMAKE_CURRENT_SOURCE_DIR}/platform/unix/debian/cmake/fixup_deb.sh.in ${CMAKE_BINARY_DIR}/fixup_deb.sh)
   # The DEB generator does not support a multi-line description.
   set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Open Object Rexx is an object-oriented scripting language.")
endif ()

if (BUILD_PKG)
   set(CPACK_GENERATOR PKG)
   #set (CPACK_PREFLIGHT_SCRIPT xxx)
   #set (CPACK_POSTFLIGHT_SCRIPT xxx)
   #set (CPACK_OSX_PACKAGE_VERSION "10.10")
   #set (CPACK_BUNDLE_PLIST xxx)
   #set (CPACK_BUNDLE_STARTUP_COMMABD xxx)

   if (DEFINED PROD)
      set(CPACK_PACKAGE_RELEASE 1)
   else ()
      set(CPACK_PACKAGE_RELEASE ${ORX_BLD_LVL})
   endif ()
   set(orx_install_arch "x86_64")
   if (DEFINED OS_DIST)
      set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${ORX_MAJOR}.${ORX_MINOR}.${ORX_MOD_LVL}-${CPACK_PACKAGE_RELEASE}.${orx_install_arch}.${OS_DIST}")
   else ()
      set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${ORX_MAJOR}.${ORX_MINOR}.${ORX_MOD_LVL}-${CPACK_PACKAGE_RELEASE}.${orx_install_arch}")
   endif ()
   #set (CPACK_BUNDLE_NAME ${CPACK_PACKAGE_FILE_NAME})

   set (CPACK_PACKAGE_ICON ${ORX_PLATFORM_DIR}/oorexx.png)
   #set (CPACK_BUNDLE_ICON ${CPACK_PACKAGE_ICON})

   set(CPACK_PACKAGE_CONTACT "wdashley@sourceforge.net")
   set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
"Open Object Rexx is an object-oriented scripting language. The language
is designed for both beginners and experienced Rexx programmers. It is
easy to learn and use, and provides an excellent vehicle to enter the
world of object-oriented programming without much effort.

It extends the procedural way of Rexx programming with object-oriented
features that allow you to gradually change your programming style as
you learn more about objects.

For more information on ooRexx, visit http://www.oorexx.org/
For more information on Rexx, visit http://www.rexxla.org/")
endif ()






# Do not move the following statement!! It must be the last statement in this file.
include(CPack)
