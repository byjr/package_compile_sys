cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(RtspPullHandle)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os -D_WIN_API_")
else()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -Os")
endif()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-write-strings -Wno-deprecated-declarations -DUSE_PIPE_BUFFER=1 -Wfatal-errors")
set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_C_FLAGS}")


include_directories(${CMAKE_INCLUDE_PATH})
file(GLOB_RECURSE PROJECT_SOURCES
	*.cpp
)
message(PROJECT_SOURCES:${PROJECT_SOURCES})
link_directories(${CMAKE_LIBRARY_PATH})
link_libraries(
	cppUtils
	lzUtils
	avformat
	avcodec
	avutil
	pthread
	curl
)
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "bin")
