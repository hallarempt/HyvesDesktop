project(qjson)
include_directories (./)

ADD_DEFINITIONS(-DQJSON_MAKEDLL)


# add_custom_command (OUTPUT json_parser.cc
#                    PRE_BUILD
#                    COMMAND bison -t -o json_parser.cc -d json_parser.yy
#                    DEPENDS json_parser.yy
#                    WORKING_DIRECTORY ${qjson_SOURCE_DIR}/src/
#                    )

set (qjson_SRCS json_driver.cc json_scanner.cpp json_parser.cc)

kde4_add_library (qjson ${qjson_SRCS})
IF(UNIX AND NOT APPLE)
SET_TARGET_PROPERTIES( qjson PROPERTIES COMPILE_FLAGS -fPIC)
ENDIF(UNIX AND NOT APPLE)
target_link_libraries( qjson ${QT_LIBRARIES})
