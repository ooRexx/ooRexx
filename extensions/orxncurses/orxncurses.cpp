/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2010-2021 Rexx Language Association. All rights reserved.    */
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <oorexxapi.h>
#if defined CURSES_HAVE_NCURSES_H
#include <ncurses.h>
#elif defined CURSES_HAVE_NCURSES_NCURSES_H
#include <ncurses/ncurses.h>
#else
#include <curses.h>
#endif

/*----------------------------------------------------------------------------*/
/* Local Definitions/variables                                                */
/*----------------------------------------------------------------------------*/

#define VERSTRING(major,minor,rel) #major "." #minor "." #rel

bool onebased = true;
#define ADDONE(x)        (onebased ? x + 1 : x)
#define SUBTRACTONE(x)   (onebased ? x - 1 : x)

/*============================================================================*/
/* Public Methods                                                             */
/*============================================================================*/

/**
 * Method:  OrxCurVersion
 *
 * Return the OrxnCurses version string.
 *
 * @return        Zero.
 **/
RexxMethod0(RexxObjectPtr,             // Return type
            OrxCurVersion)             // Object_method name
{

    return (RexxObjectPtr)context->NewStringFromAsciiz(VERSTRING(VMAJOR, VMINOR, VREL));
}

/**
 * Method:  OrxCurSetBase
 *
 * Set whether or not the library uses one-based indexes or
 * zero-based indexes. The default is one-based.
 *
 * @param base    1 = one-based, 0 = zero-based
 *
 * @return        base.
 **/
RexxMethod1(logical_t,                 // Return type
            OrxCurSetBase,             // Object_method name
            OPTIONAL_logical_t, base)
{

   if (argumentExists(1)) {
       onebased = base;
   }
   return onebased;
}

/**
 * Method:  OrxCurInitscr
 *
 * Init the standard screen..
 *
 * @return        Zero.
 **/
RexxMethod0(POINTER,                   // Return type
            OrxCurInitscr)             // Object_method name
{

    initscr();
    context->SetObjectVariable("CSELF", context->NewPointer(stdscr));
    return context->NewPointer(stdscr);
}

/**
 * Method:  OrxCurNewwin
 *
 * Create a new window.
 *
 * @param nlines  Number of lines.
 *
 * @param ncols   Number of columns.
 *
 * @param begin_y Y position start position.
 *
 * @param begin_x X position start position.
 *
 * @return        Zero.
 **/
RexxMethod4(int,                       // Return type
            OrxCurNewwin,              // Object_method name
            int, nlines,
            int, ncols,
            int, begin_y,
            int, begin_x)
{

    context->SetObjectVariable("CSELF", context->NewPointer(newwin(nlines, ncols,
                                                                   SUBTRACTONE(begin_y),
                                                                   SUBTRACTONE(begin_x))));
    return 0;
}

/**
 * Method:  OrxCurNewwinfromptr
 *
 * Create a new window.
 *
 * @param ptr     Pointer to the window.
 *
 * @return        Zero.
 **/
RexxMethod1(RexxObjectPtr,             // Return type
            OrxCurNewwinfromptr,       // Object_method name
            POINTER, ptr)
{

    context->SetObjectVariable("CSELF", context->NewPointer(ptr));
    return 0;
}

/**
 * Method:  OrxCurEndwin
 *
 * End nCurses formatting.
 *
 * @return        Zero.
 **/
RexxMethod0(int,                       // Return type
            OrxCurEndwin)              // Object_method name
{

    return endwin();
}

/**
 * Method:  OrxCurRefresh
 *
 * Refresh the screen and turn nCurses formatting on.
 *
 * @return        Zero.
 **/
RexxMethod1(int,                       // Return type
            OrxCurRefresh,             // Object_method name
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return wrefresh((WINDOW *)cself);
}

/**
 * Method:  OrxCurAddch
 *
 * Add a single character to the screen.
 *
 * @param str     Character to be added.
 *
 * @return        Zero.
 **/
RexxMethod2(int,                       // Return type
            OrxCurAddch,               // Object_method name
            CSTRING, str,              // Character
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return waddch((WINDOW *)cself, *str);
}

/**
 * Method:  OrxCurMvaddch
 *
 * Add a single character to the screen after moving the cursor.
 *
 * @param y       Y position.
 *
 * @param x       X position.
 *
 * @param str     Character to be added.
 *
 * @return        Zero.
 **/
RexxMethod4(int,                       // Return type
            OrxCurMvaddch,             // Object_method name
            int, y,                    // Y position
            int, x,                    // X position
            CSTRING, str,              // Character
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return mvwaddch((WINDOW *)cself, SUBTRACTONE(y), SUBTRACTONE(x), *str);
}

/**
 * Method:  OrxCurAddchstr
 *
 * Add a chtype character string to the screen.
 *
 * @param str     String to be added.
 *
 * @return        Zero.
 **/
RexxMethod2(int,                       // Return type
            OrxCurAddchstr,            // Object_method name
            CSTRING, str,              // Character
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return waddchstr((WINDOW *)cself, (const chtype *)str);
}

/**
 * Method:  OrxCurMvaddchstr
 *
 * Add a chtype character string to the screen after moving the
 * cursor.
 *
 * @param y       Y position.
 *
 * @param x       X position.
 *
 * @param str     String to be added.
 *
 * @return        Zero.
 **/
RexxMethod4(int,                       // Return type
            OrxCurMvaddchstr,          // Object_method name
            int, y,                    // Y position
            int, x,                    // X position
            CSTRING, str,              // Character
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return mvwaddchstr((WINDOW *)cself, SUBTRACTONE(y), SUBTRACTONE(x), (const chtype *)str);
}

/**
 * Method:  OrxCurAddchnstr
 *
 * Add a chtype character string to the screen.
 *
 * @param str     String to be added.
 *
 * @param n       Number of characters to add.
 *
 * @return        Zero.
 **/
RexxMethod3(int,                       // Return type
            OrxCurAddchnstr,           // Object_method name
            CSTRING, str,              // Character
            int, n,                    // Number of characters
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return waddchnstr((WINDOW *)cself, (const chtype *)str, n);
}

/**
 * Method:  OrxCurMvaddchnstr
 *
 * Add a chtype character string to the screen after moving the
 * cursor.
 *
 * @param y       Y position.
 *
 * @param x       X position.
 *
 * @param str     String to be added.
 *
 * @param n       Number of characters to add.
 *
 * @return        Zero.
 **/
RexxMethod5(int,                       // Return type
            OrxCurMvaddchnstr,         // Object_method name
            int, y,                    // Y position
            int, x,                    // X position
            CSTRING, str,              // Character
            int, n,                    // Number of characters
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return mvwaddchnstr((WINDOW *)cself, SUBTRACTONE(y), SUBTRACTONE(x), (const chtype *)str, n);
}

/**
 * Method:  OrxCurAddstr
 *
 * Add a character string to the screen.
 *
 * @param str     String to be added.
 *
 * @return        Zero.
 **/
RexxMethod2(int,                       // Return type
            OrxCurAddstr,              // Object_method name
            CSTRING, str,              // Character
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return waddstr((WINDOW *)cself, str);
}

/**
 * Method:  OrxCurMvaddstr
 *
 * Add a character string to the screen after moving the cursor.
 *
 * @param y       Y position.
 *
 * @param x       X position.
 *
 * @param str     String to be added.
 *
 * @return        Zero.
 **/
RexxMethod4(int,                       // Return type
            OrxCurMvaddstr,            // Object_method name
            int, y,                    // Y position
            int, x,                    // X position
            CSTRING, str,              // Character
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return mvwaddstr((WINDOW *)cself, SUBTRACTONE(y), SUBTRACTONE(x), str);
}

/**
 * Method:  OrxCurAddnstr
 *
 * Add a character string to the screen.
 *
 * @param str     String to be added.
 *
 * @param n       Number of characters to add.
 *
 * @return        Zero.
 **/
RexxMethod3(int,                       // Return type
            OrxCurAddnstr,             // Object_method name
            CSTRING, str,              // Character
            int, n,                    // Number of characters
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return waddnstr((WINDOW *)cself, str, n);
}

/**
 * Method:  OrxCurMvaddnstr
 *
 * Add a character string to the screen after moving the cursor.
 *
 * @param y       Y position.
 *
 * @param x       X position.
 *
 * @param str     String to be added.
 *
 * @param n       Number of characters to add.
 *
 * @return        Zero.
 **/
RexxMethod5(int,                       // Return type
            OrxCurMvaddnstr,           // Object_method name
            int, y,                    // Y position
            int, x,                    // X position
            CSTRING, str,              // Character
            int, n,                    // Number of characters
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return mvwaddnstr((WINDOW *)cself, SUBTRACTONE(y), SUBTRACTONE(x), str, n);
}

/**
 * Method:  OrxCurAssume_default_colors
 *
 * Set default colors.
 *
 * @param fg      Forground color.
 *
 * @param bg      Background color.
 *
 * @return        Zero.
 **/
RexxMethod2(int,                       // Return type
            OrxCurAssume_default_colors, // Object_method name
            int, fg,                   // fg color
            int, bg)                   // bg color
{

    return assume_default_colors(fg, bg);
}

/**
 * Method:  OrxCurAttroff
 *
 * Turn off one or more attributes.
 *
 * @param attr    Attribute(s).
 *
 * @return        Zero.
 **/
RexxMethod2(int,                       // Return type
            OrxCurAttroff,             // Object_method name
            int, attr,                 // Attribute(s)
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return wattroff((WINDOW *)cself, attr);
}

/**
 * Method:  OrxCurAttron
 *
 * Turn on one or more attributes.
 *
 * @param attr    Attribute(s).
 *
 * @return        Zero.
 **/
RexxMethod2(int,                       // Return type
            OrxCurAttron,              // Object_method name
            int, attr,                 // Attribute(s)
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return wattron((WINDOW *)cself, attr);
}

/**
 * Method:  OrxCurAttrset
 *
 * Turn on one or more attributes.
 *
 * @param attr    Attribute(s).
 *
 * @return        Zero.
 **/
RexxMethod2(int,                       // Return type
            OrxCurAttrset,             // Object_method name
            int, attr,                 // Attribute(s)
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return wattrset((WINDOW *)cself, attr);
}

/**
 * Method:  OrxCurBaudrate
 *
 * Return the terminal baud rate.
 *
 * @return        Zero.
 **/
RexxMethod0(int,                       // Return type
            OrxCurBaudrate)            // Object_method name
{

    return baudrate();
}

/**
 * Method:  OrxCurBeep
 *
 * Beep the terminal.
 *
 * @return        Zero.
 **/
RexxMethod0(int,                       // Return type
            OrxCurBeep)                // Object_method name
{

    return beep();
}

/**
 * Method:  OrxCurBkgd
 *
 * Set backgroung attributes for the whole screen.
 *
 * @param attr    Attribute(s).
 *
 * @return        Zero.
 **/
RexxMethod2(int,                       // Return type
            OrxCurBkgd,                // Object_method name
            int, attr,             // Attribute(s)
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return wbkgd((WINDOW *)cself, (chtype)attr);
}

/**
 * Method:  OrxCurBkgdset
 *
 * Set backgroung attributes for the next output text.
 *
 * @param attr    Attribute(s).
 *
 * @return        Zero.
 **/
RexxMethod2(int,                       // Return type
            OrxCurBkgdset,             // Object_method name
            int, attr,                 // Attribute(s)
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    wbkgdset((WINDOW *)cself, (chtype)attr);
    return 0;
}

/**
 * Method:  OrxCurBorder
 *
 * Set window border.
 *
 * @param ls      Left side character.
 *
 * @param rs      Right side character.
 *
 * @param ts      Top side character.
 *
 * @param bs      Bottom side character.
 *
 * @param tl      Top/Left character.
 *
 * @param tr      Top/Right character.
 *
 * @param bl      Bottom/Left character.
 *
 * @param br      Bottom/Right character.
 *
 * @return        Zero.
 **/
RexxMethod9(int,                       // Return type
            OrxCurBorder,              // Object_method name
            int, ls,
            int, rs,
            int, ts,
            int, bs,
            int, tl,
            int, tr,
            int, bl,
            int, br,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return wborder((WINDOW *)cself, (chtype)ls, (chtype)rs, (chtype)ts, (chtype)bs,
                   (chtype)tl, (chtype)tr, (chtype)bl, (chtype)br);
}

/**
 * Method:  OrxCurAcs_map
 *
 * Return an ACS character.
 *
 * @param str     Character.
 *
 * @return        Zero.
 **/
RexxMethod1(int,                       // Return type
            OrxCurAcs_map,              // Object_method name
            CSTRING, str)
{

    return (int) NCURSES_ACS(*str);
}

/**
 * Method:  OrxCurBox
 *
 * Draw a box around the edges of a window.
 *
 * @param str     Vertical character.
 *
 * @param str     Horizontal character.
 *
 * @return        Zero.
 **/
RexxMethod3(int,                       // Return type
            OrxCurBox,                 // Object_method name
            int, verch,
            int, horch,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return box((WINDOW *)cself, (chtype)verch, (chtype)horch);
}

/**
 * Method:  OrxCurCan_change_color
 *
 * Returns whether or not a terminal can changes its color set.
 *
 * @return        Zero.
 **/
RexxMethod0(int,                       // Return type
            OrxCurCan_change_color)    // Object_method name
{

    return (int) can_change_color();
}

/**
 * Method:  OrxCurCbreak
 *
 * Activates cbreak (buffering) mode.
 *
 * @return        Zero.
 **/
RexxMethod0(int,                       // Return type
            OrxCurCbreak)              // Object_method name
{

    return cbreak();
}

/**
 * Method:  OrxCurNocbreak
 *
 * Deactivates cbreak (buffering) mode.
 *
 * @return        Zero.
 **/
RexxMethod0(int,                       // Return type
            OrxCurNocbreak)            // Object_method name
{

    return nocbreak();
}

/**
 * Method:  OrxCurChgat
 *
 * Chage attributes on the window.
 *
 * @param n       Number of character positions.
 *
 * @param attr    The attribute.
 *
 * @param color   The COLOR_PAIR number.
 *
 * @return        Zero.
 **/
RexxMethod4(int,                       // Return type
            OrxCurChgat,               // Object_method name
            int, n,
            int, attr,
            int, color,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return wchgat((WINDOW *)cself, n, (attr_t)attr, (short)SUBTRACTONE(color), NULL);
}

/**
 * Method:  OrxCurMvchgat
 *
 * Chage attributes on the window after moving the cursor.
 *
 * @param y       Y start position.
 *
 * @param x       X start position.
 *
 * @param n       Number of character positions.
 *
 * @param attr    The attribute.
 *
 * @param color   The COLOR_PAIR number.
 *
 * @return        Zero.
 **/
RexxMethod6(int,                       // Return type
            OrxCurMvchgat,             // Object_method name
            int, y,
            int, x,
            int, n,
            int, attr,
            int, color,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return mvwchgat((WINDOW *)cself, SUBTRACTONE(y), SUBTRACTONE(x), n, (attr_t)attr,
                    (short)SUBTRACTONE(color), NULL);
}

/**
 * Method:  OrxCurClear
 *
 * Clear the window.
 *
 * @return        Zero.
 **/
RexxMethod1(int,                       // Return type
            OrxCurClear,               // Object_method name
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return wclear((WINDOW *)cself);
}

/**
 * Method:  OrxCurClearok
 *
 * Force a repaint of the window on the next refresh call.
 *
 * @param bf      Boolean on/off
 *
 * @return        Zero.
 **/
RexxMethod2(int,                       // Return type
            OrxCurClearok,             // Object_method name
            logical_t, bf,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return clearok((WINDOW *)cself, bf);
}

/**
 * Method:  OrxCurClrtobot
 *
 * Clear the window from the current cursor position.
 *
 * @return        Zero.
 **/
RexxMethod1(int,                       // Return type
            OrxCurClrtobot,            // Object_method name
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return wclrtobot((WINDOW *)cself);
}

/**
 * Method:  OrxCurClrtoeol
 *
 * Clear the line from the current cursor position.
 *
 * @return        Zero.
 **/
RexxMethod1(int,                       // Return type
            OrxCurClrtoeol,            // Object_method name
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return wclrtoeol((WINDOW *)cself);
}

/**
 * Method:  OrxCurColor_set
 *
 * Sets foreground and background text color.
 *
 * @param num     COLOR_PAIR number.
 *
 * @return        Zero.
 **/
RexxMethod1(int,                       // Return type
            OrxCurColor_set,           // Object_method name
            int, num)
{

    // don't use the SUBTRACTONE macro on num!
    return color_set((short)num, NULL);
}

/**
 * Method:  OrxCurColors
 *
 * Returns the number of possible COLORs.
 *
 * @return        Zero.
 **/
RexxMethod0(int,                       // Return type
            OrxCurColors)              // Object_method name
{

    return (int) COLORS;
}

/**
 * Method:  OrxCurColor_pair
 *
 * Returns the number of possible COLOR_PAIR attribute.
 *
 * @return        Zero.
 **/
RexxMethod1(int,                       // Return type
            OrxCurColor_pair,          // Object_method name
            int, num)
{

    return (int) COLOR_PAIR(num);
}

/**
 * Method:  OrxCurColor_pairs
 *
 * Returns the number of possible COLOR_PAIRS.
 *
 * @return        Zero.
 **/
RexxMethod0(int,                       // Return type
            OrxCurColor_pairs)         // Object_method name
{

    return (int) COLOR_PAIRS;
}

/**
 * Method:  OrxCurCols
 *
 * Returns the number of columns on the stdscr.
 *
 * @return        Zero.
 **/
RexxMethod0(int,                       // Return type
            OrxCurCols)                // Object_method name
{

    return (int) COLS;
}

/**
 * Method:  OrxCurCopywin
 *
 * Copy a rectangle from one window to self.
 *
 * @param swin    Source window.
 *
 * @param sminrow Source min row number.
 *
 * @param smincol Source min col number.
 *
 * @param dminrow Destination min row number.
 *
 * @param dmincol Destination min col number.
 *
 * @param dmaxrow Destination max row number.
 *
 * @param dmaxcol Destination max col number.
 *
 * @param overlay Boolean yes/no to overlay destination text.
 *
 * @return        Zero.
 **/
RexxMethod9(int,                       // Return type
            OrxCurCopywin,             // Object_method name
            RexxObjectPtr, swin,
            int, sminrow,
            int, smincol,
            int, dminrow,
            int, dmincol,
            int, dmaxrow,
            int, dmaxcol,
            logical_t, overlay,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return copywin((WINDOW *)context->ObjectToCSelf(swin), (WINDOW *)cself,
                   SUBTRACTONE(sminrow), SUBTRACTONE(smincol), SUBTRACTONE(dminrow),
                   SUBTRACTONE(dmincol), SUBTRACTONE(dmaxrow), SUBTRACTONE(dmaxcol),
                   (int)overlay);
}

/**
 * Method:  OrxCurCurs_set
 *
 * Control the cursor visibility.
 *
 * @param vis     Visibility boolean.
 *
 * @return        Zero.
 **/
RexxMethod1(int,                       // Return type
            OrxCurCurs_set,            // Object_method name
            int, vis)
{

    return curs_set(vis);
}

/**
 * Method:  OrxCurCurses_version
 *
 * Return the nCurses version string.
 *
 * @return        Zero.
 **/
RexxMethod0(RexxObjectPtr,             // Return type
            OrxCurCurses_version)      // Object_method name
{

    return (RexxObjectPtr)context->NewStringFromAsciiz(curses_version());
}

/**
 * Method:  OrxCurDelch
 *
 * Delete the character under the cursor and slide remaining
 * characters on the line one position to the left.
 *
 * @return        Zero.
 **/
RexxMethod1(int,                       // Return type
            OrxCurDelch,               // Object_method name
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return wdelch((WINDOW *)cself);
}

/**
 * Method:  OrxCurDeleteln
 *
 * Delete the line under the cursor and slide remaining lines
 * below the cursor up one line.
 *
 * @return        Zero.
 **/
RexxMethod1(int,                       // Return type
            OrxCurDeleteln,            // Object_method name
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return wdeleteln((WINDOW *)cself);
}

/**
 * Method:  OrxCurDelwin
 *
 * Destroy a window.
 *
 * @return        Zero.
 **/
RexxMethod1(int,                       // Return type
            OrxCurDelwin,              // Object_method name
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    int retc = delwin((WINDOW *)cself);
    if (retc == OK) {
        context->SetObjectVariable("CSELF", context->NewPointer(NULL));
    }
    return retc;
}

/**
 * Method:  OrxCurDerwinprivate
 *
 * Create a  derrived window.
 *
 * @param nlines  Number of lines.
 *
 * @param ncols   Number of cols.
 *
 * @param begy    Beginning Y line.
 *
 * @param begx    Beginning X column.
 *
 * @return        Zero.
 **/
RexxMethod5(RexxObjectPtr,             // Return type
            OrxCurDerwinprivate,       // Object_method name
            int, nlines,
            int, ncols,
            int, begy,
            int, begx,
            CSELF, cself)              // Self
{

    WINDOW *ptr = derwin((WINDOW *)cself, nlines, ncols, SUBTRACTONE(begy), SUBTRACTONE(begx));
    return (RexxObjectPtr)context->NewPointer(ptr);
}

/**
 * Method:  OrxCurDupwinprivate
 *
 * Duplicate a window.
 *
 * @return        Zero.
 **/
RexxMethod1(RexxObjectPtr,             // Return type
            OrxCurDupwinprivate,       // Object_method name
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    WINDOW *newwin = dupwin((WINDOW *)cself);
    return (RexxObjectPtr)context->NewPointer(newwin);
}

/**
 * Method:  OrxCurDoupdate
 *
 * Update the terminal.
 *
 * @return        Zero.
 **/
RexxMethod0(int,                       // Return type
            OrxCurDoupdate)            // Object_method name
{

    return doupdate();
}

/**
 * Method:  OrxCurEcho
 *
 * Turn on echo.
 *
 * @return        Zero.
 **/
RexxMethod0(int,                       // Return type
            OrxCurEcho)                // Object_method name
{

    return echo();
}

/**
 * Method:  OrxCurNoecho
 *
 * Turn off echo.
 *
 * @return        Zero.
 **/
RexxMethod0(int,                       // Return type
            OrxCurNoecho)              // Object_method name
{

    return noecho();
}

/**
 * Method:  OrxCurEchochar
 *
 * Echo one character.
 *
 * @return        Zero.
 **/
RexxMethod2(int,                       // Return type
            OrxCurEchochar,            // Object_method name
            int, ch,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return wechochar((WINDOW *)cself, (chtype)ch);
}

/**
 * Method:  OrxCurErase
 *
 * Erase the window.
 *
 * @return        Zero.
 **/
RexxMethod1(int,                       // Return type
            OrxCurErase,               // Object_method name
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return werase((WINDOW *)cself);
}

/**
 * Method:  OrxCurErasechar
 *
 * Return the terminal's erase char.
 *
 * @return        Zero.
 **/
RexxMethod0(RexxObjectPtr,             // Return type
            OrxCurErasechar)           // Object_method name
{
    char er[2] = {'\0', '\0'};

    er[0] = erasechar();
    return (RexxObjectPtr)context->NewStringFromAsciiz(er);
}

/**
 * Method:  OrxCurFilter
 *
 * Restrict output to a single line.
 *
 * @return        Zero.
 **/
RexxMethod0(int,                       // Return type
            OrxCurFilter)              // Object_method name
{

    filter();
    return 0;
}

/**
 * Method:  OrxCurFlash
 *
 * Briefly flash the screen.
 *
 * @return        Zero.
 **/
RexxMethod0(int,                       // Return type
            OrxCurFlash)               // Object_method name
{

    return flash();
}

/**
 * Method:  OrxCurFlushinp
 *
 * Flush the input queue.
 *
 * @return        Zero.
 **/
RexxMethod0(int,                       // Return type
            OrxCurFlushinp)            // Object_method name
{

    return flushinp();
}

/**
 * Method:  OrxCurGetbegyx
 *
 * Get the y & x screen coordinate for the top left corner of
 * the window.
 *
 * @return        Zero.
 **/
RexxMethod1(RexxObjectPtr,             // Return type
            OrxCurGetbegyx,            // Object_method name
            CSELF, cself)              // Self
{
    char buf[32];
    int y, x;

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    getbegyx((WINDOW *)cself, y, x);
    sprintf(buf, "%d %d", ADDONE(y), ADDONE(x));
    return (RexxObjectPtr)context->NewStringFromAsciiz(buf);
}

/**
 * Method:  OrxCurGetbkgd
 *
 * Get the background attribute for the the window.
 *
 * @return        Zero.
 **/
RexxMethod1(int,                  // Return type
            OrxCurGetbkgd,             // Object_method name
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return getbkgd((WINDOW *)cself);
}

/**
 * Method:  OrxCurGetch
 *
 * Get character from the keyboard.
 *
 * @return        Zero.
 **/
RexxMethod1(RexxObjectPtr,             // Return type
            OrxCurGetch,               // Object_method name
            CSELF, cself)              // Self
{
    char buf[2] = {'\0', '\0'};
    int retc;

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    retc = wgetch((WINDOW *)cself);
    if (retc < 0 || retc > 255) {  // return ERR and key codes as numeric strings
        return (RexxObjectPtr)context->Int32ToObject(retc);
    }
    buf[0] = (char)retc;
    return (RexxObjectPtr)context->NewStringFromAsciiz(buf);
}

/**
 * Method:  OrxCurMvgetch
 *
 * Get character from the keyboard after moving the cursor.
 *
 * @return        Zero.
 **/
RexxMethod3(RexxObjectPtr,             // Return type
            OrxCurMvgetch,             // Object_method name
            int, y,
            int, x,
            CSELF, cself)              // Self
{
    char buf[2] = {'\0', '\0'};
    int retc;

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    retc = mvwgetch((WINDOW *)cself, SUBTRACTONE(y), SUBTRACTONE(x));
    if (retc < 0 || retc > 255) {  // return ERR and key codes as numeric strings
        return (RexxObjectPtr)context->Int32ToObject(retc);
    }
    buf[0] = (char)retc;
    return (RexxObjectPtr)context->NewStringFromAsciiz(buf);
}

/**
 * Method:  OrxCurGetmaxyx
 *
 * Get the width and height of a window.
 *
 * @return        Zero.
 **/
RexxMethod1(RexxObjectPtr,             // Return type
            OrxCurGetmaxyx,            // Object_method name
            CSELF, cself)              // Self
{
    char buf[32];
    int y, x;

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    getmaxyx((WINDOW *)cself, y, x);
    sprintf(buf, "%d %d", y, x);
    return (RexxObjectPtr)context->NewStringFromAsciiz(buf);
}

/**
 * Method:  OrxCurGetparyx
 *
 * Get the width and height of a window relative to the parent
 * window.
 *
 * @return        Zero.
 **/
RexxMethod1(RexxObjectPtr,             // Return type
            OrxCurGetparyx,            // Object_method name
            CSELF, cself)              // Self
{
    char buf[32];
    int y, x;

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    getparyx((WINDOW *)cself, y, x);
    sprintf(buf, "%d %d", ADDONE(y), ADDONE(x));
    return (RexxObjectPtr)context->NewStringFromAsciiz(buf);
}

/**
 * Method:  OrxCurGetmouseprivate
 *
 * Get a mouse event.
 *
 * @param ev      Mouse event object.
 *
 * @return        Zero.
 **/
RexxMethod2(int,                       // Return type
            OrxCurGetmouseprivate,     // Object_method name
            RexxObjectPtr, ev,
            CSELF, cself)              // Self
{
    MEVENT mevent;

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    int retc = getmouse(&mevent);
    context->SendMessage1(ev, "id=", context->Int32ToObject((int32_t)mevent.id));
    context->SendMessage1(ev, "x=", context->Int32ToObject(ADDONE(mevent.x)));
    context->SendMessage1(ev, "y=", context->Int32ToObject(ADDONE(mevent.y)));
    context->SendMessage1(ev, "z=", context->Int32ToObject(ADDONE(mevent.z)));
    context->SendMessage1(ev, "bstate=", context->UnsignedInt32ToObject(mevent.bstate));
    return retc;
}

/**
 * Method:  OrxCurGetstr
 *
 * Get a string from the terminal.
 *
 * @return        Zero.
 **/
RexxMethod1(RexxObjectPtr,             // Return type
            OrxCurGetstr,              // Object_method name
            CSELF, cself)              // Self
{
    char buf[1024];

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    wgetnstr((WINDOW *)cself, buf, sizeof(buf) - 1);
    return context->NewStringFromAsciiz(buf);
}

/**
 * Method:  OrxCurGetnstr
 *
 * Get a string from the terminal.
 *
 * @param n       Number of characters to get.
 *
 * @return        Zero.
 **/
RexxMethod2(RexxObjectPtr,             // Return type
            OrxCurGetnstr,             // Object_method name
            int, n,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    char *buf = (char *)malloc(n + 1);
    wgetnstr((WINDOW *)cself, buf, n);
    RexxObjectPtr tmp = context->NewStringFromAsciiz(buf);
    free(buf);
    return tmp;
}

/**
 * Method:  OrxCurMvgetstr
 *
 * Get a string from the terminal after moving the cursor.
 *
 * @param y       New Y positions for the cursor.
 *
 * @param x       New X positions for the cursor.
 *
 * @return        Zero.
 **/
RexxMethod3(RexxObjectPtr,             // Return type
            OrxCurMvgetstr,            // Object_method name
            int, y,
            int, x,
            CSELF, cself)              // Self
{
    char buf[1024];

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    mvwgetnstr((WINDOW *)cself, SUBTRACTONE(y), SUBTRACTONE(x), buf, 1024);
    return context->NewStringFromAsciiz(buf);
}

/**
 * Method:  OrxCurGetnstr
 *
 * Get a string from the terminal after moving the cursor.
 *
 * @param y       New Y positions for the cursor.
 *
 * @param x       New X positions for the cursor.
 *
 * @param n       Number of characters to get.
 *
 * @return        Zero.
 **/
RexxMethod4(RexxObjectPtr,             // Return type
            OrxCurMvgetnstr,           // Object_method name
            int, y,
            int, x,
            int, n,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    char *buf = (char *)malloc(n + 1);
    mvwgetnstr((WINDOW *)cself, SUBTRACTONE(y), SUBTRACTONE(x), buf, n);
    RexxObjectPtr tmp = context->NewStringFromAsciiz(buf);
    free(buf);
    return tmp;
}

/**
 * Method:  OrxCurGetwinprivate
 *
 * Create a new window from a file.
 *
 * @param filename File name containing the window information.
 *
 * @return        Zero.
 **/
RexxMethod1(RexxObjectPtr,             // Return type
            OrxCurGetwinprivate,       // Object_method name
            CSTRING, filename)
{
    FILE *wfile;
    WINDOW *win;

    wfile = fopen(filename, "r");
    win = getwin(wfile);
    fclose(wfile);
    return (RexxObjectPtr)context->NewPointer(win);
}

/**
 * Method:  OrxCurGetyx
 *
 * Get the cursor location from a window.
 *
 * @return        Zero.
 **/
RexxMethod1(RexxObjectPtr,             // Return type
            OrxCurGetyx,               // Object_method name
            CSELF, cself)              // Self
{
    char buf[32];
    int y, x;

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    getyx((WINDOW *)cself, y, x);
    sprintf(buf, "%d %d", ADDONE(y), ADDONE(x));
    return (RexxObjectPtr)context->NewStringFromAsciiz(buf);
}

/**
 * Method:  OrxCurHalfdelay
 *
 * Simalar to the cbreak method.
 *
 * @param tenths  Tenths of a second.
 *
 * @return        Zero.
 **/
RexxMethod1(int,                       // Return type
            OrxCurHalfdelay,           // Object_method name
            int, tenths)
{

    return halfdelay(tenths);
}

/**
 * Method:  OrxCurHas_colors
 *
 * Determine if the terminal has color support.
 *
 * @return        Zero.
 **/
RexxMethod0(logical_t,                 // Return type
            OrxCurHas_colors)          // Object_method name
{

    return has_colors();
}

/**
 * Method:  OrxCurHas_ic
 *
 * Determine if the terminal has insert character support.
 *
 * @return        Zero.
 **/
RexxMethod0(logical_t,                 // Return type
            OrxCurHas_ic)              // Object_method name
{

    return has_ic();
}

/**
 * Method:  OrxCurHas_il
 *
 * Determine if the terminal has insert line support.
 *
 * @return        Zero.
 **/
RexxMethod0(logical_t,                 // Return type
            OrxCurHas_il)              // Object_method name
{

    return has_il();
}

/**
 * Method:  OrxCurHline
 *
 * Draw a horizontal line.
 *
 * @param ch      Character type (chtype)
 *
 * @param n       Number of characters to draw.
 *
 * @return        Zero.
 **/
RexxMethod2(int,                       // Return type
            OrxCurHline,               // Object_method name
            int, ch,
            int, n)
{

    return hline((chtype)ch, n);
}

/**
 * Method:  OrxCurMvhline
 *
 * Draw a horizontal line.
 *
 * @param y       New Y cursor position
 *
 * @param x       New C cursor position
 *
 * @param ch      Character type (chtype)
 *
 * @param n       Number of characters to draw.
 *
 * @return        Zero.
 **/
RexxMethod4(int,                       // Return type
            OrxCurMvhline,             // Object_method name
            int, y,
            int, x,
            int, ch,
            int, n)
{

    return mvhline(SUBTRACTONE(y), SUBTRACTONE(x), (chtype)ch, n);
}

/**
 * Method:  OrxCurIdcok
 *
 * Use hardware to insert/delete characters.
 *
 * @param bf      Boolean on/off.
 *
 * @return        Zero.
 **/
RexxMethod2(int,                       // Return type
            OrxCurIdcok,               // Object_method name
            logical_t, bf,
            CSELF, cself)              // Self
{

    idcok((WINDOW *)cself, bf);
    return 0;
}

/**
 * Method:  OrxCurIdlok
 *
 * Use hardware to insert/delete lines.
 *
 * @param bf      Boolean on/off.
 *
 * @return        Zero.
 **/
RexxMethod2(int,                       // Return type
            OrxCurIdlok,               // Object_method name
            logical_t, bf,
            CSELF, cself)              // Self
{

    idlok((WINDOW *)cself, bf);
    return 0;
}

/**
 * Method:  OrxCurImmedok
 *
 * Use hardware to provide auto refresh.
 *
 * @param bf      Boolean on/off.
 *
 * @return        Zero.
 **/
RexxMethod2(int,                       // Return type
            OrxCurImmedok,             // Object_method name
            logical_t, bf,
            CSELF, cself)              // Self
{

    immedok((WINDOW *)cself, bf);
    return 0;
}

/**
 * Method:  OrxCurInch
 *
 * Return the attr/char under the cursor.
 *
 * @return        Zero.
 **/
RexxMethod1(int,                  // Return type
            OrxCurInch,                // Object_method name
            CSELF, cself)              // Self
{

    return (int)winch((WINDOW *)cself);
}

/**
 * Method:  OrxCurMvinch
 *
 * Return the attr/char under the cursor after moving the
 * cursor.
 *
 * @param y       New cursor y position.
 *
 * @param x       New cursor x position.
 *
 * @return        Zero.
 **/
RexxMethod3(int,                  // Return type
            OrxCurMvinch,              // Object_method name
            int, y,
            int, x,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return (int)mvwinch((WINDOW *)cself, SUBTRACTONE(y), SUBTRACTONE(x));
}

/**
 * Method:  OrxCurInchstr
 *
 * Return the attr/char array.
 *
 * @return        Zero.
 **/
RexxMethod1(RexxObjectPtr,             // Return type
            OrxCurInchstr,             // Object_method name
            CSELF, cself)              // Self
{
    chtype *buf;
    RexxBufferStringObject rxbuf;

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    rxbuf = context->NewBufferString(1024 * sizeof(chtype));
    buf = (chtype *)context->BufferStringData(rxbuf);
    winchnstr((WINDOW *)cself, buf, 1024);
    return (RexxObjectPtr)rxbuf;
}

/**
 * Method:  OrxCurMvinchstr
 *
 * Return the attr/char under the cursor after moving the
 * cursor.
 *
 * @param y       New cursor y position.
 *
 * @param x       New cursor x position.
 *
 * @return        Zero.
 **/
RexxMethod3(RexxObjectPtr,             // Return type
            OrxCurMvinchstr,           // Object_method name
            int, y,
            int, x,
            CSELF, cself)              // Self
{
    chtype *buf;
    RexxBufferStringObject rxbuf;

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    rxbuf = context->NewBufferString(1024 * sizeof(chtype));
    buf = (chtype *)context->BufferStringData(rxbuf);
    mvwinchnstr((WINDOW *)cself, SUBTRACTONE(y), SUBTRACTONE(x), buf, 1024);
    return (RexxObjectPtr)rxbuf;
}

/**
 * Method:  OrxCurInchnstr
 *
 * Return the attr/char array.
 *
 * @param n       Number of attr/char to return.
 *
 * @return        Zero.
 **/
RexxMethod2(RexxObjectPtr,             // Return type
            OrxCurInchnstr,            // Object_method name
            int, n,
            CSELF, cself)              // Self
{
    chtype *buf;
    RexxBufferStringObject rxbuf;

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    rxbuf = context->NewBufferString(n * sizeof(chtype));
    buf = (chtype *)context->BufferStringData(rxbuf);
    winchnstr((WINDOW *)cself, buf, n);
    return (RexxObjectPtr)rxbuf;
}

/**
 * Method:  OrxCurMvinchnstr
 *
 * Return the attr/char under the cursor after moving the
 * cursor.
 *
 * @param y       New cursor y position.
 *
 * @param x       New cursor x position.
 *
 * @param n       Number of attr/char to return.
 *
 * @return        Zero.
 **/
RexxMethod4(RexxObjectPtr,             // Return type
            OrxCurMvinchnstr,          // Object_method name
            int, y,
            int, x,
            int, n,
            CSELF, cself)              // Self
{
    chtype *buf;
    RexxBufferStringObject rxbuf;

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    rxbuf = context->NewBufferString(n * sizeof(chtype));
    buf = (chtype *)context->BufferStringData(rxbuf);
    mvwinchnstr((WINDOW *)cself, SUBTRACTONE(y), SUBTRACTONE(x), buf, n);
    return (RexxObjectPtr)rxbuf;
}

/**
 * Method:  OrxCurInit_color
 *
 * Allow the user to redefine colors.
 *
 * @param c       Color number to change.
 *
 * @param r       Red value.
 *
 * @param g       Green value.
 *
 * @param b       Blue value.
 *
 * @return        Zero.
 **/
RexxMethod4(int,                       // Return type
            OrxCurInit_color,          // Object_method name
            int, c,
            int, r,
            int, g,
            int, b)
{

    return init_color((short)c, (short)r, (short)g, (short)b);
}

/**
 * Method:  OrxCurInit_pair
 *
 * Assign foreground an background colors to a color pair.
 *
 * @param c       COLOR_PAIR number to change.
 *
 * @param f       Foreground color.
 *
 * @param b       Background color.
 *
 * @return        Zero.
 **/
RexxMethod3(int,                       // Return type
            OrxCurInit_pair,           // Object_method name
            int, c,
            int, f,
            int, b)
{

    return init_pair((short)c, (short)f, (short)b);
}

/**
 * Method:  OrxCurInsch
 *
 * Insert attr/char under the cursor.
 *
 * @param ch      chtype to be inserted.
 *
 * @return        Zero.
 **/
RexxMethod2(int,                       // Return type
            OrxCurInsch,               // Object_method name
            CSTRING, ch,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return (int)winsch((WINDOW *)cself, (chtype)*ch);
}

/**
 * Method:  OrxCurMvinsch
 *
 * Insert the attr/char under the cursor after moving the
 * cursor.
 *
 * @param y       New cursor y position.
 *
 * @param x       New cursor x position.
 *
 * @param ch      chtype to be inserted.
 *
 * @return        Zero.
 **/
RexxMethod4(int,                       // Return type
            OrxCurMvinsch,             // Object_method name
            int, y,
            int, x,
            CSTRING, ch,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return mvwinsch((WINDOW *)cself, SUBTRACTONE(y), SUBTRACTONE(x), (chtype)*ch);
}

/**
 * Method:  OrxCurInsdelln
 *
 * Insert/delete lines.
 *
 * @param n       Number of lines to insert/delete.
 *
 * @return        Zero.
 **/
RexxMethod2(int,                       // Return type
            OrxCurInsdelln,            // Object_method name
            int, n,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return winsdelln((WINDOW *)cself, SUBTRACTONE(n));
}

/**
 * Method:  OrxCurInsertln
 *
 * Insert one line.
 *
 * @return        Zero.
 **/
RexxMethod1(int,                       // Return type
            OrxCurInsertln,            // Object_method name
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return winsertln((WINDOW *)cself);
}

/**
 * Method:  OrxCurInsstr
 *
 * Insert a string.
 *
 * @param str     String to insert.
 *
 * @return        Zero.
 **/
RexxMethod2(int,                       // Return type
            OrxCurInsstr,              // Object_method name
            CSTRING, str,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return winsstr((WINDOW *)cself, str);
}

/**
 * Method:  OrxCurInsnstr
 *
 * Insert a string.
 *
 * @param str     String to insert.
 *
 * @param n       Number of characters to insert.
 *
 * @return        Zero.
 **/
RexxMethod3(int,                       // Return type
            OrxCurInsnstr,             // Object_method name
            CSTRING, str,
            int, n,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return winsnstr((WINDOW *)cself, str, n);
}

/**
 * Method:  OrxCurMvinsstr
 *
 * Insert a string after moving the cursor.
 *
 * @param y       New cursor y position.
 *
 * @param x       New cursor x position.
 *
 * @param str     String to insert.
 *
 * @return        Zero.
 **/
RexxMethod4(int,                       // Return type
            OrxCurMvinsstr,            // Object_method name
            int, y,
            int, x,
            CSTRING, str,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return mvwinsstr((WINDOW *)cself, SUBTRACTONE(y), SUBTRACTONE(x), str);
}

/**
 * Method:  OrxCurInsnstr
 *
 * Insert a string.
 *
 * @param y       New cursor y position.
 *
 * @param x       New cursor x position.
 *
 * @param str     String to insert.
 *
 * @param n       Number of characters to insert.
 *
 * @return        Zero.
 **/
RexxMethod5(int,                       // Return type
            OrxCurMvinsnstr,           // Object_method name
            int, y,
            int, x,
            CSTRING, str,
            int, n,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return mvwinsnstr((WINDOW *)cself, SUBTRACTONE(y), SUBTRACTONE(x), str, n);
}

/**
 * Method:  OrxCurInstr
 *
 * Read a string from the terminal.
 *
 * @return        Zero.
 **/
RexxMethod1(RexxObjectPtr,             // Return type
            OrxCurInstr,               // Object_method name
            CSELF, cself)              // Self
{
    char buf[1024];

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    winstr((WINDOW *)cself, buf);
    return context->NewStringFromAsciiz(buf);
}

/**
 * Method:  OrxCurInnstr
 *
 * Read a string from the terminal.
 *
 * @param n       Number of characters to read.
 *
 * @return        Zero.
 **/
RexxMethod2(RexxObjectPtr,             // Return type
            OrxCurInnstr,              // Object_method name
            int, n,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    char *buf = (char *)malloc(n + 1);
    winnstr((WINDOW *)cself, buf, n);
    RexxObjectPtr tmp = context->NewStringFromAsciiz(buf);
    free(buf);
    return tmp;
}

/**
 * Method:  OrxCurMvinstr

 * Read a string from the terminal after moving the cursor.
 *
 * @param y       New y position for the cursor.
 *
 * @param x       New x position for the cursor.
 *
 * @return        Zero.
 **/
RexxMethod3(RexxObjectPtr,             // Return type
            OrxCurMvinstr,             // Object_method name
            int, y,
            int, x,
            CSELF, cself)              // Self
{
    char buf[1024];

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    mvwinstr((WINDOW *)cself, SUBTRACTONE(y), SUBTRACTONE(x), buf);
    return context->NewStringFromAsciiz(buf);
}

/**
 * Method:  OrxCurMvinnstr
 *
 * Read a string from the terminal after moving the cursor.
 *
 * @param y       New y position for the cursor.
 *
 * @param x       New x position for the cursor.
 *
 * @param n       Number of characters to read.
 *
 * @return        Zero.
 **/
RexxMethod4(RexxObjectPtr,             // Return type
            OrxCurMvinnstr,            // Object_method name
            int, y,
            int, x,
            int, n,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    char *buf = (char *)malloc(n + 1);
    mvwinnstr((WINDOW *)cself, SUBTRACTONE(y), SUBTRACTONE(x), buf, n);
    RexxObjectPtr tmp = context->NewStringFromAsciiz(buf);
    free(buf);
    return tmp;
}

/**
 * Method:  OrxCurIntrflush
 *
 * Turn on/off input queue flushing when a interrupt key is
 * typed at the keyboard.
 *
 * @param bf      Boolean
 *
 * @return        Zero.
 **/
RexxMethod2(int,                       // Return type
            OrxCurIntrflush,           // Object_method name
            logical_t, bf,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return intrflush((WINDOW *)cself, bf);
}

/**
 * Method:  OrxCurIsbitset
 *
 * Return true if a bit is set.
 *
 * @param field   Field value to test.
 *
 * @param bit     The bit to test.
 *
 * @return        Zero.
 **/
RexxMethod2(logical_t,                 // Return type
            OrxCurIsbitset,            // Object_method name
            int, field,
            int, bit)
{

    return (field & bit);
}

/**
 * Method:  OrxCurIsendwin
 *
 * Return true if endwin() has been called.
 *
 * @return        Zero.
 **/
RexxMethod0(logical_t,                 // Return type
            OrxCurIsendwin)            // Object_method name
{

    return isendwin();
}

/**
 * Method:  OrxCurIs_linetouched
 *
 * Determine if a line has been changed since the last refresh.
 *
 * @param n       Line number.
 *
 * @return        Zero.
 **/
RexxMethod2(logical_t,                 // Return type
            OrxCurIs_linetouched,      // Object_method name
            int, n,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return is_linetouched((WINDOW *)cself, SUBTRACTONE(n));
}

/**
 * Method:  OrxCurIs_wintouched
 *
 * Determine if a window has been changed since the last
 * refresh.
 *
 * @return        Zero.
 **/
RexxMethod1(logical_t,                 // Return type
            OrxCurIs_wintouched,       // Object_method name
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return is_wintouched((WINDOW *)cself);
}

/**
 * Method:  OrxCurKeyname
 *
 * Return a string representing the specified input key.
 *
 * @param k       Key value (decimal number).
 *
 * @return        Zero.
 **/
RexxMethod1(RexxObjectPtr,             // Return type
            OrxCurKeyname,             // Object_method name
            int, k)
{

    return (RexxObjectPtr)context->NewStringFromAsciiz(keyname(k));
}

/**
 * Method:  OrxCurKeypad
 *
 * Turn on/off reading keypad (function, etc) keys.
 *
 * @param bf      Boolean.
 *
 * @return        Zero.
 **/
RexxMethod2(int,                       // Return type
            OrxCurKeypad,              // Object_method name
            logical_t, bf,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return keypad((WINDOW *)cself, bf);
}

/**
 * Method:  OrxCurKillchar
 *
 * Return the kill terminal's kill character.
 *
 * @return        Zero.
 **/
RexxMethod0(RexxObjectPtr,             // Return type
            OrxCurKillchar)            // Object_method name
{
    char buf[2] = {'\0', '\0'};

    buf[0] = killchar();
    return (RexxObjectPtr)context->NewStringFromAsciiz(buf);
}

/**
 * Method:  OrxCurLeaveok
 *
 * Turn on/off synchrnizing the logical and the hardware cursor
 * location after a refresh.
 *
 * @param bf      Boolean.
 *
 * @return        Zero.
 **/
RexxMethod2(int,                       // Return type
            OrxCurLeaveok,             // Object_method name
            logical_t, bf,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return leaveok((WINDOW *)cself, bf);
}

/**
 * Method:  OrxCurLines
 *
 * Return the number of lines on the stdscr.
 *
 * @return        Zero.
 **/
RexxMethod0(int,                       // Return type
            OrxCurLines)               // Object_method name
{

    return LINES;
}

/**
 * Method:  OrxCurLongname
 *
 * Return the terminal string description.
 *
 * @return        Zero.
 **/
RexxMethod0(RexxObjectPtr,             // Return type
            OrxCurLongname)            // Object_method name
{

    return (RexxObjectPtr)context->NewStringFromAsciiz(longname());
}

/**
 * Method:  OrxCurMeta
 *
 * Turn on/off reading the Alt key info in the 8th bit.
 *
 * @param bf      Boolean.
 *
 * @return        Zero.
 **/
RexxMethod2(int,                       // Return type
            OrxCurMeta,                // Object_method name
            logical_t, bf,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return meta((WINDOW *)cself, bf);
}

/**
 * Method:  OrxCurMouse_trafo
 *
 * Translate mouse y and x coordinates.
 *
 * @param y       Y mouse coordinate.
 *
 * @param x       X mouse coordinate.
 *
 * @param bf      Boolean.
 *
 * @return        Zero.
 **/
RexxMethod4(RexxObjectPtr,             // Return type
            OrxCurMouse_trafo,         // Object_method name
            int, y,
            int, x,
            logical_t, bf,
            CSELF, cself)              // Self
{
    char buf[32];

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    wmouse_trafo((WINDOW *)cself, &y, &x, bf);
    sprintf(buf, "%d %d", ADDONE(y), ADDONE(x));
    return (RexxObjectPtr)context->NewStringFromAsciiz(buf);
}

/**
 * Method:  OrxCurMousemask
 *
 * Set/return the mouse event mask.
 *
 * @param old     Old mouse mask ou 0.
 *
 * @return        Zero.
 **/
RexxMethod1(int,                  // Return type
            OrxCurMousemask,           // Object_method name
            int, newmask)
{

    return mousemask(newmask, NULL);
}

/**
 * Method:  OrxCurMove
 *
 * Move the cursor.
 *
 * @param y       New Y cursor position.
 *
 * @param x       New X cursor position.
 *
 * @return        Zero.
 **/
RexxMethod3(int,                       // Return type
            OrxCurMove,                // Object_method name
            int, y,
            int, x,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return wmove((WINDOW *)cself, SUBTRACTONE(y), SUBTRACTONE(x));
}

/**
 * Method:  OrxCurMvderwin
 *
 * Display a different portion of the parent window.
 *
 * @param y       Parent Y position.
 *
 * @param x       Parent X position.
 *
 * @return        Zero.
 **/
RexxMethod3(int,                       // Return type
            OrxCurMvderwin,            // Object_method name
            int, y,
            int, x,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return mvderwin((WINDOW *)cself, SUBTRACTONE(y), SUBTRACTONE(x));
}

/**
 * Method:  OrxCurMvwin
 *
 * Move a window.
 *
 * @param y       Parent Y position.
 *
 * @param x       Parent X position.
 *
 * @return        Zero.
 **/
RexxMethod3(int,                       // Return type
            OrxCurMvwin,               // Object_method name
            int, y,
            int, x,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return mvwin((WINDOW *)cself, SUBTRACTONE(y), SUBTRACTONE(x));
}

/**
 * Method:  OrxCurNapms
 *
 * Pause the program for number of microseconds.
 *
 * @param n       Number of microseconds
 *
 * @return        Zero.
 **/
RexxMethod1(int,                       // Return type
            OrxCurNapms,               // Object_method name
            int, n)
{

    return napms(n);
}

/**
 * Method:  OrxCurNcurses_mouse_version
 *
 * Return the Ncurses mouse version string.
 *
 * @return        Zero.
 **/
RexxMethod0(RexxObjectPtr,             // Return type
            OrxCurNcurses_mouse_version) // Object_method name
{
    char buf[64];

    sprintf(buf, "%d", NCURSES_MOUSE_VERSION);
    return (RexxObjectPtr)context->NewStringFromAsciiz(buf);
}

/**
 * Method:  OrxCurNcurses_version
 *
 * Return the Ncurses version string.
 *
 * @return        Zero.
 **/
RexxMethod0(RexxObjectPtr,             // Return type
            OrxCurNcurses_version)     // Object_method name
{

    return (RexxObjectPtr)context->NewStringFromAsciiz(NCURSES_VERSION);
}

/**
 * Method:  OrxCurNewpad
 *
 * Create a new pad.
 *
 * @param nlines  Number of lines.
 *
 * @param ncols   Number of columns.
 *
 * @return        Zero.
 **/
RexxMethod2(int,                       // Return type
            OrxCurNewpad,              // Object_method name
            int, nlines,
            int, ncols)
{

    context->SetObjectVariable("CSELF", context->NewPointer(newpad(nlines, ncols)));
    return 0;
}

/**
 * Method:  OrxCurNl
 *
 * Distinguish between cr and lf.
 *
 * @return        Zero.
 **/
RexxMethod0(int,                       // Return type
            OrxCurNl)                  // Object_method name
{

    return nl();
}

/**
 * Method:  OrxCurNonl
 *
 * Do not distinguish between cr and lf.
 *
 * @return        Zero.
 **/
RexxMethod0(int,                       // Return type
            OrxCurNonl)                // Object_method name
{

    return nonl();
}

/**
 * Method:  OrxCurNodelay
 *
 * Transforms getch() function to nonblocking.
 *
 * @param bf      On/off boolean.
 *
 * @return        Zero.
 **/
RexxMethod2(int,                       // Return type
            OrxCurNodelay,             // Object_method name
            logical_t, bf,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return nodelay((WINDOW *)cself, bf);
}

/**
 * Method:  OrxCurNotimeout
 *
 * Pause input function after user presses ESC key.
 *
 * @param bf      On/off boolean.
 *
 * @return        Zero.
 **/
RexxMethod2(int,                       // Return type
            OrxCurNotimeout,           // Object_method name
            logical_t, bf,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return notimeout((WINDOW *)cself, bf);
}

/**
 * Method:  OrxCurOverlay
 *
 * Overlay the destination window.
 *
 * @param dwin    Destination window.
 *
 * @return        Zero.
 **/
RexxMethod2(int,                       // Return type
            OrxCurOverlay,             // Object_method name
            RexxObjectPtr, rxdwin,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    WINDOW *dwin = (WINDOW *)context->ObjectToCSelf(rxdwin);
    return overlay((WINDOW *)cself, dwin);
}

/**
 * Method:  OrxCurOverwrite
 *
 * Overlay the destination window.
 *
 * @param dwin    Destination window.
 *
 * @return        Zero.
 **/
RexxMethod2(int,                       // Return type
            OrxCurOverwrite,           // Object_method name
            RexxObjectPtr, rxdwin,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    WINDOW *dwin = (WINDOW *)context->ObjectToCSelf(rxdwin);
    return overwrite((WINDOW *)cself, dwin);
}

/**
 * Method:  OrxCurPair_content
 *
 * Return the pair of colors in a color pair.
 *
 * @param num     Color pair number.
 *
 * @return        Zero.
 **/
RexxMethod1(RexxObjectPtr,             // Return type
            OrxCurPair_content,        // Object_method name
            int, num)
{
    char buf[64];
    short f, b;

    int retc = pair_content((short)num, &f, &b);
    sprintf(buf, "%d %d", f, b);
    return (RexxObjectPtr)context->NewStringFromAsciiz(buf);
}

/**
 * Method:  OrxCurPair_number
 *
 * Return the default pair attr.
 *
 * @param attr    Color pair attribute.
 *
 * @return        Zero.
 **/
RexxMethod1(int,                  // Return type
            OrxCurPair_number,         // Object_method name
            int, num)
{

    return PAIR_NUMBER(num);
}

/**
 * Method:  OrxCurPechochar
 *
 * Echo one character to a pad and the stdscr..
 *
 * @param ch      Character to echo.
 *
 * @return        Zero.
 **/
RexxMethod2(int,                       // Return type
            OrxCurPechochar,           // Object_method name
            CSTRING, ch,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Pad"));
        return 0;
    }
    return pechochar((WINDOW *)cself, (chtype)*ch);
}

/**
 * Method:  OrxCurPnoutrefresh
 *
 * Refresh a pad.
 *
 * @param minrow  Minimum row number.
 *
 * @param mincol  Minimum col number.
 *
 * @param sminrow Minimum row number.
 *
 * @param smincol Minimum col number.
 *
 * @param smaxrow Maximum row number.
 *
 * @param smaxcol Maximum col number.
 *
 * @return        Zero.
 **/
RexxMethod7(int,                       // Return type
            OrxCurPnoutrefresh,        // Object_method name
            int, minrow,
            int, mincol,
            int, sminrow,
            int, smincol,
            int, smaxrow,
            int, smaxcol,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Pad"));
        return 0;
    }
    return pnoutrefresh((WINDOW *)cself, SUBTRACTONE(minrow), SUBTRACTONE(mincol),
                        SUBTRACTONE(sminrow), SUBTRACTONE(smincol),
                        SUBTRACTONE(smaxrow), SUBTRACTONE(smaxcol));
}

/**
 * Method:  OrxCurPrefresh
 *
 * Refresh the pad to the stdscr.
 *
 * @param minrow  Minimum row number.
 *
 * @param mincol  Minimum col number.
 *
 * @param sminrow Minimum row number.
 *
 * @param smincol Minimum col number.
 *
 * @param smaxrow Maximum row number.
 *
 * @param smaxcol Maximum col number.
 *
 * @return        Zero.
 **/
RexxMethod7(int,                       // Return type
            OrxCurPrefresh,            // Object_method name
            int, minrow,
            int, mincol,
            int, sminrow,
            int, smincol,
            int, smaxrow,
            int, smaxcol,
            CSELF, cself)              // Pad
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Pad"));
        return 0;
    }
    return prefresh((WINDOW *)cself, SUBTRACTONE(minrow), SUBTRACTONE(mincol),
                    SUBTRACTONE(sminrow), SUBTRACTONE(smincol),
                    SUBTRACTONE(smaxrow), SUBTRACTONE(smaxcol));
}

/**
 * Method:  OrxCurPutwin
 *
 * Save a new window to a file.
 *
 * @param fname   File name.
 *
 * @return        Zero.
 **/
RexxMethod2(int,                       // Return type
            OrxCurPutwin,              // Object_method name
            CSTRING, filename,
            CSELF, cself)              // Self
{
    FILE *wfile;

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    wfile = fopen(filename, "w");
    int retc = putwin((WINDOW *)cself, wfile);
    fclose(wfile);
    return retc;
}

/**
 * Method:  OrxCurQiflush
 *
 * Flush the input queue on interrupt.
 *
 * @return        Zero.
 **/
RexxMethod0(int,                       // Return type
            OrxCurQiflush)             // Object_method name
{

    qiflush();
    return 0;
}

/**
 * Method:  OrxCurNoqiflush
 *
 * Do no flush the input queue on interrupt.
 *
 * @return        Zero.
 **/
RexxMethod0(int,                       // Return type
            OrxCurNoqiflush)           // Object_method name
{

    noqiflush();
    return 0;
}

/**
 * Method:  OrxCurRaw
 *
 * Remove modification done by the terminal to input.
 *
 * @return        Zero.
 **/
RexxMethod0(int,                       // Return type
            OrxCurRaw)                 // Object_method name
{

    return raw();
}

/**
 * Method:  OrxCurNoraw
 *
 * Do not remove modification done by the terminal to input.
 *
 * @return        Zero.
 **/
RexxMethod0(int,                       // Return type
            OrxCurNoraw)               // Object_method name
{

    return noraw();
}

/**
 * Method:  OrxCurRedrawwin
 *
 * Redraw an entire window.
 *
 * @return        Zero.
 **/
RexxMethod1(int,                       // Return type
            OrxCurRedrawwin,           // Object_method name
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return redrawwin((WINDOW *)cself);
}

/**
 * Method:  OrxCurScr_dump
 *
 * Dump the terminal screen to a file.
 *
 * @param fname   Output file name.
 *
 * @return        Zero.
 **/
RexxMethod1(int,                       // Return type
            OrxCurScr_dump,            // Object_method name
            CSTRING, fname)
{

    return scr_dump(fname);
}

/**
 * Method:  OrxCurScr_restore
 *
 * Restore the terminal screen drom a file.
 *
 * @param fname   Input file name.
 *
 * @return        Zero.
 **/
RexxMethod1(int,                       // Return type
            OrxCurScr_restore,         // Object_method name
            CSTRING, fname)
{

    return scr_restore(fname);
}

/**
 * Method:  OrxCurScrl
 *
 * Scroll a window.
 *
 * @param num     Number of lines to scroll.
 *
 * @return        Zero.
 **/
RexxMethod2(int,                       // Return type
            OrxCurScrl,                // Object_method name
            int, n,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return wscrl((WINDOW *)cself, n);
}

/**
 * Method:  OrxCurScroll
 *
 * Scroll a window.
 *
 * @return        Zero.
 **/
RexxMethod1(int,                       // Return type
            OrxCurScroll,              // Object_method name
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return scroll((WINDOW *)cself);
}

/**
 * Method:  OrxCurScrollok
 *
 * Allaow a wildow to scroll.
 *
 * @param bf      Boolean indicator.
 *
 * @return        Zero.
 **/
RexxMethod2(int,                       // Return type
            OrxCurScrollok,            // Object_method name
            logical_t, bf,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return scrollok((WINDOW *)cself, bf);
}

/**
 * Method:  OrxCurSetscrreg
 *
 * Set a window reagion scrollable.
 *
 * @param top     Top row.
 *
 * @param bot     Bottom row.
 *
 * @return        Zero.
 **/
RexxMethod3(int,                       // Return type
            OrxCurSetscrreg,           // Object_method name
            int, top,
            int, bot,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return wsetscrreg((WINDOW *)cself, SUBTRACTONE(top), SUBTRACTONE(bot));
}

/**
 * Method:  OrxCurSlk_attr
 *
 * Return soft label attr_t.
 *
 * @return        Zero.
 **/
RexxMethod0(int,                  // Return type
            OrxCurSlk_attr)            // Object_method name
{

    return slk_attr();
}

/**
 * Method:  OrxCurSlk_attroff
 *
 * Turn off one or more soft label attributes.
 *
 * @param attr    Attribute(s).
 *
 * @return        Zero.
 **/
RexxMethod1(int,                       // Return type
            OrxCurSlk_attroff,         // Object_method name
            int, attr)
{

    return slk_attroff(attr);
}

/**
 * Method:  OrxCurSlk_attron
 *
 * Turn on one or more soft label attributes.
 *
 * @param attr    Attribute(s).
 *
 * @return        Zero.
 **/
RexxMethod1(int,                       // Return type
            OrxCurSlk_attron,          // Object_method name
            int, attr)
{

    return slk_attron(attr);
}

/**
 * Method:  OrxCurSlk_attrset
 *
 * Set soft label attributes.
 *
 * @param attr    Attribute(s).
 *
 * @return        Zero.
 **/
RexxMethod1(int,                       // Return type
            OrxCurSlk_attrset,         // Object_method name
            int, attr)
{

    return slk_attrset(attr);
}

/**
 * Method:  OrxCurSlk_clear
 *
 * Hide the soft labels.
 *
 * @return        Zero.
 **/
RexxMethod0(int,                       // Return type
            OrxCurSlk_clear)           // Object_method name
{

    return slk_clear();
}

/**
 * Method:  OrxCurSlk_color
 *
 * Set soft label colors.
 *
 * @param color   Color pair number.
 *
 * @return        Zero.
 **/
RexxMethod1(int,                       // Return type
            OrxCurSlk_color,           // Object_method name
            int, color)
{

    return slk_color((short)color);
}

/**
 * Method:  OrxCurSlk_init
 *
 * Initialize for soft label.
 *
 * @param fmt     Line format.
 *
 * @return        Zero.
 **/
RexxMethod1(int,                       // Return type
            OrxCurSlk_init,            // Object_method name
            int, fmt)
{

    return slk_init(fmt);
}

/**
 * Method:  OrxCurSlk_label
 *
 * Return the soft label text.
 *
 * @param num     Number of the soft label.
 *
 * @return        Zero.
 **/
RexxMethod1(RexxObjectPtr,             // Return type
            OrxCurSlk_label,           // Object_method name
            int, num)
{

    return (RexxObjectPtr)context->NewStringFromAsciiz(slk_label(num));
}

/**
 * Method:  OrxCurSlk_noutrefresh
 *
 * Prepares soft labels for a refresh.
 *
 * @return        Zero.
 **/
RexxMethod0(int,                       // Return type
            OrxCurSlk_noutrefresh)     // Object_method name
{

    return slk_noutrefresh();
}

/**
 * Method:  OrxCurSlk_refresh
 *
 * Refresh soft labels.
 *
 * @return        Zero.
 **/
RexxMethod0(int,                       // Return type
            OrxCurSlk_refresh)         // Object_method name
{

    return slk_refresh();
}

/**
 * Method:  OrxCurSlk_restore
 *
 * Restore soft labels.
 *
 * @return        Zero.
 **/
RexxMethod0(int,                       // Return type
            OrxCurSlk_restore)         // Object_method name
{

    return slk_restore();
}

/**
 * Method:  OrxCurSlk_set
 *
 * Set soft label text.
 *
 * @param text    Soft label text.
 *
 * @param num     Soft label number.
 *
 * @return        Zero.
 **/
RexxMethod3(int,                       // Return type
            OrxCurSlk_set,             // Object_method name
            int, num,
            CSTRING, text,
            int, fmt)
{

    return slk_set(num, text, fmt);
}

/**
 * Method:  OrxCurSlk_touch
 *
 * Touch soft labels.
 *
 * @return        Zero.
 **/
RexxMethod0(int,                       // Return type
            OrxCurSlk_touch)           // Object_method name
{

    return slk_touch();
}

/**
 * Method:  OrxCurStandend
 *
 * Turn off text attributes.
 *
 * @return        Zero.
 **/
RexxMethod1(int,                       // Return type
            OrxCurStandend,            // Object_method name
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return wstandend((WINDOW *)cself);
}

/**
 * Method:  OrxCurStandout
 *
 * Turn on text attributes.
 *
 * @return        Zero.
 **/
RexxMethod1(int,                       // Return type
            OrxCurStandout,            // Object_method name
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return wstandout((WINDOW *)cself);
}

/**
 * Method:  OrxCurStart_color
 *
 * Turn on using color attributes.
 *
 * @return        Zero.
 **/
RexxMethod0(int,                       // Return type
            OrxCurStart_color)         // Object_method name
{

    return start_color();
}

/**
 * Method:  OrxCurSubpadprivate
 *
 * Create a subpad.
 *
 * @param nlines  Number of lines.
 *
 * @param ncols   Number of cols.
 *
 * @param begy    Beginning Y line.
 *
 * @param begx    Beginning X column.
 *
 * @return        Zero.
 **/
RexxMethod6(int,                       // Return type
            OrxCurSubpadprivate,       // Object_method name
            RexxObjectPtr, pad,
            int, nlines,
            int, ncols,
            int, begy,
            int, begx,
            CSELF, cself)              // Self
{

    WINDOW *padptr = (WINDOW *) context->ObjectToCSelf(pad);
    WINDOW *ptr = subpad((WINDOW *)padptr, nlines, ncols, SUBTRACTONE(begy), SUBTRACTONE(begx));
    context->SetObjectVariable("CSELF", context->NewPointer(ptr));
    return 0;
}

/**
 * Method:  OrxCurSubwinprivate
 *
 * Create a subwindow.
 *
 * @param nlines  Number of lines.
 *
 * @param ncols   Number of cols.
 *
 * @param begy    Beginning Y line.
 *
 * @param begx    Beginning X column.
 *
 * @return        Zero.
 **/
RexxMethod5(RexxObjectPtr,             // Return type
            OrxCurSubwinprivate,       // Object_method name
            int, nlines,
            int, ncols,
            int, begy,
            int, begx,
            CSELF, cself)              // Self
{

    WINDOW *ptr = subwin((WINDOW *)cself, nlines, ncols, SUBTRACTONE(begy), SUBTRACTONE(begx));
    return (RexxObjectPtr)context->NewPointer(ptr);
}

/**
 * Method:  OrxCurSyncok
 *
 * Turn on/off aut touch for a window.
 *
 * @param bf      On/off boolean.
 *
 * @return        Zero.
 **/
RexxMethod2(int,                       // Return type
            OrxCurSyncok,              // Object_method name
            logical_t, bf,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return syncok((WINDOW *)cself, bf);
}

/**
 * Method:  OrxCurTabsize
 *
 * Return/set the tab size.
 *
 * @param sz      New tab size.
 *
 * @return        Zero.
 **/
RexxMethod1(int,                       // Return type
            OrxCurTabsize,             // Object_method name
            OPTIONAL_int, sz)
{
    if (argumentExists(1)) {
        set_tabsize(sz);
    }
    return TABSIZE;
}

/**
 * Method:  OrxCurTermattrs
 *
 * Return the which attributes the terminal is capable of
 * producing.
 *
 * @return        Zero.
 **/
RexxMethod0(int,                       // Return type
            OrxCurTermattrs)           // Object_method name
{

    return (int) termattrs();
}

/**
 * Method:  OrxCurtermname
 *
 * Return the terminal name.
 *
 * @return        Zero.
 **/
RexxMethod0(RexxObjectPtr,             // Return type
            OrxCurTermname)            // Object_method name
{

    return (RexxObjectPtr)context->NewStringFromAsciiz(termname());
}

/**
 * Method:  OrxCurTimeout
 *
 * Set the timeout for text input.
 *
 * @param tmout   Timeout value.
 *
 * @return        Zero.
 **/
RexxMethod1(int,                       // Return type
            OrxCurTimeout,             // Object_method name
            int, tmout)
{

    timeout(tmout);
    return 0;
}

/**
 * Method:  OrxCurTouchline
 *
 * Mark one or more lines as changed.
 *
 * @param bf      On/off boolean.
 *
 * @return        Zero.
 **/
RexxMethod3(int,                       // Return type
            OrxCurTouchline,           // Object_method name
            int, start,
            int, cnt,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return touchline((WINDOW *)cself, SUBTRACTONE(start), cnt);
}

/**
 * Method:  OrxCurTouchwin
 *
 * Mark window as changed.
 *
 * @return        Zero.
 **/
RexxMethod1(int,                       // Return type
            OrxCurTouchwin,            // Object_method name
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return touchwin((WINDOW *)cself);
}

/**
 * Method:  OrxCurTypeahead
 *
 * Set the typeahead for text input.
 *
 * @param fd      Typeahead value.
 *
 * @return        Zero.
 **/
RexxMethod1(int,                       // Return type
            OrxCurTypeahead,           // Object_method name
            int, fd)
{

    return typeahead(fd);
}

/**
 * Method:  OrxCurUnctrl
 *
 * Convert a control code to a ^code sequence.
 *
 * @param ch      Code to convert.
 *
 * @return        Zero.
 **/
RexxMethod1(RexxObjectPtr,             // Return type
            OrxCurUnctrl,              // Object_method name
            int, ch)
{

    return (RexxObjectPtr)context->NewStringFromAsciiz(unctrl((chtype)ch));
}

/**
 * Method:  OrxCurUngetch
 *
 * Put a character into the keyboard buffer.
 *
 * @param ch      Character to insert.
 *
 * @return        Zero.
 **/
RexxMethod1(int,                       // Return type
            OrxCurUngetch,             // Object_method name
            CSTRING, ch)
{

    return ungetch((int)*ch);
}

/**
 * Method:  OrxCurUntouchwin
 *
 * Unmark window as changed.
 *
 * @return        Zero.
 **/
RexxMethod1(int,                       // Return type
            OrxCurUntouchwin,          // Object_method name
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return untouchwin((WINDOW *)cself);
}

/**
 * Method:  OrxCurUse_default_colors
 *
 * Use the default terminal colors.
 *
 * @return        Zero.
 **/
RexxMethod0(int,                       // Return type
            OrxCurUse_default_colors)  // Object_method name
{

    return use_default_colors();
}

/**
 * Method:  OrxCurUse_env
 *
 * Use the default terminal colors.
 *
 * @return        Zero.
 **/
RexxMethod1(int,                       // Return type
            OrxCurUse_env,             // Object_method name
            logical_t, bf)
{

    use_env(bf);
    return 0;
}

/**
 * Method:  OrxCurVline
 *
 * Draw a vertical line.
 *
 * @param ch      chtype.
 *
 * @param n       Number of rows.
 *
 * @return        Zero.
 **/
RexxMethod3(int,                       // Return type
            OrxCurVline,               // Object_method name
            int, ch,
            int, n,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return wvline((WINDOW *)cself, (chtype)ch, n);
}

/**
 * Method:  OrxCurMvvline
 *
 * Draw a vertical line after moving the cursor.
 *
 * @param y       New Y cursor position.
 *
 * @param x       New X cursor position.
 *
 * @param ch      chtype.
 *
 * @param n       Number of rows.
 *
 * @return        Zero.
 **/
RexxMethod5(int,                       // Return type
            OrxCurMvvline,             // Object_method name
            int, y,
            int, x,
            int, ch,
            int, n,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return mvwvline((WINDOW *)cself, SUBTRACTONE(y), SUBTRACTONE(x), (chtype)ch, n);
}

/**
 * Method:  OrxCurWenclose
 *
 * Determine if a mouse click are within a window.
 *
 * @param y       Y cursor screen position.
 *
 * @param x       X cursor screen position.
 *
 * @return        Zero.
 **/
RexxMethod3(int,                       // Return type
            OrxCurWenclose,            // Object_method name
            int, y,
            int, x,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return wenclose((WINDOW *)cself, SUBTRACTONE(y), SUBTRACTONE(x));
}

/**
 * Method:  OrxCurWnoutrefresh
 *
 * Copy modified text to the screen.
 *
 * @return        Zero.
 **/
RexxMethod1(int,                       // Return type
            OrxCurWnoutrefresh,        // Object_method name
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return wnoutrefresh((WINDOW *)cself);
}

/**
 * Method:  OrxCurWredrawln
 *
 * Redraw a line to the screen.
 *
 * @param beg     Beginning line number.
 *
 * @param n       Number of lines.
 *
 * @return        Zero.
 **/
RexxMethod3(int,                       // Return type
            OrxCurWredrawln,           // Object_method name
            int, beg,
            int, n,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return wredrawln((WINDOW *)cself, SUBTRACTONE(beg), n);
}

/**
 * Method:  OrxCurWsyncdown
 *
 * Ensure subwindows are touched when the main window is
 * touched.
 *
 * @return        Zero.
 **/
RexxMethod1(int,                       // Return type
            OrxCurWsyncdown,           // Object_method name
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    wsyncdown((WINDOW *)cself);
    return 0;
}

/**
 * Method:  OrxCurWsyncup
 *
 * Ensure parent windows are touched when the sub window is
 * touched.
 *
 * @return        Zero.
 **/
RexxMethod1(int,                       // Return type
            OrxCurWsyncup,             // Object_method name
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    wsyncup((WINDOW *)cself);
    return 0;
}

/**
 * Method:  OrxCurWtouchln
 *
 * Mark one or more lines as changed.
 *
 * @param y       Start line number.
 *
 * @param n       Number of lines.
 *
 * @param bf      Changed/Unchanged boolean.
 *
 * @return        Zero.
 **/
RexxMethod4(int,                       // Return type
            OrxCurWtouchln,            // Object_method name
            int, y,
            int, n,
            logical_t, bf,
            CSELF, cself)              // Self
{

    if (cself == NULL) {
        context->RaiseException2(Rexx_Error_Incorrect_method_noclass,
                                 context->WholeNumberToObject(1),
                                 context->NewStringFromAsciiz("Window"));
        return 0;
    }
    return wtouchln((WINDOW *)cself, SUBTRACTONE(y), n, bf);
}


// build the actual function entry list
RexxRoutineEntry orxcur_routines[] = {
    REXX_LAST_ROUTINE()
};


// build the actual method entry list
RexxMethodEntry orxcur_methods[] = {
    REXX_METHOD(OrxCurVersion, OrxCurVersion),
    REXX_METHOD(OrxCurSetBase, OrxCurSetBase),
    REXX_METHOD(OrxCurInitscr, OrxCurInitscr),
    REXX_METHOD(OrxCurNewwin, OrxCurNewwin),
    REXX_METHOD(OrxCurNewwinfromptr, OrxCurNewwinfromptr),
    REXX_METHOD(OrxCurEndwin, OrxCurEndwin),
    REXX_METHOD(OrxCurRefresh, OrxCurRefresh),
    REXX_METHOD(OrxCurAddch, OrxCurAddch),
    REXX_METHOD(OrxCurMvaddch, OrxCurMvaddch),
    REXX_METHOD(OrxCurAddchstr, OrxCurAddchstr),
    REXX_METHOD(OrxCurMvaddchstr, OrxCurMvaddchstr),
    REXX_METHOD(OrxCurAddchnstr, OrxCurAddchnstr),
    REXX_METHOD(OrxCurMvaddchnstr, OrxCurMvaddchnstr),
    REXX_METHOD(OrxCurAddstr, OrxCurAddstr),
    REXX_METHOD(OrxCurMvaddstr, OrxCurMvaddstr),
    REXX_METHOD(OrxCurAddnstr, OrxCurAddnstr),
    REXX_METHOD(OrxCurMvaddnstr, OrxCurMvaddnstr),
    REXX_METHOD(OrxCurAssume_default_colors, OrxCurAssume_default_colors),
    REXX_METHOD(OrxCurAttroff, OrxCurAttroff),
    REXX_METHOD(OrxCurAttron, OrxCurAttron),
    REXX_METHOD(OrxCurAttrset, OrxCurAttrset),
    REXX_METHOD(OrxCurBaudrate, OrxCurBaudrate),
    REXX_METHOD(OrxCurBeep, OrxCurBeep),
    REXX_METHOD(OrxCurBkgd, OrxCurBkgd),
    REXX_METHOD(OrxCurBkgdset, OrxCurBkgdset),
    REXX_METHOD(OrxCurBorder, OrxCurBorder),
    REXX_METHOD(OrxCurAcs_map, OrxCurAcs_map),
    REXX_METHOD(OrxCurBox, OrxCurBox),
    REXX_METHOD(OrxCurCan_change_color, OrxCurCan_change_color),
    REXX_METHOD(OrxCurCbreak, OrxCurCbreak),
    REXX_METHOD(OrxCurNocbreak, OrxCurNocbreak),
    REXX_METHOD(OrxCurChgat, OrxCurChgat),
    REXX_METHOD(OrxCurMvchgat, OrxCurMvchgat),
    REXX_METHOD(OrxCurClear, OrxCurClear),
    REXX_METHOD(OrxCurClearok, OrxCurClearok),
    REXX_METHOD(OrxCurClrtobot, OrxCurClrtobot),
    REXX_METHOD(OrxCurClrtoeol, OrxCurClrtoeol),
    REXX_METHOD(OrxCurColor_set, OrxCurColor_set),
    REXX_METHOD(OrxCurColors, OrxCurColors),
    REXX_METHOD(OrxCurColor_pair, OrxCurColor_pair),
    REXX_METHOD(OrxCurColor_pairs, OrxCurColor_pairs),
    REXX_METHOD(OrxCurCols, OrxCurCols),
    REXX_METHOD(OrxCurCopywin, OrxCurCopywin),
    REXX_METHOD(OrxCurCurs_set, OrxCurCurs_set),
    REXX_METHOD(OrxCurCurses_version, OrxCurCurses_version),
    REXX_METHOD(OrxCurDelch, OrxCurDelch),
    REXX_METHOD(OrxCurDeleteln, OrxCurDeleteln),
    REXX_METHOD(OrxCurDelwin, OrxCurDelwin),
    REXX_METHOD(OrxCurDerwinprivate, OrxCurDerwinprivate),
    REXX_METHOD(OrxCurDupwinprivate, OrxCurDupwinprivate),
    REXX_METHOD(OrxCurDoupdate, OrxCurDoupdate),
    REXX_METHOD(OrxCurEcho, OrxCurEcho),
    REXX_METHOD(OrxCurNoecho, OrxCurNoecho),
    REXX_METHOD(OrxCurErase, OrxCurErase),
    REXX_METHOD(OrxCurErasechar, OrxCurErasechar),
    REXX_METHOD(OrxCurEchochar, OrxCurEchochar),
    REXX_METHOD(OrxCurFilter, OrxCurFilter),
    REXX_METHOD(OrxCurFlash, OrxCurFlash),
    REXX_METHOD(OrxCurFlushinp, OrxCurFlushinp),
    REXX_METHOD(OrxCurGetbegyx, OrxCurGetbegyx),
    REXX_METHOD(OrxCurGetbkgd, OrxCurGetbkgd),
    REXX_METHOD(OrxCurGetch, OrxCurGetch),
    REXX_METHOD(OrxCurMvgetch, OrxCurMvgetch),
    REXX_METHOD(OrxCurGetmaxyx, OrxCurGetmaxyx),
    REXX_METHOD(OrxCurGetparyx, OrxCurGetparyx),
    REXX_METHOD(OrxCurGetmouseprivate, OrxCurGetmouseprivate),
    REXX_METHOD(OrxCurGetstr, OrxCurGetstr),
    REXX_METHOD(OrxCurGetnstr, OrxCurGetnstr),
    REXX_METHOD(OrxCurMvgetstr, OrxCurMvgetstr),
    REXX_METHOD(OrxCurMvgetnstr, OrxCurMvgetnstr),
    REXX_METHOD(OrxCurGetwinprivate, OrxCurGetwinprivate),
    REXX_METHOD(OrxCurGetyx, OrxCurGetyx),
    REXX_METHOD(OrxCurHalfdelay, OrxCurHalfdelay),
    REXX_METHOD(OrxCurHas_colors, OrxCurHas_colors),
    REXX_METHOD(OrxCurHas_ic, OrxCurHas_ic),
    REXX_METHOD(OrxCurHas_il, OrxCurHas_il),
    REXX_METHOD(OrxCurHline, OrxCurHline),
    REXX_METHOD(OrxCurMvhline, OrxCurMvhline),
    REXX_METHOD(OrxCurIdcok, OrxCurIdcok),
    REXX_METHOD(OrxCurIdlok, OrxCurIdlok),
    REXX_METHOD(OrxCurImmedok, OrxCurImmedok),
    REXX_METHOD(OrxCurInch, OrxCurInch),
    REXX_METHOD(OrxCurMvinch, OrxCurMvinch),
    REXX_METHOD(OrxCurInchstr, OrxCurInchstr),
    REXX_METHOD(OrxCurMvinchstr, OrxCurMvinchstr),
    REXX_METHOD(OrxCurInchnstr, OrxCurInchstr),
    REXX_METHOD(OrxCurMvinchnstr, OrxCurMvinchnstr),
    REXX_METHOD(OrxCurInit_color, OrxCurInit_color),
    REXX_METHOD(OrxCurInit_pair, OrxCurInit_pair),
    REXX_METHOD(OrxCurInsch, OrxCurInsch),
    REXX_METHOD(OrxCurMvinsch, OrxCurMvinsch),
    REXX_METHOD(OrxCurInsdelln, OrxCurInsdelln),
    REXX_METHOD(OrxCurInsertln, OrxCurInsertln),
    REXX_METHOD(OrxCurInsstr, OrxCurInsstr),
    REXX_METHOD(OrxCurInsnstr, OrxCurInsnstr),
    REXX_METHOD(OrxCurMvinsstr, OrxCurMvinsstr),
    REXX_METHOD(OrxCurMvinsnstr, OrxCurMvinsnstr),
    REXX_METHOD(OrxCurInstr, OrxCurInstr),
    REXX_METHOD(OrxCurInnstr, OrxCurInnstr),
    REXX_METHOD(OrxCurMvinstr, OrxCurMvinstr),
    REXX_METHOD(OrxCurMvinnstr, OrxCurMvinnstr),
    REXX_METHOD(OrxCurIntrflush, OrxCurIntrflush),
    REXX_METHOD(OrxCurIsbitset, OrxCurIsbitset),
    REXX_METHOD(OrxCurIsendwin, OrxCurIsendwin),
    REXX_METHOD(OrxCurIs_linetouched, OrxCurIs_linetouched),
    REXX_METHOD(OrxCurIs_wintouched, OrxCurIs_wintouched),
    REXX_METHOD(OrxCurKeyname, OrxCurKeyname),
    REXX_METHOD(OrxCurKeypad, OrxCurKeypad),
    REXX_METHOD(OrxCurKillchar, OrxCurKillchar),
    REXX_METHOD(OrxCurLeaveok, OrxCurLeaveok),
    REXX_METHOD(OrxCurLines, OrxCurLines),
    REXX_METHOD(OrxCurLongname, OrxCurLongname),
    REXX_METHOD(OrxCurMeta, OrxCurMeta),
    REXX_METHOD(OrxCurMouse_trafo, OrxCurMouse_trafo),
    REXX_METHOD(OrxCurMousemask, OrxCurMousemask),
    REXX_METHOD(OrxCurMove, OrxCurMove),
    REXX_METHOD(OrxCurMvderwin, OrxCurMvderwin),
    REXX_METHOD(OrxCurMvwin, OrxCurMvwin),
    REXX_METHOD(OrxCurNapms, OrxCurNapms),
    REXX_METHOD(OrxCurNcurses_mouse_version, OrxCurNcurses_mouse_version),
    REXX_METHOD(OrxCurNcurses_version, OrxCurNcurses_version),
    REXX_METHOD(OrxCurNewpad, OrxCurNewpad),
    REXX_METHOD(OrxCurNl, OrxCurNl),
    REXX_METHOD(OrxCurNonl, OrxCurNonl),
    REXX_METHOD(OrxCurNodelay, OrxCurNodelay),
    REXX_METHOD(OrxCurNotimeout, OrxCurNotimeout),
    REXX_METHOD(OrxCurOverlay, OrxCurOverlay),
    REXX_METHOD(OrxCurOverwrite, OrxCurOverwrite),
    REXX_METHOD(OrxCurPair_content, OrxCurPair_content),
    REXX_METHOD(OrxCurPair_number, OrxCurPair_number),
    REXX_METHOD(OrxCurPechochar, OrxCurPechochar),
    REXX_METHOD(OrxCurPnoutrefresh, OrxCurPnoutrefresh),
    REXX_METHOD(OrxCurPrefresh, OrxCurPrefresh),
    REXX_METHOD(OrxCurPutwin, OrxCurPutwin),
    REXX_METHOD(OrxCurQiflush, OrxCurQiflush),
    REXX_METHOD(OrxCurNoqiflush, OrxCurNoqiflush),
    REXX_METHOD(OrxCurRaw, OrxCurRaw),
    REXX_METHOD(OrxCurNoraw, OrxCurNoraw),
    REXX_METHOD(OrxCurRedrawwin, OrxCurRedrawwin),
    REXX_METHOD(OrxCurScr_dump, OrxCurScr_dump),
    REXX_METHOD(OrxCurScr_restore, OrxCurScr_restore),
    REXX_METHOD(OrxCurScrl, OrxCurScrl),
    REXX_METHOD(OrxCurScroll, OrxCurScroll),
    REXX_METHOD(OrxCurScrollok, OrxCurScrollok),
    REXX_METHOD(OrxCurSetscrreg, OrxCurSetscrreg),
    REXX_METHOD(OrxCurSlk_attr, OrxCurSlk_attr),
    REXX_METHOD(OrxCurSlk_attroff, OrxCurSlk_attroff),
    REXX_METHOD(OrxCurSlk_attron, OrxCurSlk_attron),
    REXX_METHOD(OrxCurSlk_attrset, OrxCurSlk_attrset),
    REXX_METHOD(OrxCurSlk_clear, OrxCurSlk_clear),
    REXX_METHOD(OrxCurSlk_color, OrxCurSlk_color),
    REXX_METHOD(OrxCurSlk_init, OrxCurSlk_init),
    REXX_METHOD(OrxCurSlk_label, OrxCurSlk_label),
    REXX_METHOD(OrxCurSlk_noutrefresh, OrxCurSlk_noutrefresh),
    REXX_METHOD(OrxCurSlk_refresh, OrxCurSlk_refresh),
    REXX_METHOD(OrxCurSlk_restore, OrxCurSlk_restore),
    REXX_METHOD(OrxCurSlk_set, OrxCurSlk_set),
    REXX_METHOD(OrxCurSlk_touch, OrxCurSlk_touch),
    REXX_METHOD(OrxCurStandend, OrxCurStandend),
    REXX_METHOD(OrxCurStandout, OrxCurStandout),
    REXX_METHOD(OrxCurStart_color, OrxCurStart_color),
    REXX_METHOD(OrxCurSubwinprivate, OrxCurSubwinprivate),
    REXX_METHOD(OrxCurSubpadprivate, OrxCurSubpadprivate),
    REXX_METHOD(OrxCurSyncok, OrxCurSyncok),
    REXX_METHOD(OrxCurTabsize, OrxCurTabsize),
    REXX_METHOD(OrxCurTermattrs, OrxCurTermattrs),
    REXX_METHOD(OrxCurTermname, OrxCurTermname),
    REXX_METHOD(OrxCurTimeout, OrxCurTimeout),
    REXX_METHOD(OrxCurTouchline, OrxCurTouchline),
    REXX_METHOD(OrxCurTouchwin, OrxCurTouchwin),
    REXX_METHOD(OrxCurTypeahead, OrxCurTypeahead),
    REXX_METHOD(OrxCurUnctrl, OrxCurUnctrl),
    REXX_METHOD(OrxCurUngetch, OrxCurUngetch),
    REXX_METHOD(OrxCurUntouchwin, OrxCurUntouchwin),
    REXX_METHOD(OrxCurUse_default_colors, OrxCurUse_default_colors),
    REXX_METHOD(OrxCurUse_env, OrxCurUse_env),
    REXX_METHOD(OrxCurVline, OrxCurVline),
    REXX_METHOD(OrxCurMvvline, OrxCurMvvline),
    REXX_METHOD(OrxCurWenclose, OrxCurWenclose),
    REXX_METHOD(OrxCurWnoutrefresh, OrxCurWnoutrefresh),
    REXX_METHOD(OrxCurWredrawln, OrxCurWredrawln),
    REXX_METHOD(OrxCurWsyncdown, OrxCurWsyncdown),
    REXX_METHOD(OrxCurWsyncup, OrxCurWsyncup),
    REXX_METHOD(OrxCurWtouchln, OrxCurWtouchln),
    REXX_LAST_METHOD()
};


RexxPackageEntry orxcur_package_entry = {
    STANDARD_PACKAGE_HEADER
    REXX_INTERPRETER_4_0_0,              // anything after 4.0.0 will work
    "orxncurses",                        // name of the package
    VERSTRING(VMAJOR,VMINOR,VREL),       // package information
    NULL,                                // no load functions
    NULL,                                // no unload functions
    orxcur_routines,                     // the exported routines
    orxcur_methods                       // the exported methods
};

// package loading stub.
OOREXX_GET_PACKAGE(orxcur);

