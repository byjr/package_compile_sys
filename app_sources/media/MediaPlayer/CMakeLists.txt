cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(${PKG_NAME})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -g")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wfatal-errors -D_PKG_NAME_=${PKG_NAME}")
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_WIN_API_")
endif()
set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_C_FLAGS}")

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
	alsa_wrapper
	lzUtils
	pthread
	rt
)

install(TARGETS ${PROJECT_NAME} DESTINATION "bin")