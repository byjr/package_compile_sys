cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(${PKG_NAME})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-write-strings -Wno-deprecated-declarations -DUSE_PIPE_BUFFER=1 -Wfatal-errors")
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WIN_API_")
endif()

include_directories(${CMAKE_INCLUDE_PATH} ${SOURCE_DIR})

file(GLOB_RECURSE PROJECT_SOURCES
	*.cpp
)

link_directories(
	${CMAKE_LIBRARY_PATH}
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_link_libraries(${PROJECT_NAME}
	ixml upnp
	avcodec
	avutil
	avformat
	swresample
	asound
	lzUtils
	pthread
	rt
	m
)

install(TARGETS ${PROJECT_NAME} DESTINATION "bin")