@REM /*----------------------------------------------------------------------------*/
@REM /*                                                                            */
@REM /* Copyright (c) 2014 Rexx Language Association. All rights reserved.         */
@REM /*                                                                            */
@REM /* This program and the accompanying materials are made available under       */
@REM /* the terms of the Common Public License v1.0 which accompanies this         */
@REM /* distribution. A copy is also available at the following address:           */
@REM /* http://www.oorexx.org/license.html                                         */
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
REM  This  build program needs to be run any time the file interpreter/messages/rexxmsg.xml
REM  is updated.  This must be run from the root source directory, and will update
REM  the different message files that are generated from rexxmsg.xml directly into
REM  the source tree so they can be checked into SVN.
REM

@ECHO Generating interpreter\platform\windows\winmsgtb.rc
xalan -o interpreter\platform\windows\winmsgtb.rc interpreter\messages\rexxmsg.xml interpreter\platform\windows\WinMessageResource.xsl
@ECHO Generating interpreter\messages\DocErrorMessages.sgml
xalan -o interpreter\messages\DocErrorMessages.sgml interpreter\messages\rexxmsg.xml interpreter\messages\DocBookErrors.xsl
@ECHO Generating interpreter\messages\errnums.xml
xalan -o interpreter\messages\errnums.xml interpreter\messages\rexxmsg.xml interpreter\messages\errnums.xsl
@ECHO Generating interpreter\messages\errnumsrxqueue.xml
xalan -o interpreter\messages\errnumsrxqueue.xml interpreter\messages\rexxmsg.xml interpreter\messages\errnumsrxqueue.xsl
@ECHO Generating interpreter\messages\errnumssubcom.xml
xalan -o interpreter\messages\errnumssubcom.xml interpreter\messages\rexxmsg.xml interpreter\messages\errnumssubcom.xsl
@ECHO Generating interpreter\messages\errnumsrexxc.xml
xalan -o interpreter\messages\errnumsrexxc.xml interpreter\messages\rexxmsg.xml interpreter\messages\errnumsrexxc.xsl
@ECHO Generating interpreter\messages\RexxErrorCodes.h
xalan -o interpreter\messages\RexxErrorCodes.h interpreter\messages\rexxmsg.xml interpreter\messages\RexxErrorCodes.xsl
@ECHO Generating interpreter\messages\RexxMessageNumbers.h
xalan -o interpreter\messages\RexxMessageNumbers.h interpreter\messages\rexxmsg.xml interpreter\messages\RexxMessageNumbers.xsl
@ECHO Generating interpreter\messages\RexxMessageTable.h
xalan -o interpreter\messages\RexxMessageTable.h interpreter\messages\rexxmsg.xml interpreter\messages\RexxMessageTable.xsl
@ECHO Generating api\oorexxerrors.h
xalan -o api\oorexxerrors.h interpreter\messages\rexxmsg.xml interpreter\messages\ApiErrorCodes.xsl
