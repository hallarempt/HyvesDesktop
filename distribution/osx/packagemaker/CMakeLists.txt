# The CPack PackageMaker generator ignores CMAKE_INSTALL_PREFIX and places everything in /usr, complain if there is a mismatch
#IF(NOT "${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr")
#       MESSAGE(SEND_ERROR "K3D_CREATE_PACKAGEMAKER_PACKAGE requires CMAKE_INSTALL_PREFIX set to \"/usr\".")
#ENDIF(NOT "${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr")

# Generate a packagemaker package ...
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/Copyright.txt ${CMAKE_CURRENT_BINARY_DIR}/License.txt COPYONLY)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/package-binary-packagemaker.cpack.in ${CMAKE_CURRENT_BINARY_DIR}/package-binary-packagemaker.cpack)

ADD_CUSTOM_TARGET(package-binary-packagemaker
	${CMAKE_CPACK_COMMAND}
	--config ${CMAKE_CURRENT_BINARY_DIR}/package-binary-packagemaker.cpack
    DEPENDS HyvesDesktop
	)

