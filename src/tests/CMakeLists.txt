
REMOVE_DEFINITIONS(-DHYVESLIB_MAKEDLL)

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

INCLUDE_DIRECTORIES(
	${CMAKE_SOURCE_DIR}/3rdparty/genusapis
	${CMAKE_SOURCE_DIR}/3rdparty/idle
	${QCA2_INCLUDE_DIR}
	${CMAKE_SOURCE_DIR}/3rdparty/qjson/lib
	${CMAKE_SOURCE_DIR}/3rdparty/quazip
	${CMAKE_SOURCE_DIR}/src
)

SET(LINK_LIBRARIES HyvesDesktopLib genusapis idle qjson quazip ${QCA2_LIBRARIES} ${QT_LIBRARIES} ${ZLIB_LIBRARIES})

########### ExtenderTest ###############
SET(ExtenderTest_SRCS ExtenderTest.cpp)
KDE4_ADD_UNIT_TEST(ExtenderTest TESTNAME hyves-ExtenderTest ${ExtenderTest_SRCS})
TARGET_LINK_LIBRARIES(ExtenderTest ${LINK_LIBRARIES})

########### JabberTest ###############
# FIXME: Disabled because of incorrect credentials
#SET(JabberTest_SRCS JabberTest.cpp)
#KDE4_ADD_UNIT_TEST(JabberTest TESTNAME hyves-JabberTest ${JabberTest_SRCS})
#TARGET_LINK_LIBRARIES(JabberTest ${LINK_LIBRARIES})

########### RosterTest ###############
SET(RosterTest_SRCS RosterTest.cpp)
KDE4_ADD_UNIT_TEST(RosterTest TESTNAME hyves-RosterTest ${RosterTest_SRCS})
TARGET_LINK_LIBRARIES(RosterTest ${LINK_LIBRARIES})

########### UpdaterTest ###############
SET(UpdaterTest_SRCS UpdaterTest.cpp)
KDE4_ADD_UNIT_TEST(UpdaterTest TESTNAME hyves-UpdaterTest ${UpdaterTest_SRCS})
TARGET_LINK_LIBRARIES(UpdaterTest ${LINK_LIBRARIES})

########### UrlFetcherTest ###############
SET(UrlFetcherTest_SRCS UrlFetcherTest.cpp)
KDE4_ADD_UNIT_TEST(UrlFetcherTest TESTNAME hyves-UrlFetcherTest ${UrlFetcherTest_SRCS})
TARGET_LINK_LIBRARIES(UrlFetcherTest ${LINK_LIBRARIES})

########### UtilTest ###############
SET(UtilTest_SRCS UtilTest.cpp)
KDE4_ADD_UNIT_TEST(UtilTest TESTNAME hyves-UtilTest ${UtilTest_SRCS})
TARGET_LINK_LIBRARIES(UtilTest ${LINK_LIBRARIES})

########### VersionTest ###############
SET(VersionTest_SRCS VersionTest.cpp)
KDE4_ADD_UNIT_TEST(VersionTest TESTNAME hyves-VersionTest ${VersionTest_SRCS})
TARGET_LINK_LIBRARIES(VersionTest ${LINK_LIBRARIES})
