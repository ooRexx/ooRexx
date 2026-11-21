/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2024-2026 Josep Maria Blasco. All rights reserved.           */
/* Copyright (c) 2026 Rexx Language Association. All rights reserved.         */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                                        */
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
/******************************************************************************/
/*                                                                            */
/* Colors.cls - Set of HTML named colors                                      */
/* =====================================                                      */
/*                                                                            */
/* This program is part of the Rexx Parser package                            */
/* [See https://rexx.epbcn.com/rexx-parser/]                                  */
/*                                                                            */
/* Version history:                                                           */
/*                                                                            */
/* Date     Version Details                                                   */
/* -------- ------- --------------------------------------------------------- */
/* 20250105    0.1f First public release                                      */
/* 20250328    0.2  Main dir is now rexx-parser instead of rexx[.]parser      */
/* 20261121         Common Public License 1.0, and copyright also of the Rexx */
/*                  Language Association, as the rest of ooRexx (RFE #878)    */
/*                                                                            */
/******************************************************************************/

-- See https://www.w3.org/TR/css-color-4/#named-colors

If .environment~HTML.Color~isA(.Stem) Then Exit

Color. = ""

Color.['aliceblue'           ] = 'f0f8ff'
Color.['antiquewhite'        ] = 'faebd7'
Color.['aqua'                ] = '00ffff'
Color.['aquamarine'          ] = '7fffd4'
Color.['azure'               ] = 'f0ffff'
Color.['beige'               ] = 'f5f5dc'
Color.['bisque'              ] = 'ffe4c4'
Color.['black'               ] = '000000'
Color.['blanchedalmond'      ] = 'ffebcd'
Color.['blue'                ] = '0000ff'
Color.['blueviolet'          ] = '8a2be2'
Color.['brown'               ] = 'a52a2a'
Color.['burlywood'           ] = 'deb887'
Color.['cadetblue'           ] = '5f9ea0'
Color.['chartreuse'          ] = '7fff00'
Color.['chocolate'           ] = 'd2691e'
Color.['coral'               ] = 'ff7f50'
Color.['cornflowerblue'      ] = '6495ed'
Color.['cornsilk'            ] = 'fff8dc'
Color.['crimson'             ] = 'dc143c'
Color.['cyan'                ] = '00ffff'
Color.['darkblue'            ] = '00008b'
Color.['darkcyan'            ] = '008b8b'
Color.['darkgoldenrod'       ] = 'b8860b'
Color.['darkgray'            ] = 'a9a9a9'
Color.['darkgreen'           ] = '006400'
Color.['darkgrey'            ] = 'a9a9a9'
Color.['darkkhaki'           ] = 'bdb76b'
Color.['darkmagenta'         ] = '8b008b'
Color.['darkolivegreen'      ] = '556b2f'
Color.['darkorange'          ] = 'ff8c00'
Color.['darkorchid'          ] = '9932cc'
Color.['darkred'             ] = '8b0000'
Color.['darksalmon'          ] = 'e9967a'
Color.['darkseagreen'        ] = '8fbc8f'
Color.['darkslateblue'       ] = '483d8b'
Color.['darkslategray'       ] = '2f4f4f'
Color.['darkslategrey'       ] = '2f4f4f'
Color.['darkturquoise'       ] = '00ced1'
Color.['darkviolet'          ] = '9400d3'
Color.['deeppink'            ] = 'ff1493'
Color.['deepskyblue'         ] = '00bfff'
Color.['dimgray'             ] = '696969'
Color.['dimgrey'             ] = '696969'
Color.['dodgerblue'          ] = '1e90ff'
Color.['firebrick'           ] = 'b22222'
Color.['floralwhite'         ] = 'fffaf0'
Color.['forestgreen'         ] = '228b22'
Color.['fuchsia'             ] = 'ff00ff'
Color.['gainsboro'           ] = 'dcdcdc'
Color.['ghostwhite'          ] = 'f8f8ff'
Color.['gold'                ] = 'ffd700'
Color.['goldenrod'           ] = 'daa520'
Color.['gray'                ] = '808080'
Color.['green'               ] = '008000'
Color.['greenyellow'         ] = 'adff2f'
Color.['grey'                ] = '808080'
Color.['honeydew'            ] = 'f0fff0'
Color.['hotpink'             ] = 'ff69b4'
Color.['indianred'           ] = 'cd5c5c'
Color.['indigo'              ] = '4b0082'
Color.['ivory'               ] = 'fffff0'
Color.['khaki'               ] = 'f0e68c'
Color.['lavender'            ] = 'e6e6fa'
Color.['lavenderblush'       ] = 'fff0f5'
Color.['lawngreen'           ] = '7cfc00'
Color.['lemonchiffon'        ] = 'fffacd'
Color.['lightblue'           ] = 'add8e6'
Color.['lightcoral'          ] = 'f08080'
Color.['lightcyan'           ] = 'e0ffff'
Color.['lightgoldenrodyellow'] = 'fafad2'
Color.['lightgray'           ] = 'd3d3d3'
Color.['lightgreen'          ] = '90ee90'
Color.['lightgrey'           ] = 'd3d3d3'
Color.['lightpink'           ] = 'ffb6c1'
Color.['lightsalmon'         ] = 'ffa07a'
Color.['lightseagreen'       ] = '20b2aa'
Color.['lightskyblue'        ] = '87cefa'
Color.['lightslategray'      ] = '778899'
Color.['lightslategrey'      ] = '778899'
Color.['lightsteelblue'      ] = 'b0c4de'
Color.['lightyellow'         ] = 'ffffe0'
Color.['lime'                ] = '00ff00'
Color.['limegreen'           ] = '32cd32'
Color.['linen'               ] = 'faf0e6'
Color.['magenta'             ] = 'ff00ff'
Color.['maroon'              ] = '800000'
Color.['mediumaquamarine'    ] = '66cdaa'
Color.['mediumblue'          ] = '0000cd'
Color.['mediumorchid'        ] = 'ba55d3'
Color.['mediumpurple'        ] = '9370db'
Color.['mediumseagreen'      ] = '3cb371'
Color.['mediumslateblue'     ] = '7b68ee'
Color.['mediumspringgreen'   ] = '00fa9a'
Color.['mediumturquoise'     ] = '48d1cc'
Color.['mediumvioletred'     ] = 'c71585'
Color.['midnightblue'        ] = '191970'
Color.['mintcream'           ] = 'f5fffa'
Color.['mistyrose'           ] = 'ffe4e1'
Color.['moccasin'            ] = 'ffe4b5'
Color.['navajowhite'         ] = 'ffdead'
Color.['navy'                ] = '000080'
Color.['oldlace'             ] = 'fdf5e6'
Color.['olive'               ] = '808000'
Color.['olivedrab'           ] = '6b8e23'
Color.['orange'              ] = 'ffa500'
Color.['orangered'           ] = 'ff4500'
Color.['orchid'              ] = 'da70d6'
Color.['palegoldenrod'       ] = 'eee8aa'
Color.['palegreen'           ] = '98fb98'
Color.['paleturquoise'       ] = 'afeeee'
Color.['palevioletred'       ] = 'db7093'
Color.['papayawhip'          ] = 'ffefd5'
Color.['peachpuff'           ] = 'ffdab9'
Color.['peru'                ] = 'cd853f'
Color.['pink'                ] = 'ffc0cb'
Color.['plum'                ] = 'dda0dd'
Color.['powderblue'          ] = 'b0e0e6'
Color.['purple'              ] = '800080'
Color.['rebeccapurple'       ] = '663399'
Color.['red'                 ] = 'ff0000'
Color.['rosybrown'           ] = 'bc8f8f'
Color.['royalblue'           ] = '4169e1'
Color.['saddlebrown'         ] = '8b4513'
Color.['salmon'              ] = 'fa8072'
Color.['sandybrown'          ] = 'f4a460'
Color.['seagreen'            ] = '2e8b57'
Color.['seashell'            ] = 'fff5ee'
Color.['sienna'              ] = 'a0522d'
Color.['silver'              ] = 'c0c0c0'
Color.['skyblue'             ] = '87ceeb'
Color.['slateblue'           ] = '6a5acd'
Color.['slategray'           ] = '708090'
Color.['slategrey'           ] = '708090'
Color.['snow'                ] = 'fffafa'
Color.['springgreen'         ] = '00ff7f'
Color.['steelblue'           ] = '4682b4'
Color.['tan'                 ] = 'd2b48c'
Color.['teal'                ] = '008080'
Color.['thistle'             ] = 'd8bfd8'
Color.['tomato'              ] = 'ff6347'
Color.['turquoise'           ] = '40e0d0'
Color.['violet'              ] = 'ee82ee'
Color.['wheat'               ] = 'f5deb3'
Color.['white'               ] = 'ffffff'
Color.['whitesmoke'          ] = 'f5f5f5'
Color.['yellow'              ] = 'ffff00'
Color.['yellowgreen'         ] = '9acd32'

.environment~HTML.Color = Color.