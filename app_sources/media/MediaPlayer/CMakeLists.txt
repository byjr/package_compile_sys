cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(${PKG_NAME})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -Os")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-write-strings -Wno-deprecated-declarations -DUSE_PIPE_BUFFER=1 -Wfatal-errors")

include_directories(${CMAKE_INCLUDE_PATH})

file(GLOB_RECURSE PROJECT_SOURCES
	*.cpp
)

link_directories(
	${CMAKE_LIBRARY_PATH}
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_link_libraries(${PROJECT_NAME}
	avcodec avformat avutil swresample
	asound
	lzUtils
	pthread
	rt
)

install(TARGETS ${PROJECT_NAME} DESTINATION "bin")