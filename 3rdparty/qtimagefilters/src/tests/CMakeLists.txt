SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

add_definitions(-DFILES_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

INCLUDE_DIRECTORIES(
	${CMAKE_SOURCE_DIR}/..
)

SET(LINK_LIBRARIES ${QT_LIBRARIES} qtimagefilters)

########### FilterCrashTest  ###############
#SET(FilterCrashTest_SRCS FilterCrashTest.cpp)
#KDE4_ADD_UNIT_TEST(FilterCrashTest TESTNAME qtimagefilters-FilterCrashTest ${FilterCrashTest_SRCS})
#TARGET_LINK_LIBRARIES(FilterCrashTest ${LINK_LIBRARIES})

########### FilterLenaTest  ###############
SET(FilterLenaTest_SRCS FilterLenaTest.cpp)
KDE4_ADD_UNIT_TEST(FilterLenaTest TESTNAME qtimagefilters-FilterLenaTest ${FilterLenaTest_SRCS})
TARGET_LINK_LIBRARIES(FilterLenaTest ${LINK_LIBRARIES})


########### FilterDefaultTest  ###############
SET(FilterDefaultTest_SRCS FilterDefaultTest.cpp)
KDE4_ADD_UNIT_TEST(FilterDefaultTest TESTNAME qtimagefilters-FilterDefaultTest ${FilterDefaultTest_SRCS})
TARGET_LINK_LIBRARIES(FilterDefaultTest ${LINK_LIBRARIES})

########### ImageSelectionTest  ###############
SET(ImageSelectionTest_SRCS ImageSelectionTest.cpp)
KDE4_ADD_UNIT_TEST(ImageSelectionTest TESTNAME qtimagefilters-ImageSelectionTest ${ImageSelectionTest_SRCS})
TARGET_LINK_LIBRARIES(ImageSelectionTest ${LINK_LIBRARIES})

