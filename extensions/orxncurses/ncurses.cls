/*----------------------------------------------------------Zero or 1.------------------*/
/*                                                                            */
/* Copyright (c) 2010-2010 Rexx Language Association. All rights reserved.    */
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
/* Authors;                                                                   */
/*       W. David Ashley <dashley@us.ibm.com>                                 */
/*                                                                            */
/*----------------------------------------------------------------------------*/


/*============================================================================*/
/* Class: Window - main class for the stdscr and other windows                */
/*============================================================================*/

::class Window public

::ATTRIBUTE stdscr class

-- chtype constants
::CONSTANT A_NORMAL	    0
::CONSTANT A_ATTRIBUTES	4294967040
::CONSTANT A_CHARTEXT	255
::CONSTANT A_COLOR		65208
::CONSTANT A_STANDOUT	65536
::CONSTANT A_UNDERLINE	131072
::CONSTANT A_REVERSE	262144
::CONSTANT A_BLINK		524288
::CONSTANT A_DIM		1048576
::CONSTANT A_BOLD		2097152
::CONSTANT A_ALTCHARSET	4194304
::CONSTANT A_INVIS		8388608
::CONSTANT A_PROTECT	16777216
::CONSTANT A_HORIZONTAL	33554432
::CONSTANT A_LEFT		67108864
::CONSTANT A_LOW		134217728
::CONSTANT A_RIGHT		268435456
::CONSTANT A_TOP		536870912
::CONSTANT A_VERTICAL	1073741824

-- mouse constants
::CONSTANT BUTTON1_RELEASED            1
::CONSTANT BUTTON1_PRESSED             2
::CONSTANT BUTTON1_CLICKED             4
::CONSTANT BUTTON1_DOUBLE_CLICKED      8
::CONSTANT BUTTON1_TRIPLE_CLICKED      16
::CONSTANT BUTTON1_RESERVED_EVENT      32
::CONSTANT BUTTON2_RELEASED            64
::CONSTANT BUTTON2_PRESSED             128
::CONSTANT BUTTON2_CLICKED             256
::CONSTANT BUTTON2_DOUBLE_CLICKED      512
::CONSTANT BUTTON2_TRIPLE_CLICKED      1024
::CONSTANT BUTTON2_RESERVED_EVENT      2048
::CONSTANT BUTTON3_RELEASED            4096
::CONSTANT BUTTON3_PRESSED             8192
::CONSTANT BUTTON3_CLICKED             16384
::CONSTANT BUTTON3_DOUBLE_CLICKED      32768
::CONSTANT BUTTON3_TRIPLE_CLICKED      65536
::CONSTANT BUTTON3_RESERVED_EVENT      131072
::CONSTANT BUTTON4_RELEASED            262144
::CONSTANT BUTTON4_PRESSED             524288
::CONSTANT BUTTON4_CLICKED             1048576
::CONSTANT BUTTON4_DOUBLE_CLICKED      2097152
::CONSTANT BUTTON4_TRIPLE_CLICKED      4194304
::CONSTANT BUTTON4_RESERVED_EVENT      8388600
::CONSTANT BUTTON_CTRL                 16777216
::CONSTANT BUTTON_SHIFT                33554432
::CONSTANT BUTTON_ALT                  61708864
::CONSTANT ALL_MOUSE_EVENTS            134217727
::CONSTANT REPORT_MOUSE_POSITION       134217728

-- nCurses constants
::CONSTANT ERR    -1
::CONSTANT OK     0
::CONSTANT FALSE  .false
::CONSTANT TRUE   .true

-- VT100 alternative character set symbols begin here
::METHOD ACS_ULCORNER	
return self~acs_map('l') /* upper left corner */
::METHOD ACS_LLCORNER	
return self~acs_map('m') /* lower left corner */
::METHOD ACS_URCORNER	
return self~acs_map('k') /* upper right corner */
::METHOD ACS_LRCORNER	
return self~acs_map('j') /* lower right corner */
::METHOD ACS_LTEE	
return self~acs_map('t') /* tee pointing right */
::METHOD ACS_RTEE	
return self~acs_map('u') /* tee pointing left */
::METHOD ACS_BTEE	
return self~acs_map('v') /* tee pointing up */
::METHOD ACS_TTEE	
return self~acs_map('w') /* tee pointing down */
::METHOD ACS_HLINE	
return self~acs_map('q') /* horizontal line */
::METHOD ACS_VLINE	
return self~acs_map('x') /* vertical line */
::METHOD ACS_PLUS	
return self~acs_map('n') /* large plus or crossover */
::METHOD ACS_S1		
return self~acs_map('o') /* scan line 1 */
::METHOD ACS_S9		
return self~acs_map('s') /* scan line 9 */
::METHOD ACS_DIAMOND	
return self~acs_map('`') /* diamond */
::METHOD ACS_CKBOARD	
return self~acs_map('a') /* checker board (stipple) */
::METHOD ACS_DEGREE	
return self~acs_map('f') /* degree symbol */
::METHOD ACS_PLMINUS	
return self~acs_map('g') /* plus/minus */
::METHOD ACS_BULLET	
return self~acs_map('~') /* bullet */
-- Teletype 5410v1 symbols begin here
::METHOD ACS_LARROW	
return self~acs_map(',') /* arrow pointing left */
::METHOD ACS_RARROW	
return self~acs_map('+') /* arrow pointing right */
::METHOD ACS_DARROW	
return self~acs_map('.') /* arrow pointing down */
::METHOD ACS_UARROW	
return self~acs_map('-') /* arrow pointing up */
::METHOD ACS_BOARD	
return self~acs_map('h') /* board of squares */
::METHOD ACS_LANTERN	
return self~acs_map('i') /* lantern symbol */
::METHOD ACS_BLOCK	
return self~acs_map('0') /* solid square block */
-- These aren't documented, but a lot of System Vs have them anyway
-- (you can spot pprryyzz{{||}} in a lot of AT&T terminfo strings).
-- The ACS_names may not match AT&T's, our source didn't know them.
::METHOD ACS_S3		
return self~acs_map('p') /* scan line 3 */
::METHOD ACS_S7		
return self~acs_map('r') /* scan line 7 */
::METHOD ACS_LEQUAL	
return self~acs_map('y') /* less/equal */
::METHOD ACS_GEQUAL	
return self~acs_map('z') /* greater/equal */
::METHOD ACS_PI		
return self~acs_map('{') /* Pi */
::METHOD ACS_NEQUAL	
return self~acs_map('|') /* not equal */
::METHOD ACS_STERLING	
return self~acs_map('}') /* UK pound sign */

-- ASCII codes
::CONSTANT ASCII_NUL	  0
::CONSTANT ASCII_SOH	  1
::CONSTANT ASCII_STX	  2
::CONSTANT ASCII_ETX	  3
::CONSTANT ASCII_EOT	  4
::CONSTANT ASCII_ENQ	  5
::CONSTANT ASCII_ACK	  6
::CONSTANT ASCII_BEL	  7
::CONSTANT ASCII_BS 	  8
::CONSTANT ASCII_TAB	  9
::CONSTANT ASCII_LF 	 10
::CONSTANT ASCII_VT 	 11
::CONSTANT ASCII_FF 	 12
::CONSTANT ASCII_CR 	 13
::CONSTANT ASCII_SO 	 14
::CONSTANT ASCII_SI 	 15
::CONSTANT ASCII_DLE	 16
::CONSTANT ASCII_DC1	 17
::CONSTANT ASCII_DC2	 18
::CONSTANT ASCII_DC3	 19
::CONSTANT ASCII_DC4	 20
::CONSTANT ASCII_NAK	 21
::CONSTANT ASCII_SYN	 22
::CONSTANT ASCII_ETB	 23
::CONSTANT ASCII_CAN	 24
::CONSTANT ASCII_EM 	 25
::CONSTANT ASCII_SUB	 26
::CONSTANT ASCII_ESC	 27
::CONSTANT ASCII_FS 	 28
::CONSTANT ASCII_GS 	 29
::CONSTANT ASCII_RS 	 30
::CONSTANT ASCII_US 	 31
::CONSTANT ASCII_DEL	127

-- Key codes
::CONSTANT KEY_CODE_YES	256		/* A wchar_t contains a key code */
::CONSTANT KEY_MIN		257		/* Minimum curses key */
::CONSTANT KEY_BREAK	257		/* Break key (unreliable) */
::CONSTANT KEY_SRESET	344		/* Soft (partial) reset (unreliable) */
::CONSTANT KEY_RESET	345		/* Reset or hard reset (unreliable) */
::CONSTANT KEY_DOWN	    258		/* down-arrow key */
::CONSTANT KEY_UP		259		/* up-arrow key */
::CONSTANT KEY_LEFT	    260		/* left-arrow key */
::CONSTANT KEY_RIGHT	261		/* right-arrow key */
::CONSTANT KEY_HOME	    262		/* home key */
::CONSTANT KEY_BACKSPACE 263	/* backspace key */
::CONSTANT KEY_F0		264		/* Function key */
::CONSTANT KEY_F1		265		/* Function key */
::CONSTANT KEY_F2		266		/* Function key */
::CONSTANT KEY_F3		267		/* Function key */
::CONSTANT KEY_F4		268		/* Function key */
::CONSTANT KEY_F5		269		/* Function key */
::CONSTANT KEY_F6		270		/* Function key */
::CONSTANT KEY_F7		271		/* Function key */
::CONSTANT KEY_F8		272		/* Function key */
::CONSTANT KEY_F9		273		/* Function key */
::CONSTANT KEY_F10		274		/* Function key */
::CONSTANT KEY_F11		275		/* Function key */
::CONSTANT KEY_F12		276		/* Function key */
::CONSTANT KEY_F13		277		/* Function key */
::CONSTANT KEY_F14		278		/* Function key */
::CONSTANT KEY_F15		279		/* Function key */
::CONSTANT KEY_F16		280		/* Function key */
::CONSTANT KEY_F17		281		/* Function key */
::CONSTANT KEY_F18		282		/* Function key */
::CONSTANT KEY_F19		283		/* Function key */
::CONSTANT KEY_F20		284		/* Function key */
::CONSTANT KEY_F21		285		/* Function key */
::CONSTANT KEY_F22		286		/* Function key */
::CONSTANT KEY_F23		287		/* Function key */
::CONSTANT KEY_F24		288		/* Function key */
::CONSTANT KEY_DL		328		/* delete-line key */
::CONSTANT KEY_IL		329		/* insert-line key */
::CONSTANT KEY_DC		330		/* delete-character key */
::CONSTANT KEY_IC		331		/* insert-character key */
::CONSTANT KEY_EIC		332		/* sent by rmir or smir in insert mode */
::CONSTANT KEY_CLEAR	333		/* clear-screen or erase key */
::CONSTANT KEY_EOS		334		/* clear-to-end-of-screen key */
::CONSTANT KEY_EOL		335		/* clear-to-end-of-line key */
::CONSTANT KEY_SF		336		/* scroll-forward key */
::CONSTANT KEY_SR		337		/* scroll-backward key */
::CONSTANT KEY_NPAGE	338		/* next-page key */
::CONSTANT KEY_PPAGE	339		/* previous-page key */
::CONSTANT KEY_STAB	    340		/* set-tab key */
::CONSTANT KEY_CTAB	    341		/* clear-tab key */
::CONSTANT KEY_CATAB	342		/* clear-all-tabs key */
::CONSTANT KEY_ENTER	343		/* enter/send key */
::CONSTANT KEY_PRINT	346		/* print key */
::CONSTANT KEY_LL		347		/* lower-left key (home down) */
::CONSTANT KEY_A1		348		/* upper left of keypad */
::CONSTANT KEY_A3		349		/* upper right of keypad */
::CONSTANT KEY_B2		350		/* center of keypad */
::CONSTANT KEY_C1		351		/* lower left of keypad */
::CONSTANT KEY_C3		352		/* lower right of keypad */
::CONSTANT KEY_BTAB	    353		/* back-tab key */
::CONSTANT KEY_BEG		354		/* begin key */
::CONSTANT KEY_CANCEL	355		/* cancel key */
::CONSTANT KEY_CLOSE	356		/* close key */
::CONSTANT KEY_COMMAND	357		/* command key */
::CONSTANT KEY_COPY	    358		/* copy key */
::CONSTANT KEY_CREATE	359		/* create key */
::CONSTANT KEY_END		360		/* end key */
::CONSTANT KEY_EXIT	    361		/* exit key */
::CONSTANT KEY_FIND	    362		/* find key */
::CONSTANT KEY_HELP	    363		/* help key */
::CONSTANT KEY_MARK	    364		/* mark key */
::CONSTANT KEY_MESSAGE	365		/* message key */
::CONSTANT KEY_MOVE	    366		/* move key */
::CONSTANT KEY_NEXT	    367		/* next key */
::CONSTANT KEY_OPEN	    368		/* open key */
::CONSTANT KEY_OPTIONS	369		/* options key */
::CONSTANT KEY_PREVIOUS	370		/* previous key */
::CONSTANT KEY_REDO	    371		/* redo key */
::CONSTANT KEY_REFERENCE 372		/* reference key */
::CONSTANT KEY_REFRESH	373		/* refresh key */
::CONSTANT KEY_REPLACE	374		/* replace key */
::CONSTANT KEY_RESTART	375		/* restart key */
::CONSTANT KEY_RESUME	376		/* resume key */
::CONSTANT KEY_SAVE	    377		/* save key */
::CONSTANT KEY_SBEG	    378		/* shifted begin key */
::CONSTANT KEY_SCANCEL	379		/* shifted cancel key */
::CONSTANT KEY_SCOMMAND	380		/* shifted command key */
::CONSTANT KEY_SCOPY	381		/* shifted copy key */
::CONSTANT KEY_SCREATE	382		/* shifted create key */
::CONSTANT KEY_SDC		383		/* shifted delete-character key */
::CONSTANT KEY_SDL		384		/* shifted delete-line key */
::CONSTANT KEY_SELECT	385		/* select key */
::CONSTANT KEY_SEND	    386		/* shifted end key */
::CONSTANT KEY_SEOL	    387		/* shifted clear-to-end-of-line key */
::CONSTANT KEY_SEXIT	388		/* shifted exit key */
::CONSTANT KEY_SFIND	389		/* shifted find key */
::CONSTANT KEY_SHELP	390		/* shifted help key */
::CONSTANT KEY_SHOME	391		/* shifted home key */
::CONSTANT KEY_SIC		392		/* shifted insert-character key */
::CONSTANT KEY_SLEFT	393		/* shifted left-arrow key */
::CONSTANT KEY_SMESSAGE	394		/* shifted message key */
::CONSTANT KEY_SMOVE	395		/* shifted move key */
::CONSTANT KEY_SNEXT	396		/* shifted next key */
::CONSTANT KEY_SOPTIONS	397		/* shifted options key */
::CONSTANT KEY_SPREVIOUS 398	/* shifted previous key */
::CONSTANT KEY_SPRINT	399		/* shifted print key */
::CONSTANT KEY_SREDO	400		/* shifted redo key */
::CONSTANT KEY_SREPLACE	401		/* shifted replace key */
::CONSTANT KEY_SRIGHT	402		/* shifted right-arrow key */
::CONSTANT KEY_SRSUME	403		/* shifted resume key */
::CONSTANT KEY_SSAVE	404		/* shifted save key */
::CONSTANT KEY_SSUSPEND	405		/* shifted suspend key */
::CONSTANT KEY_SUNDO	406		/* shifted undo key */
::CONSTANT KEY_SUSPEND	407		/* suspend key */
::CONSTANT KEY_UNDO	    408		/* undo key */
::CONSTANT KEY_MOUSE	409		/* Mouse event has occurred */
::CONSTANT KEY_RESIZE	410		/* Terminal resize event */
::CONSTANT KEY_EVENT	411		/* We were interrupted by an event */
::CONSTANT KEY_MAX		511		/* Maximum key value is 0633 */

-- color constants
::CONSTANT COLOR_BLACK     0
::CONSTANT COLOR_RED       1
::CONSTANT COLOR_GREEN     2
::CONSTANT COLOR_YELLOW    3
::CONSTANT COLOR_BLUE      4
::CONSTANT COLOR_MAGENTA   5
::CONSTANT COLOR_CYAN      6
::CONSTANT COLOR_WHITE     7
                           
::METHOD setbase public class EXTERNAL "LIBRARY orxncurses OrxCurSetBase"
::METHOD orxncurses_version public class EXTERNAL "LIBRARY orxncurses OrxCurVersion"
::METHOD slk_init public class EXTERNAL "LIBRARY orxncurses OrxCurSlk_init"

::METHOD init
if .window~stdscr() = 'STDSCR' then do
   use strict arg
   self~first_init()
   .window~stdscr = self
   end
else if arg() = 1 then do
   use strict arg ptr
   self~newwinfromptr(ptr)
   end
else do
   use strict arg nlines, ncols, begin_y, begin_x
   self~next_init(nlines, ncols, begin_y, begin_x)
   end
return

::METHOD first_init private EXTERNAL "LIBRARY orxncurses OrxCurInitscr"
::METHOD next_init private EXTERNAL "LIBRARY orxncurses OrxCurNewwin"
::METHOD newwinfromptr private EXTERNAL "LIBRARY orxncurses OrxCurNewwinfromptr"
::METHOD acs_map private EXTERNAL "LIBRARY orxncurses OrxCurAcs_map"
::METHOD subwinprivate private EXTERNAL "LIBRARY orxncurses OrxCurSubwinprivate"
::METHOD subpadprivate private EXTERNAL "LIBRARY orxncurses OrxCurSubpadprivate"

::METHOD addch EXTERNAL "LIBRARY orxncurses OrxCurAddch"
::METHOD mvaddch EXTERNAL "LIBRARY orxncurses OrxCurMvaddch"
::METHOD addchstr EXTERNAL "LIBRARY orxncurses OrxCurAddchstr"
::METHOD mvaddchstr EXTERNAL "LIBRARY orxncurses OrxCurMvaddchstr"
::METHOD addchnstr EXTERNAL "LIBRARY orxncurses OrxCurAddchnstr"
::METHOD mvaddchnstr EXTERNAL "LIBRARY orxncurses OrxCurMvaddchnstr"
::METHOD addstr EXTERNAL "LIBRARY orxncurses OrxCurAddstr"
::METHOD mvaddstr EXTERNAL "LIBRARY orxncurses OrxCurMvaddstr"
::METHOD addnstr EXTERNAL "LIBRARY orxncurses OrxCurAddnstr"
::METHOD mvaddnstr EXTERNAL "LIBRARY orxncurses OrxCurMvaddnstr"
::METHOD assume_default_colors EXTERNAL "LIBRARY orxncurses OrxCurAssume_default_colors"
::METHOD attroff EXTERNAL "LIBRARY orxncurses OrxCurAttroff"
::METHOD attron EXTERNAL "LIBRARY orxncurses OrxCurAttron"
::METHOD attrset EXTERNAL "LIBRARY orxncurses OrxCurAttrset"
::METHOD baudrate EXTERNAL "LIBRARY orxncurses OrxCurBaudrate"
::METHOD beep EXTERNAL "LIBRARY orxncurses OrxCurBeep"
::METHOD bkgd EXTERNAL "LIBRARY orxncurses OrxCurBkgd"
::METHOD bkgdset EXTERNAL "LIBRARY orxncurses OrxCurBkgdset"
::METHOD border EXTERNAL "LIBRARY orxncurses OrxCurBorder"
::METHOD box EXTERNAL "LIBRARY orxncurses OrxCurBox"
::METHOD can_change_color EXTERNAL "LIBRARY orxncurses OrxCurCan_change_color"
::METHOD cbreak EXTERNAL "LIBRARY orxncurses OrxCurCbreak"
::METHOD nocbreak EXTERNAL "LIBRARY orxncurses OrxCurNocbreak"
::METHOD chgat EXTERNAL "LIBRARY orxncurses OrxCurChgat"
::METHOD mvchgat EXTERNAL "LIBRARY orxncurses OrxCurMvchgat"
::METHOD clear EXTERNAL "LIBRARY orxncurses OrxCurClear"
::METHOD clearok EXTERNAL "LIBRARY orxncurses OrxCurClearok"
::METHOD clrtobot EXTERNAL "LIBRARY orxncurses OrxCurClrtobot"
::METHOD clrtoeol EXTERNAL "LIBRARY orxncurses OrxCurClrtoeol"
::METHOD color_set EXTERNAL "LIBRARY orxncurses OrxCurColor_set"
::METHOD colors EXTERNAL "LIBRARY orxncurses OrxCurColors"
::METHOD color_pair EXTERNAL "LIBRARY orxncurses OrxCurColor_pair"
::METHOD color_pairs EXTERNAL "LIBRARY orxncurses OrxCurColor_pairs"
::METHOD cols EXTERNAL "LIBRARY orxncurses OrxCurCols"
::METHOD copywin EXTERNAL "LIBRARY orxncurses OrxCurCopywin"
::METHOD curs_set EXTERNAL "LIBRARY orxncurses OrxCurCurs_set"
::METHOD curses_version EXTERNAL "LIBRARY orxncurses OrxCurCurses_version"
::METHOD delch EXTERNAL "LIBRARY orxncurses OrxCurDelch"
::METHOD deleteln EXTERNAL "LIBRARY orxncurses OrxCurDeleteln"
::METHOD delwin EXTERNAL "LIBRARY orxncurses OrxCurDelwin"
::METHOD derwin
use strict arg nlines, ncols, begy, begx
ptr = self~derwinprivate(nlines, ncols, begy, begx)
return .window~new(ptr)
::METHOD derwinprivate private EXTERNAL "LIBRARY orxncurses OrxCurDerwinprivate"
::METHOD doupdate EXTERNAL "LIBRARY orxncurses OrxCurDoupdate"
::METHOD dupwin
use strict arg
ptr = self~dupwinprivate()
return .window~new(ptr)
::METHOD dupwinprivate private EXTERNAL "LIBRARY orxncurses OrxCurDupwinprivate"
::METHOD echo EXTERNAL "LIBRARY orxncurses OrxCurEcho"
::METHOD noecho EXTERNAL "LIBRARY orxncurses OrxCurNoecho"
::METHOD echochar EXTERNAL "LIBRARY orxncurses OrxCurEchochar"
::METHOD endwin EXTERNAL "LIBRARY orxncurses OrxCurEndwin"
::METHOD erase EXTERNAL "LIBRARY orxncurses OrxCurErase"
::METHOD erasechar EXTERNAL "LIBRARY orxncurses OrxCurErasechar"
::METHOD filter EXTERNAL "LIBRARY orxncurses OrxCurFilter"
::METHOD flash EXTERNAL "LIBRARY orxncurses OrxCurFlash"
::METHOD flushinp EXTERNAL "LIBRARY orxncurses OrxCurFlushinp"
::METHOD getbegyx EXTERNAL "LIBRARY orxncurses OrxCurGetbegyx"
::METHOD getbkgd EXTERNAL "LIBRARY orxncurses OrxCurGetbkgd"
::METHOD getch EXTERNAL "LIBRARY orxncurses OrxCurGetch"
::METHOD mvgetch EXTERNAL "LIBRARY orxncurses OrxCurMvgetch"
::METHOD getmaxyx EXTERNAL "LIBRARY orxncurses OrxCurGetmaxyx"
::METHOD getparyx EXTERNAL "LIBRARY orxncurses OrxCurGetparyx"
::METHOD getmouseprivate private EXTERNAL "LIBRARY orxncurses OrxCurGetmouseprivate"
::METHOD getmouse
ev = .Mevent~new()
self~getmouseprivate(ev)
return ev
::METHOD getstr EXTERNAL "LIBRARY orxncurses OrxCurGetstr"
::METHOD getnstr EXTERNAL "LIBRARY orxncurses OrxCurGetnstr"
-- ::METHOD getsyx EXTERNAL "LIBRARY orxncurses OrxCurGetsyx"
::METHOD mvgetstr EXTERNAL "LIBRARY orxncurses OrxCurMvgetstr"
::METHOD mvgetnstr EXTERNAL "LIBRARY orxncurses OrxCurMvgetnstr"
::METHOD getwinprivate private EXTERNAL "LIBRARY orxncurses OrxCurGetwinprivate"
::METHOD getwin
use strict arg fname
ptr = self~getwinprivate(fname)
return .Window~new(ptr)
::METHOD getyx EXTERNAL "LIBRARY orxncurses OrxCurGetyx"
::METHOD halfdelay EXTERNAL "LIBRARY orxncurses OrxCurHalfdelay"
::METHOD has_colors EXTERNAL "LIBRARY orxncurses OrxCurHas_colors"
::METHOD has_ic EXTERNAL "LIBRARY orxncurses OrxCurHas_ic"
::METHOD has_il EXTERNAL "LIBRARY orxncurses OrxCurHas_il"
::METHOD hline EXTERNAL "LIBRARY orxncurses OrxCurHline"
::METHOD mvhline EXTERNAL "LIBRARY orxncurses OrxCurMvhline"
::METHOD idcok EXTERNAL "LIBRARY orxncurses OrxCurIdcok"
::METHOD idlok EXTERNAL "LIBRARY orxncurses OrxCurIdlok"
::METHOD immedok EXTERNAL "LIBRARY orxncurses OrxCurImmedok"
::METHOD inch EXTERNAL "LIBRARY orxncurses OrxCurInch"
::METHOD mvinch EXTERNAL "LIBRARY orxncurses OrxCurMvinch"
::METHOD inchstr EXTERNAL "LIBRARY orxncurses OrxCurInchstr"
::METHOD mvinchstr EXTERNAL "LIBRARY orxncurses OrxCurMvinchstr"
::METHOD inchnstr EXTERNAL "LIBRARY orxncurses OrxCurInchnstr"
::METHOD mvinchnstr EXTERNAL "LIBRARY orxncurses OrxCurMvinchnstr"
::METHOD init_color EXTERNAL "LIBRARY orxncurses OrxCurInit_color"
::METHOD init_pair EXTERNAL "LIBRARY orxncurses OrxCurInit_pair"
::METHOD insdelln EXTERNAL "LIBRARY orxncurses OrxCurInsdelln"
::METHOD insertln EXTERNAL "LIBRARY orxncurses OrxCurInsertln"
::METHOD insch EXTERNAL "LIBRARY orxncurses OrxCurInsch"
::METHOD mvinsch EXTERNAL "LIBRARY orxncurses OrxCurMvinsch"
::METHOD insstr EXTERNAL "LIBRARY orxncurses OrxCurInsstr"
::METHOD insnstr EXTERNAL "LIBRARY orxncurses OrxCurInsnstr"
::METHOD mvinsstr EXTERNAL "LIBRARY orxncurses OrxCurMvinsstr"
::METHOD mvinsnstr EXTERNAL "LIBRARY orxncurses OrxCurMvinsnstr"
::METHOD instr EXTERNAL "LIBRARY orxncurses OrxCurInstr"
::METHOD innstr EXTERNAL "LIBRARY orxncurses OrxCurInnstr"
::METHOD mvinstr EXTERNAL "LIBRARY orxncurses OrxCurMvinstr"
::METHOD mvinnstr EXTERNAL "LIBRARY orxncurses OrxCurMvinnstr"
::METHOD intrflush EXTERNAL "LIBRARY orxncurses OrxCurIntrflush"
::METHOD isendwin EXTERNAL "LIBRARY orxncurses OrxCurIsendwin"
::METHOD is_linetouched EXTERNAL "LIBRARY orxncurses OrxCurIs_linetouched"
::METHOD is_wintouched EXTERNAL "LIBRARY orxncurses OrxCurIs_wintouched"
::METHOD keyname EXTERNAL "LIBRARY orxncurses OrxCurKeyname"
::METHOD keypad EXTERNAL "LIBRARY orxncurses OrxCurKeypad"
::METHOD killchar EXTERNAL "LIBRARY orxncurses OrxCurKillchar"
::METHOD leaveok EXTERNAL "LIBRARY orxncurses OrxCurLeaveok"
::METHOD lines EXTERNAL "LIBRARY orxncurses OrxCurLines"
::METHOD longname EXTERNAL "LIBRARY orxncurses OrxCurLongname"
::METHOD meta EXTERNAL "LIBRARY orxncurses OrxCurMeta"
::METHOD mouse_trafo EXTERNAL "LIBRARY orxncurses OrxCurMouse_trafo"
::METHOD mousemask EXTERNAL "LIBRARY orxncurses OrxCurMousemask"
::METHOD move EXTERNAL "LIBRARY orxncurses OrxCurMove"
::METHOD mvderwin EXTERNAL "LIBRARY orxncurses OrxCurMvderwin"
::METHOD mvwin EXTERNAL "LIBRARY orxncurses OrxCurMvwin"
::METHOD napms EXTERNAL "LIBRARY orxncurses OrxCurNapms"
::METHOD ncurses_mouse_version EXTERNAL "LIBRARY orxncurses OrxCurNcurses_mouse_version"
::METHOD ncurses_version EXTERNAL "LIBRARY orxncurses OrxCurNcurses_version"
-- ::METHOD newterm EXTERNAL "LIBRARY orxncurses OrxCurNewterm"
::METHOD nl EXTERNAL "LIBRARY orxncurses OrxCurNl"
::METHOD nonl EXTERNAL "LIBRARY orxncurses OrxCurNonl"
::METHOD nodelay EXTERNAL "LIBRARY orxncurses OrxCurNodelay"
::METHOD notimeout EXTERNAL "LIBRARY orxncurses OrxCurNotimeout"
::METHOD overlay EXTERNAL "LIBRARY orxncurses OrxCurOverlay"
::METHOD overwrite EXTERNAL "LIBRARY orxncurses OrxCurOverwrite"
::METHOD pair_content EXTERNAL "LIBRARY orxncurses OrxCurPair_content"
::METHOD pair_number EXTERNAL "LIBRARY orxncurses OrxCurPair_number"
::METHOD pechochar
return self~ERR
::METHOD pnoutrefresh 
return self~ERR
::METHOD prefresh 
return self~ERR
-- ::METHOD printw EXTERNAL "LIBRARY orxncurses OrxCurPrintw"
::METHOD putwin EXTERNAL "LIBRARY orxncurses OrxCurPutwin"
::METHOD qiflush EXTERNAL "LIBRARY orxncurses OrxCurQiflush"
::METHOD noqiflush EXTERNAL "LIBRARY orxncurses OrxCurNoqiflush"
::METHOD raw EXTERNAL "LIBRARY orxncurses OrxCurRaw"
::METHOD noraw EXTERNAL "LIBRARY orxncurses OrxCurNoraw"
::METHOD redrawwin EXTERNAL "LIBRARY orxncurses OrxCurRedrawwin"
::METHOD refresh EXTERNAL "LIBRARY orxncurses OrxCurRefresh"
-- ::METHOD ripoffline EXTERNAL "LIBRARY orxncurses OrxCurRipoffline"
-- ::METHOD scanw EXTERNAL "LIBRARY orxncurses OrxCurScanw"
::METHOD scr_dump EXTERNAL "LIBRARY orxncurses OrxCurScr_dump"
::METHOD scr_restore EXTERNAL "LIBRARY orxncurses OrxCurScr_restore"
::METHOD scrl EXTERNAL "LIBRARY orxncurses OrxCurScrl"
::METHOD scroll EXTERNAL "LIBRARY orxncurses OrxCurScroll"
::METHOD scrollok EXTERNAL "LIBRARY orxncurses OrxCurScrollok"
::METHOD setscrreg EXTERNAL "LIBRARY orxncurses OrxCurSetscrreg"
-- ::METHOD setsyx EXTERNAL "LIBRARY orxncurses OrxCurSetsyx"
-- ::METHOD set_term EXTERNAL "LIBRARY orxncurses OrxCurSet_term"
::METHOD slk_attr EXTERNAL "LIBRARY orxncurses OrxCurSlk_attr"
::METHOD slk_attroff EXTERNAL "LIBRARY orxncurses OrxCurSlk_attroff"
::METHOD slk_attron EXTERNAL "LIBRARY orxncurses OrxCurSlk_attron"
::METHOD slk_attrset EXTERNAL "LIBRARY orxncurses OrxCurSlk_attrset"
::METHOD slk_clear EXTERNAL "LIBRARY orxncurses OrxCurSlk_clear"
::METHOD slk_color EXTERNAL "LIBRARY orxncurses OrxCurSlk_color"
::METHOD slk_label EXTERNAL "LIBRARY orxncurses OrxCurSlk_label"
::METHOD slk_noutrefresh EXTERNAL "LIBRARY orxncurses OrxCurSlk_noutrefresh"
::METHOD slk_refresh EXTERNAL "LIBRARY orxncurses OrxCurSlk_refresh"
::METHOD slk_restore EXTERNAL "LIBRARY orxncurses OrxCurSlk_restore"
::METHOD slk_set EXTERNAL "LIBRARY orxncurses OrxCurSlk_set"
::METHOD slk_touch EXTERNAL "LIBRARY orxncurses OrxCurSlk_touch"
::METHOD standend EXTERNAL "LIBRARY orxncurses OrxCurStandend"
::METHOD standout EXTERNAL "LIBRARY orxncurses OrxCurStandout"
::METHOD start_color EXTERNAL "LIBRARY orxncurses OrxCurStart_color"
::METHOD subpad
return self~ERR
::METHOD subwin
use strict arg nlines, ncols, begy, begx
ptr = self~subwinprivate(nlines, ncols, begy, begx)
return .window~new(ptr)
::METHOD syncok EXTERNAL "LIBRARY orxncurses OrxCurSyncok"
::METHOD tabsize EXTERNAL "LIBRARY orxncurses OrxCurTabsize"
::METHOD termattrs EXTERNAL "LIBRARY orxncurses OrxCurTermattrs"
::METHOD termname EXTERNAL "LIBRARY orxncurses OrxCurTermname"
::METHOD timeout EXTERNAL "LIBRARY orxncurses OrxCurTimeout"
::METHOD touchline EXTERNAL "LIBRARY orxncurses OrxCurTouchline"
::METHOD touchwin EXTERNAL "LIBRARY orxncurses OrxCurTouchwin"
::METHOD typeahead EXTERNAL "LIBRARY orxncurses OrxCurTypeahead"
::METHOD unctrl EXTERNAL "LIBRARY orxncurses OrxCurUnctrl"
::METHOD ungetch EXTERNAL "LIBRARY orxncurses OrxCurUngetch"
::METHOD untouchwin EXTERNAL "LIBRARY orxncurses OrxCurUntouchwin"
::METHOD use_default_colors EXTERNAL "LIBRARY orxncurses OrxCurUse_default_colors"
::METHOD use_env EXTERNAL "LIBRARY orxncurses OrxCurUse_env"
::METHOD vline EXTERNAL "LIBRARY orxncurses OrxCurVline"
::METHOD mvvline EXTERNAL "LIBRARY orxncurses OrxCurMvvline"
::METHOD wenclose EXTERNAL "LIBRARY orxncurses OrxCurWenclose"
::METHOD wnoutrefresh EXTERNAL "LIBRARY orxncurses OrxCurWnoutrefresh"
::METHOD wredrawln EXTERNAL "LIBRARY orxncurses OrxCurWredrawln"
::METHOD wsyncdown EXTERNAL "LIBRARY orxncurses OrxCurWsyncdown"
::METHOD wsyncup EXTERNAL "LIBRARY orxncurses OrxCurWsyncup"
::METHOD wtouchln EXTERNAL "LIBRARY orxncurses OrxCurWtouchln"


/*============================================================================*/
/* Class: Mevent - mouse events                                               */
/*============================================================================*/

::class Mevent public

::CONSTANT BUTTON1_RELEASED            1
::CONSTANT BUTTON1_PRESSED             2
::CONSTANT BUTTON1_CLICKED             4
::CONSTANT BUTTON1_DOUBLE_CLICKED      8
::CONSTANT BUTTON1_TRIPLE_CLICKED      16
::CONSTANT BUTTON1_RESERVED_EVENT      32
::CONSTANT BUTTON2_RELEASED            64
::CONSTANT BUTTON2_PRESSED             128
::CONSTANT BUTTON2_CLICKED             256
::CONSTANT BUTTON2_DOUBLE_CLICKED      512
::CONSTANT BUTTON2_TRIPLE_CLICKED      1024
::CONSTANT BUTTON2_RESERVED_EVENT      2048
::CONSTANT BUTTON3_RELEASED            4096
::CONSTANT BUTTON3_PRESSED             8192
::CONSTANT BUTTON3_CLICKED             16384
::CONSTANT BUTTON3_DOUBLE_CLICKED      32768
::CONSTANT BUTTON3_TRIPLE_CLICKED      65536
::CONSTANT BUTTON3_RESERVED_EVENT      131072
::CONSTANT BUTTON4_RELEASED            262144
::CONSTANT BUTTON4_PRESSED             524288
::CONSTANT BUTTON4_CLICKED             1048576
::CONSTANT BUTTON4_DOUBLE_CLICKED      2097152
::CONSTANT BUTTON4_TRIPLE_CLICKED      4194304
::CONSTANT BUTTON4_RESERVED_EVENT      8388600
::CONSTANT BUTTON_CTRL                 16777216
::CONSTANT BUTTON_SHIFT                33554432
::CONSTANT BUTTON_ALT                  61708864
::CONSTANT ALL_MOUSE_EVENTS            134217727
::CONSTANT REPORT_MOUSE_POSITION       134217728

::ATTRIBUTE id
::ATTRIBUTE x
::ATTRIBUTE y
::ATTRIBUTE z
::ATTRIBUTE bstate
::METHOD isbitset EXTERNAL "LIBRARY orxncurses OrxCurIsbitset"


/*============================================================================*/
/* Class: Pad - window pads                                                   */
/*============================================================================*/

::class Pad public subclass Window

::METHOD init 
if arg() = 2 then do
   use strict arg nlines, ncols
   self~newpadprivate(nlines, ncols)
   end
else do
   use strict arg pad, nlines, ncols, begy, begx
   self~newsubpadprivate(pad, nlines, ncols, begy, begx)
   end
return

::METHOD newpadprivate private EXTERNAL "LIBRARY orxncurses OrxCurNewpad"
::METHOD newsubpadprivate private EXTERNAL "LIBRARY orxncurses OrxCurSubpadprivate"

::METHOD echochar
return self~ERR
::METHOD mvwin
return self~ERR
::METHOD pechochar EXTERNAL "LIBRARY orxncurses OrxCurPechochar"
-- TODO: Need to figure out why pnoutrefresh does not work
--::METHOD pnoutrefresh EXTERNAL "LIBRARY orxncurses OrxCurPnoutrefresh"
::METHOD pnoutrefresh 
use strict arg minrow, mincol, sminrow, smincol, smaxrow, smaxcol
.window~stdscr()~copywin(self, minrow, mincol, sminrow, smincol, smaxrow, smaxcol, .false)
return self~OK
-- TODO: Need to figure out why prefresh does not work
--::METHOD prefresh EXTERNAL "LIBRARY orxncurses OrxCurPrefresh"
::METHOD prefresh
use strict arg minrow, mincol, sminrow, smincol, smaxrow, smaxcol
.window~stdscr()~copywin(self, minrow, mincol, sminrow, smincol, smaxrow, smaxcol, .false)
.window~stdscr()~refresh()
return self~OK
::METHOD redrawwin
return self~ERR
::METHOD refresh
return self~ERR
::METHOD scroll
return self~ERR
::METHOD scrollok
return self~ERR
::METHOD scrl
return self~ERR
::METHOD subpad
use strict arg nlines, ncols, begy, begx
return .pad~new(self, nlines, ncols, begy, begx)
::METHOD subwin
return self~ERR
::METHOD wnoutrefresh
return self~ERR


/*============================================================================*/
/* Class: Chtype - support class for a full chtype                            */
/*============================================================================*/

::class Chtype public

::ATTRIBUTE attr
::ATTRIBUTE colorpair
::ATTRIBUTE char

::METHOD init
expose attr colorpair char
use strict arg attr, colorpair, char
return

::METHOD string
expose attr colorpair char
use strict arg attr, colorpair, char
numeric digits 12
return (attr + colorpair + char~c2d())

