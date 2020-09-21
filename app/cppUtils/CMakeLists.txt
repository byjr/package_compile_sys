cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(cppUtils)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-write-strings -Wno-deprecated-declarations -DUSE_PIPE_BUFFER=1 -Wfatal-errors -g0 -Os")

file(GLOB_RECURSE cppUtils_SOURCES
	"*.cpp"
)

include_directories(

)

add_library(cppUtils SHARED ${cppUtils_SOURCES})

target_link_libraries(cppUtils
	lzUtils
)

install(TARGETS cppUtils DESTINATION "lib")

install(DIRECTORY "./"
	DESTINATION "include/cppUtils"
	FILES_MATCHING PATTERN "*.h"
)