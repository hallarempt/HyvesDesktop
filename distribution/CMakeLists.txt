IF (EXISTS "${CMAKE_SOURCE_DIR}/.svn")
	SET(HD_SVN_ROOT    "https://trac.startpda.net/svn/hyves/HyvesDesktop")
	SET(HD_SVN_TRUNK   "${HD_SVN_ROOT}/trunk")
	SET(HD_SVN_TAGS    "${HD_SVN_ROOT}/tags")
	SET(HD_SVN_TAGS    "${HD_SVN_ROOT}/branches")
	SET(HD_BRANCH_TAG  "HyvesDesktop-release-${HD_MAJOR_VERSION}.${HD_MINOR_VERSION}")
	SET(HD_VERSION_TAG "HyvesDesktop-${HD_MAJOR_VERSION}.${HD_MINOR_VERSION}.${HD_PATCH_VERSION}.${HD_BUILD_VERSION}")
	
	ADD_CUSTOM_TARGET(changelog
		COMMAND ${SVNCOMMAND} log > ${HD_BINARY_DIR}/ChangeLog
		WORKING_DIRECTORY ${HD_SOURCE_DIR}
	)
	
	ADD_CUSTOM_TARGET(tag-trunk
		COMMAND ${SVNCOMMAND} cp ${HD_SVN_TRUNK}
		${HD_SVN_TAGS}/${HD_VERSION_TAG} -m "COMP: Tagged Hyves Desktop version ${HD_VERSION}"
		WORKING_DIRECTORY ${HD_SOURCE_DIR}
	)
	
	ADD_CUSTOM_TARGET(tag-branch
		COMMAND ${SVNCOMMAND} cp ${HD_SVN_TRUNK}
		${HD_SVN_BRANCHES}/${HD_BRANCH_TAG} -m "COMP: Branched Hyves Desktop version ${HD_MAJOR_VERSION}.${HD_MINOR_VERSION}"
		WORKING_DIRECTORY ${HD_SOURCE_DIR}
	)

ENDIF (EXISTS "${CMAKE_SOURCE_DIR}/.svn")

# Setup global packaging parameters ...
MESSAGE(STATUS "Binary dir: ${HD_BINARY_DIR}")
SET(HD_CPACK_INSTALL_CMAKE_PROJECTS "${HD_BINARY_DIR};HyvesDesktop;ALL;/")
SET(HD_CPACK_PACKAGE_DESCRIPTION "Hyves Desktop -- Always in touch with your friends")
SET(HD_CPACK_PACKAGE_FILE_NAME "Hyves Desktop-setup-${HD_VERSION}")
SET(HD_CPACK_SOURCE_PACKAGE_FILE_NAME "HyvesDesktop-src-${HD_VERSION}")
SET(HD_CPACK_PACKAGE_NAME "Hyves Desktop")
SET(HD_CPACK_PACKAGE_VERSION "${HD_VERSION}")
SET(HD_CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/terms_of_use.txt")
SET(HD_CPACK_SOURCE_IGNORE_FILES "/CVS/;/.svn/;build;photouploader")
SET(HD_CPACK_SOURCE_INSTALLED_DIRECTORIES "${CMAKE_SOURCE_DIR};/")

SET(COMMON_IGNORE_FILES '/CMakeFiles/' '/.svn/' '_CPack_Packages/'
	'.cmake$' '~' '.swp' '.log' '.gz' '.directory$' 'CMakeCache.txt')

# generate source package for archiving
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/package-source-tgz.cpack.in ${CMAKE_CURRENT_BINARY_DIR}/package-source-tgz.cpack)
ADD_CUSTOM_TARGET(package-source-tgz
	${CMAKE_CPACK_COMMAND}
	--config ${CMAKE_CURRENT_BINARY_DIR}/package-source-tgz.cpack
)

IF (APPLE)
	ADD_SUBDIRECTORY(osx)
ENDIF (APPLE)

IF (UNIX AND NOT APPLE)
	ADD_SUBDIRECTORY(posix)
ENDIF (UNIX AND NOT APPLE)

IF (WIN32)
	ADD_SUBDIRECTORY(win32)
ENDIF (WIN32)
