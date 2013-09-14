	SET(SOURCES
	ioapi.c
	quazip.cpp
	quazipfile.cpp
	quazipnewinfo.cpp
	unzip.c
	zip.c
)

SET(HEADERS
	crypt.h
	ioapi.h
	quazip.h
	quazipfileinfo.h
	quazipnewinfo.h
	unzip.h
	zip.h
)

SET(MOC_HEADERS
	quazipfile.h
)

# Qt wrappers
QT4_WRAP_CPP(MOC_SOURCES ${MOC_HEADERS})

# Library
ADD_LIBRARY(quazip
	${SOURCES} ${HEADERS} ${MOC_SOURCES} ${MOC_HEADERS}
)

IF(UNIX AND NOT APPLE) 
SET_TARGET_PROPERTIES(quazip PROPERTIES COMPILE_FLAGS -fPIC)
ENDIF(UNIX AND NOT APPLE)
# Specified libraries
TARGET_LINK_LIBRARIES(quazip ${QT_LIBRARIES})
