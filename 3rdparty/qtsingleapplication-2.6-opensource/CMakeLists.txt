SET(SOURCES
src/qtlocalpeer.cpp
src/qtlockedfile.cpp
src/qtsingleapplication.cpp
src/qtsinglecoreapplication.cpp
)

IF(UNIX)
	SET(SOURCES ${SOURCES}
		    src/qtlockedfile_unix.cpp
	)
ELSE(UNIX)
	SET(SOURCES ${SOURCES}
	 	    src/qtlockedfile_win.cpp
	)
ENDIF(UNIX)

SET(HEADERS
src/qtlocalpeer.h
src/qtlockedfile.h
src/qtsingleapplication.h
src/qtsinglecoreapplication.h
)

KDE4_ADD_LIBRARY(qtsingleapplication ${SOURCES} ${HEADERS})
IF(UNIX AND NOT APPLE)
	SET_TARGET_PROPERTIES( qtsingleapplication PROPERTIES COMPILE_FLAGS -fPIC)
ENDIF(UNIX AND NOT APPLE)
TARGET_LINK_LIBRARIES(qtsingleapplication ${QT_LIBRARIES})

