INCLUDE_DIRECTORIES(
	${CMAKE_SOURCE_DIR}/plugins/kwekker/src
	${CMAKE_SOURCE_DIR}/src
)

SET(Kwekker_SRCS
	Kwekker.cpp
	MainWindow.cpp
	SettingsDialog.cpp
)

SET(Kwekker_HDRS
	Kwekker.h
	MainWindow.h
	SettingsDialog.h
)

IF(WIN32)
	SET( KWEKKER_SRCS ${KWEKKER_SRCS} ${CMAKE_SOURCE_DIR}/HyvesDesktop.rc)
ENDIF(WIN32)

SET(Kwekker_FRMS
	MainWindow.ui
)

SET(Kwekker_RSCS Kwekker.qrc)
QT4_ADD_RESOURCES(Kwekker_RSCS_SRCS ${Kwekker_RSCS})

# Qt wrappers
QT4_WRAP_UI(Kwekker_FRMS_HDRS ${Kwekker_FRMS})

# Library
KDE4_ADD_LIBRARY(Kwekker SHARED
	${Kwekker_SRCS} ${Kwekker_HDRS} ${Kwekker_FRMS_HDRS} ${Kwekker_RSCS_SRCS}
)

# Specified libraries
TARGET_LINK_LIBRARIES(Kwekker HyvesDesktopLib ${QT_LIBRARIES})

SET(KWEKKER_SOUNDS buzz.wav newonline.wav headline.wav newsession.wav)
IF(APPLE)
	SET(KWEKKER_BUNDLE_PATH HyvesDesktop.app/Contents/MacOS/plugins/kwekker)
	SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/${KWEKKER_BUNDLE_PATH}/bin)
	ADD_CUSTOM_COMMAND(TARGET Kwekker
		POST_BUILD 
		COMMAND ${CMAKE_COMMAND} -E copy plugins/kwekker/manifest.xml ${CMAKE_BINARY_DIR}/${KWEKKER_BUNDLE_PATH}/
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	)
	ADD_CUSTOM_COMMAND(TARGET Kwekker
		POST_BUILD 
		COMMAND "${QT_BINARY_DIR}/fixupplugins" "HyvesDesktop.app"  "${KWEKKER_BUNDLE_PATH}/bin/libKwekker.dylib"
		COMMAND ${CMAKE_COMMAND} -E make_directory "${KWEKKER_BUNDLE_PATH}/sounds"
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	)
	FOREACH(KWEKKER_SOUND ${KWEKKER_SOUNDS})
		ADD_CUSTOM_COMMAND(TARGET Kwekker
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/../data/${KWEKKER_SOUND}" "${CMAKE_BINARY_DIR}/${KWEKKER_BUNDLE_PATH}/sounds/"
			WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
		)
	ENDFOREACH(KWEKKER_SOUND)
ELSE(APPLE)

	ADD_CUSTOM_COMMAND(TARGET Kwekker
		POST_BUILD 
		COMMAND ${CMAKE_COMMAND} -E copy plugins/kwekker/manifest.xml ${CMAKE_BINARY_DIR}/plugins/kwekker
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	)
	
	INSTALL(TARGETS Kwekker DESTINATION bin/plugins/kwekker/bin)
	SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/plugins/kwekker/bin)
	
	INSTALL(FILES ${CMAKE_SOURCE_DIR}/plugins/kwekker/manifest.xml DESTINATION bin/plugins/kwekker)

	FOREACH(KWEKKER_SOUND ${KWEKKER_SOUNDS})
		INSTALL(FILES "${CMAKE_CURRENT_SOURCE_DIR}/../data/${KWEKKER_SOUND}" DESTINATION bin/plugins/kwekker/sounds)
	ENDFOREACH(KWEKKER_SOUND)

ENDIF(APPLE)


