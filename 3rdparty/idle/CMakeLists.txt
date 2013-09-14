project(idle)

set(HEADERS idle.h)
set(SOURCES idle.cpp)

if (UNIX)
    if (APPLE)
        set(SOURCES ${SOURCES} idle_mac.cpp)
    else (APPLE)
        set(SOURCES ${SOURCES} idle_x11.cpp)
    endif (APPLE)
endif(UNIX)
if (WIN32)
   set(SOURCES ${SOURCES} idle_win.cpp)
endif (WIN32)

kde4_add_library(idle ${SOURCES})
IF(UNIX AND NOT APPLE)
SET_TARGET_PROPERTIES( idle PROPERTIES COMPILE_FLAGS -fPIC)
ENDIF(UNIX AND NOT APPLE)
target_link_libraries(idle ${QT_LIBRARIES})

