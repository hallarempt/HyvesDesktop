# - Relocates an OSX library
#
# Given an file to relocate, library and the relative location of the location
# of the library then this macro will run install_name_tool to correctly
# relocate the library to point to the correct location.
#
# This module defines the following macros:
# relocate_osx_library
macro(relocate_osx_library RELOCATION_FILE RELOCATE_LIBRARY
      RELOCATE_LIBRARY_RELATIVE)
   get_filename_component(_RELOCATE_LIBRARY_NAME
        ${RELOCATE_LIBRARY} NAME
    )
    install(CODE
        "execute_process(
            COMMAND install_name_tool -change
            ${RELOCATE_LIBRARY} ${RELOCATE_LIBRARY_RELATIVE}/${_RELOCATE_LIBRARY_NAME}
            \$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${RELOCATION_FILE})"
    )
endmacro(relocate_osx_library RELOCATION_FILE RELOCATE_LIBRARY 
         RELOCATE_LIBRARY_RELATIVE)