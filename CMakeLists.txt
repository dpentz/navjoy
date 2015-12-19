project(navjoy)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -D_GNU_SOURCE -pedantic")

#get libspacenavd locations
find_path(spnav_INCLUDE_DIR NAMES spnav.h)
if(spnav_INCLUDE_DIR)
	message(STATUS "Found spacenavd headers in ${spnav_INCLUDE_DIR}")
	include_directories(${spnav_INCLUDE_DIR})
else()
	message(FATAL_ERROR "Could not find spacenavd includes, do you have libspacenavd installed?")
endif()

find_library(spnav_LIBRARY NAMES spnav)
if(spnav_LIBRARY)
	message(STATUS "Found spacenavd library in ${spnav_LIBRARY}")
else()
	message(FATAL_ERROR "Could not find libspacenavd")
endif()

add_subdirectory(src)
