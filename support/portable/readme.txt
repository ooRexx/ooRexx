2021-12-04, 2022-01-30, rgf

        - this directory containes Rexx programs for "make portable"

                - createPortable.rex

                        - expects the path for DESTDIR and the name of the zip archive

                          - installs into DESTDIR
                          - creates the zip directory which gets zipped up
                            - copies the files from the installation to its flattened subdirectoires
                              - bin     ... binaries
                              - lib     ... link, shared or dynamic libraries
                              - include ... include (*.h) files to be able to compile C/C++ programs
                              - docs    ... pdf documentation
                              - samples ... samples (some depend on operating system)

                          - copies "setupoorexx.rex" and "test.rex" to zip archive directory; its
                            hashbang line points to "bin/rexx" to run it
                          - creates "setupoorexx.{cmd|sh}" which will run "bin/rexx setupoorexx.rex"

                - test.rex

                        - test program that the readme.txt refers to, displays .rexxinfo information

                - setupoorexx.rex

                        - creates "readme.txt" adjusted for the operating system

                        - creates two script files,

                          - "rxenv.{cmd|sh}": "REXX_HOME ENVironment"

                          - one to run Rexx related programs that use the archive's
                            bin/rexx locally (environment not changed)

                          - "setenv2rxenv.{cmd|sh}": "set environment to REXX_HOME ENVironment"

temporarily:

        createPortable.patch ... patch for CMakeLists.txt to add "portable" target

TODO: If adding to svn do:

        svn propset svn:eol-style native filename
        svn commit filename

i.e.

        svn propset svn:eol-style native *.rex *.txt
        svn commit -m "..."

(cf. developer mailing list
Subject: Re: [Oorexx-devel] [Oorexx-svn] SF.net SVN: oorexx-code-0:[12319] main/trunk/samples/api/c++/external
Date:    Fri, 26 Nov 2021 14:36:56 -0500
From:    Rick McGuire <object.rexx@gmail.com> )

