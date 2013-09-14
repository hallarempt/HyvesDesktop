QT4_ADD_RESOURCES(CrashReporter_RCC_SRCS CrashReporter.qrc)

SET(CrashReporter_SRCS
	main.cpp
        Restarter.cpp
	CrashReporter.cpp
)

QT4_WRAP_UI(CrashReporter_FRMS 
		CrashReporter.ui
)

KDE4_ADD_EXECUTABLE(CrashReporter ${CrashReporter_SRCS} ${CrashReporter_RCC_SRCS} ${CrashReporter_FRMS} )
TARGET_LINK_LIBRARIES(CrashReporter ${QT_LIBRARIES})
INSTALL(TARGETS CrashReporter DESTINATION bin)

IF (APPLE)
        ADD_CUSTOM_COMMAND(TARGET CrashReporter
                POST_BUILD 
                COMMAND "${QT_BINARY_DIR}/macdeployqt" "CrashReporter.app"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "CrashReporter.app/Contents/MacOS/CrashReporter" "${CMAKE_BINARY_DIR}/HyvesDesktop.app/Contents/MacOS/"
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
ENDIF (APPLE)

