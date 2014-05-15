                    Instructions For Using CMake To Build ooRexx
                                  May 15, 2014

The ooRexx project has begun the transition to using CMake to build ooRexx.
There are a number of advantages to using CMake which include support for
multiple platforms (Windows, Linux, AIX, etc.), a simple build
mechanism, and many others.

One of the big advantages for developers is that CMake is designed to 
perform "out of source" builds of the target project. This means that
builds should be performed from a disk location that is NOT in the
project source tree. Using this methodology keeps the project source tree
clean of all build output files and also allows different types of builds
(development, debug, release) to be active at the same time.

To build ooRexx using CMake you should follow these instruction. You can build
ooRexx on multiple platforms (Windows, Linux, etc) using CMake.

1. Download and install CMake. For Windows go to http://www.cmake.org/ and
   download the Windows install file. For *nix distribution please install CMake
   from your distribution's repository.
2. Check out the ooRexx source from the SourceForge SVN repository.
3. Now create a subdirectory on your disk that is NOT in the ooRexx source tree
   you checked out in the previous step. All build files will be placed in this
   location.
4. You are now ready to create everything necessary in building ooRexx on your
   platform. On Windows run the following commands:

   cmake -G "NMake Makefiles" C:\ooRexx\source lication
   nmake
   cpack ./

   On Linux run the following commands:

   cmake -DBUILD_RPM=1 /ooRexx/source/location # use -DBUILD_DEB=1 for debian
   make
   cpack ./

   After the completion of these commands there should be a bin subdirectory 
   created in your current directory. It should contain all the binaries
   and install files (if the build succeeded).

