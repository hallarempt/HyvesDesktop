project(qtimagefilters)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0 FATAL_ERROR)
SET(CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(.)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(./src)

add_subdirectory(src)
