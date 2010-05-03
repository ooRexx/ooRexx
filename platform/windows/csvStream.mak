#/*----------------------------------------------------------------------------*/
#------------------------
# csvStream.MAK make file
#------------------------
all: $(OR_OUTDIR)\csvStream.cls
    @ECHO .
    @ECHO All done csvStrean.cls
    @ECHO .

!include "$(OR_LIBSRC)\ORXWIN32.MAK"

SOURCE_DIR = $(OR_EXTENSIONS)\csvstream

#
# Copy csvStream.cls to the build directory so the test suite can be run directly
# from that location without doing an install.
#
$(OR_OUTDIR)\cvsStream.cls : $(SOURCE_DIR)\csvStream.cls
    @ECHO .
    @ECHO Copying $(SOURCEDIR)\csvStream.cls
    copy $(SOURCE_DIR)\csvStream.cls $(OR_OUTDIR)


