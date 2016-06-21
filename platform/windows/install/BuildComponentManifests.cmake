function(build_component_manifest component manifest install_dir source_dir)
  # the generated manifest file needs to use directories in native form
  file(TO_NATIVE_PATH ${source_dir} NATIVE_SOURCE_DIR)
  message("Creating manifest file ${manifest}")
  # set the installation dir location...this is the initial
  # value.  If we have a more complex directory structure, this
  # we will need to switch to the subdirectory
  FILE(WRITE ${manifest} "; Installing files for component ${component}\n")
  # this is the relative location of the subdirectory.  Any time
  # the relative subdirectory changes, we'll need to change the outdir again
  # we set this to an invalid path so we end up changing this the first
  # time through
  set(RELATIVE_OUTDIR "////")

  # Now gather the files and copy
  FILE(GLOB_RECURSE files RELATIVE ${source_dir} ${source_dir}/*)
  foreach(file ${files})
     # split the name into the path and the file name pieces.  Note that
     # we've got a relative path here, which may require us to change
     # the outdir location
     get_filename_component(name ${file} NAME)
     get_filename_component(path ${file} PATH)

     # convert this to a Windows-format path
     string(REPLACE "/" "\\" path "${path}")
     string(COMPARE NOTEQUAL "${path}" "${RELATIVE_OUTDIR}" new_outdir)

     # now check to see if we need to change the outdir
     if (${new_outdir})
        # this could just be a root file element, so just revert to the
        # given source directory
        if (NOT path)
           # just switch to the original installation directory
           FILE(APPEND ${manifest}
"\n  ; Set output path to the installation directory.
  \${SetOutPath} ${install_dir}\n\n")
        else ()
           # add on the subdirectory
           FILE(APPEND ${manifest}
"\n  ; Set output path to the installation subdirectory.
  \${SetOutPath} ${install_dir}\\${path}\n\n")
        endif()
        # set the new relative path
        set(RELATIVE_OUTDIR ${path})
     endif()
     # build up the fully qualified source path...complicated by
     # the root outdir values

     if (NOT path)
       # directly from the source path
       set(file_source ${NATIVE_SOURCE_DIR})
     else ()
       # add on the relative location
       set(file_source "${NATIVE_SOURCE_DIR}\\${path}")
     endif()

     # add a ${File} statement for this install file
     FILE(APPEND ${manifest}
"  \${File} \"${file_source}\\\" \"${name}\"\n")
  endforeach(file)
  # Finally switch back to the original install directory. Note that this
  # final one does NOT use the ${SetOutPath} macro because we don't want
  # include this change in the uninstall log
  FILE(APPEND ${manifest}
"\n  ; Set output path to the installation directory just in case
  SetOutPath $INSTDIR")
endfunction()

# Set some handy directories
set(NSIS_DIR ${BINARY_DIR}/NSIS)
set(NSIS_FILES ${NSIS_DIR}/files)

# Core component files.
build_component_manifest(Core ${NSIS_DIR}/Core_component_manifest.nsh "\$INSTDIR" ${NSIS_FILES}/Core)
# The rest of the components install in subdirectories, so just copy the whole file structure.
build_component_manifest(Docs ${NSIS_DIR}/Docs_component_manifest.nsh "\$INSTDIR" ${NSIS_FILES}/Docs)
build_component_manifest(Samples ${NSIS_DIR}/Samples_component_manifest.nsh "\$INSTDIR" ${NSIS_FILES}/Samples)
build_component_manifest(DevLib ${NSIS_DIR}/DevLib_component_manifest.nsh "\$INSTDIR" ${NSIS_FILES}/DevLib)


