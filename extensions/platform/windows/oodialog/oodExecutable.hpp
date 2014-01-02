/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2013-2014 Rexx Language Association. All rights reserved.    */
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

/**
 * oodExecutable.hpp
 *
 * This file contains defines and other data used in ooDialog.com, ooDialog.exe,
 * and the .rc file for those 2 executables.
 *
 * The first portion includes all IDs needed in the .rc scripts.  Following the
 * first portion is the more typical header stuff, struct definitions, et., used
 * in the C / C++ code.
 */

#ifndef oodExeuctable_Included
#define oodExeuctable_Included

#ifndef IDC_STATIC
#define IDC_STATIC (-1)
#endif

// ResEdit IDs
#define IDD_OODIALOG_DISPLAY                    102
#define IDD_FILEASSOC                           104
#define IDD_CONFIGURE_SERVICES                  106
#define IDI_APP_ICON                            107
#define IDI_APP_BLUE_BEAKER_ICON                108
#define IDI_UP_ARROW                            110
#define IDI_DOWN_ARROW                          112
#define IDI_DELETE                              114
#define IDI_DELETE_XX                           115
#define IDC_ST_SCOPE                            1000
#define IDC_GB_VIEW                             1001
#define IDC_GB_PATHEXT                          1002
#define IDC_ST_RUNAS                            1003
#define IDC_CK_FILEASSOC                        1004
#define IDC_ST_PATHEXT_DETAILS                  1005
#define IDC_ST_REGISTERED                       1006
#define IDC_ST_FTYPE                            1007
#define IDC_EDIT                                1008
#define IDC_RB_SINGLE_VIEW                      1009
#define IDC_ST_REGALL                           1010
#define IDC_ST_ELEVATED                         1011
#define IDC_PB_UP                               1012
#define IDC_PB_ADD_FA_EXT                       1013
#define IDC_ST_SHORT_MSG                        1014
#define IDC_GB_SERVICES                         1015
#define IDC_RB_DOUBLE_VIEW                      1016
#define IDC_PB_DOWN                             1017
#define IDC_ST_REGCURRENT                       1018
#define IDC_LB_SUGGESTED                        1019
#define IDC_LB_CURRENT                          1020
#define IDC_LB_PATHEXT                          1021
#define IDC_PB_ADD_CURRENT                      1022
#define IDC_PB_REMOVE_CURRENT                   1023
#define IDC_PB_ADD_PATHEXT                      1024
#define IDC_PB_REGISTER                         1025
#define IDC_PB_DEL                              1026
#define IDC_RB_CURRENT                          1027
#define IDC_RB_ALL                              1028
#define IDC_EDIT_FA_EXT                         1029
#define IDC_PB_CONFIGURE                        1030
#define IDC_ST_INADMINGROUP                     1031
#define IDC_ST_ISRUNASADMIN                     1032
#define IDC_ST_ISELEVATED                       1033
#define IDC_ST_IL                               1034
#define IDC_GB_ASSOCIATE                        1035
#define IDC_CK_UPDATE                           1036
#define IDC_ST_DISPLAY_ICON                     1037
#define IDC_PB_DELXX                            1038
#define IDS_FRIENDLY_NAME                       40000
#define IDC_EDIT_PE_EXT                         40001
#define IDS_INFOTIP                             40002
#define IDC_PB_ADD_PE_EXT                       40003


// Eveything else is not passed to the resource compiler:
#ifndef RC_INVOKED

#define PATHEXT_DETAILS              "PATHEXT is only changed when Update is checked and Done is clicked. "
                                     /*
                                     " Use the Up / Down buttons to change the order of the extensions; the "  \
                                     "X button to delete an extension. To prevent an error, the common "       \
                                     "extensions, .com, .exe, .bat, and .cmd, can not be deleted."
                                     */

#define OS_ERR_TITLE                 "ooDialog Execute Program: Windows Error"
#define OOD_ERR_TITLE                "ooDialog Execute Program: Internal Error"
#define USER_ERR_TITLE               "ooDialog Execute Program: User Error"
#define REG_ERR_TITLE                "ooDialog Execute Program: Registry Error"

#define PATHEXT_UPDATE_FAILED        "An internal error is preventing any change to the PATHEXT variable."
#define UNICODE_CONVERSION_ERR       "Conversion from Unicode to ANSI, or memory\nallocation, failed."
#define OUT_OF_MEMORY_ERR_FMT        "Failed to allocate memory:\n\nFunction:\t\t%s\nError Code:\t%d"
#define OS_PARSING_ERR_FMT           "Operating system parsing of the command line failed.\n\nLast reported error code: %d\n"
#define EMPTY_STRING_ARG_ERR_FMT     "Argument %d is the empty string (\"\").\n\nThis is not allowed.\n"
#define REQUIRED_PATHEXT_ERR_FMT     "To prevent your computer from becoming unusable, the %s extension can not be removed from your PATHEXT"
#define NOT_IN_SCOPE_PATHEXT_ERR_FMT "The %s extension is not in the \"%s\" PATHEXT. This list box item is for information only and will not be deleted."

#define OODIALOG_PROGID              "ooRexx.ooDialog"
#define OODIALOG_PROGID_VERSIONED    "ooRexx.ooDialog.1"
#define OODIALOG_FRIENDLY_NAME       "ooDialog Program"
#define OODIALOG_DROP_HANDLER        "{60254CA5-953B-11CF-8C96-00AA00B8708C}"

// Buffer size for progID. We only currently need 18, but we add extra for
// testing now and possible future changes.
#define MAX_PROGID                   32
#define MAX_FRIENDLY_NAME            64

#define MAX_EXT_NAME   32                  // Length of an extension, includes the dot
#define MAX_EXT_DISPLAY  MAX_EXT_NAME + 4  // Add a prefix, max 4 chars, to the extension name
#define MAX_HKEY_NAME  255                 // Length for a subkey name buffer
#define MAX_HKEY_VALUE 16383               // Length for a subky value buffer

inline void safeLocalFree(void *p)
{
    if ( p != NULL )
    {
        LocalFree(p);
    }
}

static char *oodSuggestedExts[] =
{
    ".rxd",
    ".ood",
    ".ooDlg",
    ".rxooDlg",
};

#define OOD_SUGGESTED_EXT_COUNT      (sizeof(oodSuggestedExts) / sizeof(char *))

static char *requiredPathExts[] =
{
    ".COM",
    ".EXE",
    ".BAT",
    ".CMD",
};

#define REQUIRED_PATH_EXT_COUNT      (sizeof(requiredPathExts) / sizeof(char *))

// We make these arrays a little big and hopefully we won't need to realloc them.
#define MIN_SUGGESTED_ARRAY_SIZE      12
#define MIN_CURRENT_ARRAY_SIZE        12
#define MIN_ALLUSERS_ARRAY_SIZE       16
#define MIN_CURRENTUSER_ARRAY_SIZE    16

typedef enum
{
    noDislpay = 0,
    conditionDisplay,
    versionDisplay,
    syntaxDisplay
} dlgType_t;

typedef enum
{
    auPeRecArray = 0,  // All Users PATHEXT
    cuPeRecArray,      // Current User PATHEXT
    suFaRecArray,      // Suggested File Association
    cuFaRecArray       // Current File Association
} recArrayType_t;

typedef struct _programArguments
{
    char               exeName[MAX_PATH]; // The fully qualified executable, C:\ooRexx\ooDialog.exe perhaps
    RexxArrayObject    callArg;           // The legacy single string argument to a Rexx program
    RexxArrayObject    rxCArgs;           // Array of C style program arguments for SysCArgs put in .local
    RexxArrayObject    rxOodArgs;         // Similar to rxCArgs, but contains all arguments, .oodCArgs
    HINSTANCE          hInstance;         // This exectuable's instance
    const char        *oodProgram;        // Rexx progrm to run
    const char        *mainMsg;           // Used to pass text to display to the dialog proc edure
    dlgType_t          dlgType;           // If a display dialog is used, specifies what is displayed
    bool               doSetupScreen;     // Show the set up dialog, not used since we only have file association at this time.
    bool               showShortVersion;  // Show short version and quit
    bool               showVersion;       // Show version and quit
    bool               showShortHelp;     // Show short help and quit
    bool               showHelp;          // Show help and quit
    bool               allowServices;     // Show or don't show the configure services controls
    bool               doConfigure;       // Show the configure services dialog
} programArguments;
typedef programArguments *pProgramArguments;

typedef struct _extInfo
{
    char    displayName[MAX_EXT_DISPLAY];  // Dual column mode.
    char    displayName2[MAX_EXT_DISPLAY]; // Single column mode.
    char    extension[MAX_EXT_NAME];
    bool    exists;            // If true file association is in registry, otherwise not
    bool    allUsers;          // If true our file association exists for all users
    bool    curUser;           // If true our file association exists for the current user
    bool    allUsersOther;     // If true file association exists for all users, but not our Prog ID
    bool    curUserOther;      // If true file association exists for the current user, but not our Prog ID
    bool    remove;            // If true file assoication should be removed from registry
    bool    pathExt;           // If true extension should be added to PATHEXT
    bool    suggested;         // If true this is one of the suggested extensions.
} extensionInfo;
typedef extensionInfo *pExtensionInfo;

typedef struct _extRecordss
{
    pExtensionInfo    *faSuggestedRecs;   // Suggested file association records
    pExtensionInfo    *faCurrentRecs;     // Currently registered file association record
    pExtensionInfo    *allUsersRecs;      // PATHEXT records for scope All Users
    pExtensionInfo    *curUserRecs;       // PATHEXT records for scope Current User
    size_t             faSuggestedSize;   // size of faSuggestedRecs array
    size_t             faCurrentSize;     // size of faCurrentRecs array
    size_t             auSize;            // size of allUsersRecs array
    size_t             cuSize;            // size of curUserRecs array
    size_t             faSuggestedNext;   // next empty index in faSuggestedRecs array
    size_t             faCurrentNext;     // next empty index in faCurrentRecs array
    size_t             auNext;            // next empty index in allUsersRecs array
    size_t             cuNext;            // next empty index in curUserRecs array
} extRecords;
typedef extRecords *pExtRecords;

typedef struct _assocArguments
{
    char               friendlyName[MAX_FRIENDLY_NAME];
    char               progID[MAX_PROGID];
    HINSTANCE          hInstance;         // This executable's instance
    char              *exeName;
    pExtRecords        extensionRecords;
    HFONT              lbFont;
    HWND               lbSuggested;
    HWND               lbCurrent;
    HWND               lbPathExt;
    HWND               pbRegister;
    HWND               peEdit;
    HWND               faEdit;
    bool               allUsers;          // If true file associations is for all users, otherwise current user
    bool               isRunAsAdmin;
    bool               isElevated;
    bool               ftypeIsRegistered;
    bool               registeredAllUsers;
    bool               registeredCurUsers;
} assocArguments;
typedef assocArguments *pAssocArguments;

typedef struct _configureArguments
{
    HINSTANCE          hInstance;         // This executable's instance
    char              *exeName;
    uint32_t           integrityLevel;
    bool               isVistaOrLater;    // Os is at least Vista
    bool               isInAdminGroup;
    bool               isRunAsAdmin;
    bool               isElevated;
    bool               requiresElevation;
    bool               wasElevated;
} configureArguments;
typedef configureArguments *pConfigureArguments;


static char *long_syntax_text =
"The ooDialog executable is a stand alone interface to run ooDialog\r\n"
"programs, or to optionally provide other ooDialog services and\r\n"
"information.\r\n\r\n"
"Syntax:\r\n\r\n"
"    ooDialog [option flags] [programName] [arg1 arg2 ... argN]\r\n\r\n"
"If no arguments are given, this help is shown.  All option flags start\r\n"
"with the '-' character.  The first argument that does not start with\r\n"
"the '-' character is taken to be the name of an ooRexx program to be\r\n"
"executed.  All arguments following programName are passed as arguments\r\n"
"to programName.  When programName is executed, it is executed with no\r\n"
"console window.  This is similar to rexxHide.\r\n\r\n"
"option flags:\r\n"
"  -h        Show the short help text.\r\n"
"  /?        Same as -h.\r\n"
"  -H        Show this, the long, help text.\r\n"
"  --help    When run from a console window, show the long help.\r\n"
"  -s        Display the ooDialog Setup dialog.\r\n"
"  -v        Print the short version string.\r\n"
"  -V        Print the long version string.\r\n"
"  --version If from a console window, print the long version string.\r\n\r\n"
"programName:\r\n"
"  The first argument that does not begin with '-' is taken as the name\r\n"
"  of a Rexxx program to be executed.  The program is executed without\r\n"
"  creating a console window.  This implies that say, pull, and trace\r\n"
"  statements have no effect.\r\n\r\n"
"arg1 ... argN:\r\n"
"  Arguments following the perceived program name are passed on as\r\n"
"  arguments to programName\r\n"
"\r\n";

static char *short_syntax_text =
"Syntax:\r\n\r\n"
"    ooDialog [option flags] [programName] [arg1 arg2 ... argN]\r\n\r\n"
"option flags:\r\n"
"  -h        Show this, the short help text.\r\n"
"  /?        Same as -h.\r\n"
"  -H        Show the long help text.\r\n"
"  --help    If from a console window, show the long help.\r\n"
"  -s        Display the ooDialog Setup dialog.\r\n"
"  -v        Print the short version string.\r\n"
"  -V        Print the long version string.\r\n"
"  --version If from a console window, print the long version string.\r\n\r\n"
"programName:\r\n"
"  The first argument that does not begin with '-' is taken as the name\r\n"
"  of a Rexxx program to be executed.\r\n\r\n"
"arg1 ... argN:\r\n"
"  Arguments 1 through N are passed on to programName.\r\n\r\n";

#endif /* not defined RC_COMPILER_INVOKED*/

#endif /* defined oodExeuctable_Included */
