/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2014-2014 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                                         */
/*                                                                            */
/* Redistribution and use in source and binary forms, with or                 */
/* without modification, are permitted provided that the following            */
/* conditions are met:                                                        */
/*                                                                            */
/* Redistributions of source code must retain the above copyright             */
/* notice, this list of conditions and the following disclaimer.              */
/* Redistributions in binary form must reproduce the above copyright          */
/* notice, this list of conditions and the following disclaimer in            */
/* the documentation and/or other materials provided with the distribution.   */
/*                                                                            */
/* Neither the name of Rexx Language Association nor the names                */
/* of its contributors may be used to endorse or promote products             */
/* derived from this software without specific prior written permission.      */
/*                                                                            */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS        */
/* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT          */
/* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS          */
/* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   */
/* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,      */
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,        */
/* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY     */
/* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING    */
/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         */
/* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               */
/*                                                                            */
/*----------------------------------------------------------------------------*/

::requires 'ooshapes' LIBRARY

::class 'Rect' public
::method version class external "LIBRARY ooshapes ooShapes_version_cls"
::method init external "LIBRARY ooshapes rect_init"
::attribute left get external "LIBRARY ooshapes rect_left"
::attribute left set external "LIBRARY ooshapes rect_setLeft"
::attribute top get external "LIBRARY ooshapes rect_top"
::attribute top set external "LIBRARY ooshapes rect_setTop"
::attribute right get external "LIBRARY ooshapes rect_right"
::attribute right set external "LIBRARY ooshapes rect_setRight"
::attribute bottom get external "LIBRARY ooshapes rect_bottom"
::attribute bottom set external "LIBRARY ooshapes rect_setBottom"
::method copy external "LIBRARY ooshapes rect_copy"
::method string external "LIBRARY ooshapes rect_string"
::method print external "LIBRARY ooshapes rect_print"

::class 'Point' public
::method version class external "LIBRARY ooshapes ooShapes_version_cls"
::method init external "LIBRARY ooshapes point_init"
::attribute x get external "LIBRARY ooshapes point_x"
::attribute x set external "LIBRARY ooshapes point_setX"
::attribute y get external "LIBRARY ooshapes point_y"
::attribute y set external "LIBRARY ooshapes point_setY"
::method copy external "LIBRARY ooshapes point_copy"
::method '+' external "LIBRARY ooshapes point_add"
::method '-' external "LIBRARY ooshapes point_subtract"
::method incr external "LIBRARY ooshapes point_incr"
::method decr external "LIBRARY ooshapes point_decr"
::method inRect external "LIBRARY ooshapes point_inRect"
::method string external "LIBRARY ooshapes point_string"
::method print external "LIBRARY ooshapes point_print"

::class 'Size' public
::method version class external "LIBRARY ooshapes ooShapes_version_cls"
::method init external "LIBRARY ooshapes size_init"
::attribute width get external "LIBRARY ooshapes size_cx"
::attribute width set external "LIBRARY ooshapes size_setCX"
::attribute height get external "LIBRARY ooshapes size_cy"
::attribute height set external "LIBRARY ooshapes size_setCY"
::method string external "LIBRARY ooshapes size_string"
::method print external "LIBRARY ooshapes size_print"
::method '=' external "LIBRARY ooshapes size_compare"
::method '==' external "LIBRARY ooshapes size_compare"
::method '\=' external "LIBRARY ooshapes size_compare"
::method '\==' external "LIBRARY ooshapes size_compare"
::method '<' external "LIBRARY ooshapes size_compare"
::method '<<' external "LIBRARY ooshapes size_compare"
::method '<=' external "LIBRARY ooshapes size_compare"
::method '<<=' external "LIBRARY ooshapes size_compare"
::method '>' external "LIBRARY ooshapes size_compare"
::method '>>' external "LIBRARY ooshapes size_compare"
::method '>=' external "LIBRARY ooshapes size_compare"
::method '>>=' external "LIBRARY ooshapes size_compare"
::method equateTo external "LIBRARY ooshapes size_equateTo"

