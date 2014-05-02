@REM /*----------------------------------------------------------------------------*/
@REM /*                                                                            */
@REM /* Copyright (c) 2014 Rexx Language Association. All rights reserved.         */
@REM /*                                                                            */
@REM /* This program and the accompanying materials are made available under       */
@REM /* the terms of the Common Public License v1.0 which accompanies this         */
@REM /* distribution. A copy is also available at the following address:           */
@REM /* http://www.oorexx.org/license.html                          */
@REM /*                                                                            */
@REM /* Redistribution and use in source and binary forms, with or                 */
@REM /* without modification, are permitted provided that the following            */
@REM /* conditions are met:                                                        */
@REM /*                                                                            */
@REM /* Redistributions of source code must retain the above copyright             */
@REM /* notice, this list of conditions and the following disclaimer.              */
@REM /* Redistributions in binary form must reproduce the above copyright          */
@REM /* notice, this list of conditions and the following disclaimer in            */
@REM /* the documentation and/or other materials provided with the distribution.   */
@REM /*                                                                            */
@REM /* Neither the name of Rexx Language Association nor the names                */
@REM /* of its contributors may be used to endorse or promote products             */
@REM /* derived from this software without specific prior written permission.      */
@REM /*                                                                            */
@REM /* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS        */
@REM /* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT          */
@REM /* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS          */
@REM /* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   */
@REM /* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,      */
@REM /* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
@REM /* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,        */
@REM /* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY     */
@REM /* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING    */
@REM /* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         */
@REM /* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               */
@REM /*                                                                            */
@REM /*----------------------------------------------------------------------------*/
@ECHO Off
REM  This  build program needs to be run any time the file interpreter/behaviour/PrimitiveClasses.xml
REM  is updated.  This must be run from the root build directory, and will update
REM  the different header files that are generated from PrimitiveClasses.xml directly into
REM  the source tree so they can be checked into SVN.
REM

@ECHO Generating interpreter\behaviour\PrimitiveBehaviourNames.h
xalan -o interpreter\behaviour\PrimitiveBehaviourNames.h interpreter\behaviour\PrimitiveClasses.xml interpreter\behaviour\PrimitiveBehaviourNames.xsl
@ECHO Generating interpreter\behaviour\PrimitiveBehaviours.cpp
xalan -o interpreter\behaviour\PrimitiveBehaviours.cpp interpreter\behaviour\PrimitiveClasses.xml interpreter\behaviour\PrimitiveBehaviours.xsl
@ECHO Generating interpreter\behaviour\VirtualFunctionTable.cpp
xalan -o interpreter\behaviour\VirtualFunctionTable.cpp interpreter\behaviour\PrimitiveClasses.xml interpreter\behaviour\VirtualFunctionTable.xsl
@ECHO Generating interpreter\behaviour\ClassTypeCodes.h
xalan -o interpreter\behaviour\ClassTypeCodes.h interpreter\behaviour\PrimitiveClasses.xml interpreter\behaviour\ClassTypeCodes.xsl
