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

#/*----------------------------------------------------------------------------*/
#/* Project settings                                                           */
#/*----------------------------------------------------------------------------*/
project (wpipe3)

#################### wpipe3 (executable) #########################
# Sources for wpipe3
add_library(rexxapi3 SHARED ${PROJECT_SOURCE_DIR}/rexxapi3.c
                          ${PROJECT_SOURCE_DIR}/rexxapi3.def)
# Include file definition
target_include_directories(rexxapi3 PUBLIC
            ${build_api_dir}
            ${build_api_platform_dir})
# Extra link library definitions
target_link_libraries(rexxapi3 rexx rexxapi)
install(TARGETS rexxapi3 RUNTIME COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR}/api/wpipe3)
install(FILES ${PROJECT_SOURCE_DIR}/rexxapi3.c rexxapi3.def rexxapi3.mak COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR}/api/wpipe3)
install(PROGRAMS ${PROJECT_SOURCE_DIR}/apitest3.rex COMPONENT Samples DESTINATION ${INSTALL_SAMPLES_DIR}/api/wpipe3)
