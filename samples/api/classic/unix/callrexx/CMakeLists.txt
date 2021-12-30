#/*----------------------------------------------------------------------------*/
#/*                                                                            */
#/* Copyright (c) 2014-2021 Rexx Language Association. All rights reserved.    */
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
project (callrexx)

# set variables used in here
set (DEST ${INSTALL_SAMPLES_DIR}/api/classic/callrexx)
# add GROUP_WRITE right to default permissions
set (PERM_EXECUTE PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE )
set (PERM_RW      PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ  )

# message (STATUS PERM_EXECUTE=${PERM_EXECUTE} )
# message (STATUS PROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR} )

#################### callrexx1 (executable) #########################
# additional source files required by specific platforms
# Sources for callrexx1
add_executable(callrexx1 ${PROJECT_SOURCE_DIR}/callrexx1.cpp)
# Include file definition
target_include_directories(callrexx1 PUBLIC
            ${build_api_dir}
            ${build_api_platform_dir})
# Extra link library definitions
target_link_libraries(callrexx1 rexx rexxapi)
install(TARGETS callrexx1 RUNTIME DESTINATION ${DEST} ${PERM_EXECUTE})
install(FILES callrexx1.cpp Makefile.unix COMPONENT Samples DESTINATION ${DEST} ${PERM_RW} )


#################### callrexx2 (executable) #########################
# additional source files required by specific platforms
#set (platform_callrexx2_libs ${ORX_SYSLIB_DL} ${ORX_SYSLIB_PTHREAD})
# Sources for callrexx2
add_executable(callrexx2 ${PROJECT_SOURCE_DIR}/callrexx2.c)
# Include file definition
target_include_directories(callrexx2 PUBLIC
            ${build_api_dir}
	    ${build_api_platform_dir})
# Extra link library definitions
target_link_libraries(callrexx2 rexx rexxapi)
install(TARGETS callrexx2 RUNTIME DESTINATION ${DEST} ${PERM_EXECUTE})
install(FILES callrexx2.c COMPONENT Samples DESTINATION ${DEST} ${PERM_RW})


#################### Rexx scripts invoked by the executables #########################
# o.k. install(PROGRAMS del_macro.rex example.rex load_macro.rex macros.rex startrx1.rex startrx2.rex startrx3.rex
#               COMPONENT Samples DESTINATION ${DEST})

# install(DIRECTORY ${PROJECT_SOURCE_DIR} COMPONENT Samples DESTINATION ${DEST} FILES_MATCHING PATTERN "*.rex" ${PERM} )
# install(DIRECTORY ${PROJECT_SOURCE_DIR} COMPONENT Samples DESTINATION ${DEST} FILES_MATCHING PATTERN "*.rex" ${PERM} )


install(PROGRAMS del_macro.rex example.rex load_macro.rex macros.rex startrx1.rex startrx2.rex startrx3.rex
	COMPONENT Samples DESTINATION ${DEST} ${PERM_EXECUTE})

