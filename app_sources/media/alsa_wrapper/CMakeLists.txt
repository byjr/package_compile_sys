cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(${PKG_NAME})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -Os")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-write-strings -Wno-deprecated-declarations -DUSE_PIPE_BUFFER=1 -Wfatal-errors")

include_directories(${CMAKE_INCLUDE_PATH})

file(GLOB_RECURSE PROJECT_SOURCES
	*.c
)

link_directories(
	${CMAKE_LIBRARY_PATH}
)
add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES})

target_link_libraries(${PROJECT_NAME}
	asound
	lzUtils
)

install(TARGETS ${PROJECT_NAME} DESTINATION "lib")
install(DIRECTORY ${PROJECT_INCS}
		DESTINATION "include/${PROJECT_NAME}"
		FILES_MATCHING PATTERN "*.h"
)