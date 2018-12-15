#/*----------------------------------------------------------------------------*/
#/*                                                                            */
#/* Copyright (c) 2014-2018 Rexx Language Association. All rights reserved.    */
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

#/*----------------------------------------------------------------------------*/
#/* Project settings                                                           */
#/*----------------------------------------------------------------------------*/
project (oorexxtest)

# handy macro for generating test executable targets
macro (generate_test_library target)
   if (WIN32 AND ${ARGC} GREATER 1)
     set (PLATFORM_EXTRA  ${PROJECT_SOURCE_DIR}/${target}.def)
   endif ()
   add_library(${target} SHARED
      ${PLATFORM_EXTRA}
      ${PROJECT_SOURCE_DIR}/${target})
   # Include file definition
   target_include_directories(${target} PUBLIC
               ${build_api_dir}
               ${build_api_platform_dir})
   # Extra link library definitions
   target_link_libraries(${target} rexx rexxapi)
endmacro (generate_test_library)

generate_test_library(orxfunction)
generate_test_library(orxmethod)
generate_test_library(orxclassic)
generate_test_library(orxclassic1 def)


# this library is linked to other test libraries
if (WIN32)
  set (PLATFORM_EXTRA  ${PROJECT_SOURCE_DIR}/orxexits.def)
endif ()
add_library(orxexits SHARED
   ${PLATFORM_EXTRA}
   ${PROJECT_SOURCE_DIR}/orxclassicexits
   ${PROJECT_SOURCE_DIR}/orxinstance)
# Include file definition
target_include_directories(orxexits PUBLIC
            ${build_api_dir}
            ${build_api_platform_dir})
# Extra link library definitions
target_link_libraries(orxexits rexx rexxapi)

# this library has the orxexits dependency
add_library(orxinvocation SHARED
   ${PROJECT_SOURCE_DIR}/orxinvocation)
# Include file definition
target_include_directories(orxinvocation PUBLIC
            ${build_api_dir}
            ${build_api_platform_dir})
# Extra link library definitions
target_link_libraries(orxinvocation orxexits rexx rexxapi)

add_executable(rexxinstance
   ${PROJECT_SOURCE_DIR}/rexxinstance.cpp)
# Include file definition
target_include_directories(rexxinstance PUBLIC
            ${build_api_dir}
            ${build_api_platform_dir})
# Extra link library definitions
target_link_libraries(rexxinstance orxexits rexx rexxapi)



