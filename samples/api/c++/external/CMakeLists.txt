#/*----------------------------------------------------------------------------*/
#/*                                                                            */
#/* Copyright (c) 2021 Rexx Language Association. All rights reserved.         */
#/*                                                                            */
#/* This program and the accompanying materials are made available under       */
#/* the terms of the Common Public License v1.0 which accompanies this         */
#/* distribution. A copy is also available at the following address:           */
#/* https://www.oorexx.org/license.html                                        */
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
if (APPLE)
# apple build with lower cmake version have an @rpath problem
    cmake_minimum_required (VERSION 3.12)
else()
#for other platforms
    cmake_minimum_required (VERSION 2.8.12)
endif()
cmake_policy(VERSION 2.8...3.3)

#/*----------------------------------------------------------------------------*/
#/* Project settings                                                           */
#/*----------------------------------------------------------------------------*/
project (external-sample)

# installation target for all samples
set (DEST ${INSTALL_SAMPLES_DIR}/api/c++/external)

# allow group write permissions as well
set (PERM PERMISSIONS OWNER_READ OWNER_WRITE
                      GROUP_READ GROUP_WRITE
                      WORLD_READ             )
# add execute bit
set (PERM_EXE ${PERM} OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE)

install(FILES readme.txt Makefile.unix Makefile.windows
                                          COMPONENT Samples DESTINATION ${DEST} ${PERM})

#################### external_routines (library) #########################
# Sources for external_routines sample
add_library(external_routines SHARED external_routines.cpp)
# Include file definition
target_include_directories(external_routines PUBLIC
            ${build_api_dir}
            ${build_api_platform_dir})
target_link_libraries(external_routines rexxapi)

if (WIN32)
        install(TARGETS external_routines RUNTIME COMPONENT Samples DESTINATION ${DEST})
else()
        install(TARGETS external_routines LIBRARY COMPONENT Samples DESTINATION ${DEST} ${PERM_EXE} )
endif()
install(FILES   external_routines.cpp     COMPONENT Samples DESTINATION ${DEST} ${PERM})
install(PROGRAMS useExternalRoutines.rex useExternalRoutines2.rex
                                          COMPONENT Samples DESTINATION ${DEST} ${PERM_EXE})


#################### external_methods (library) #########################
# Sources for external_methods sample
add_library(external_methods SHARED external_methods.cpp)
# Include file definition
target_include_directories(external_methods PUBLIC
            ${build_api_dir}
            ${build_api_platform_dir})
target_link_libraries(external_methods rexxapi)

if (WIN32)
        install(TARGETS external_methods RUNTIME COMPONENT Samples DESTINATION ${DEST})
else()
        install(TARGETS external_methods LIBRARY COMPONENT Samples DESTINATION ${DEST} ${PERM_EXE})
endif()
install(FILES   external_methods.cpp     COMPONENT Samples DESTINATION ${DEST} ${PERM})
install(PROGRAMS useExternalMethods.rex  COMPONENT Samples DESTINATION ${DEST} ${PERM_EXE})


#################### external_methods_routines (library) #########################
# Sources for external_methods_routines sample
add_library(external_methods_routines SHARED external_methods_routines.cpp)
# Include file definition
target_include_directories(external_methods_routines PUBLIC
            ${build_api_dir}
            ${build_api_platform_dir})
target_link_libraries(external_methods_routines rexxapi)

if (WIN32)
        install(TARGETS external_methods_routines RUNTIME COMPONENT Samples DESTINATION ${DEST})
else()
        install(TARGETS external_methods_routines LIBRARY COMPONENT Samples DESTINATION ${DEST} ${PERM_EXE} )
endif()
install(FILES   external_methods_routines.cpp     COMPONENT Samples DESTINATION ${DEST} ${PERM})
install(PROGRAMS useExternalMethodsRoutines.rex   COMPONENT Samples DESTINATION ${DEST} ${PERM_EXE})


