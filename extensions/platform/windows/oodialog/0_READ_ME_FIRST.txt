                  %REXX_HOME%\samples\oodialog\source
                  -----------------------------------

    The %REXX_HOME%\samples\oodialog\source directory contains the complete
    source code for ooDialog.cls.  The source code is contained in a number
    of smaller *.cls files, and these files are combined into the single
    ooDialog.cls file.

    Included with the source code is the build_ooDialog_cls.rex Rexx
    program.  This program is used to combine the smaller files, it
    produces ooDialog.cls, oodPlain.cls, and oodWin32.cls.

    Note that oodPlain.cls and oodWin32.cls are no longer needed for
    ooDialog.  They simply provide backwards compatibility for ooDialog
    programs and do nothing.

    This source directory could be used, for example, to make some small
    changes to the class files and then recombine them into ooDialog.cls.
    Perhaps to fix a bug, or to add some debugging say statements, or for
    some other exploratory reason.

    To use build_ooDialog_cls.rex:

    Execute the program from this directory, with no arguments.  New
    versions of ooDialog.cls, oodPlain.cls, and oodWin32.cls will be
    reproduced.

    NOTE to the wise:  It would be prudent to copy the original files to a
    safe place before making changes.

    The rebuilt ooDialog.cls can then be used to replace the ooDialog.cls
    in %REXX_HOME%.
