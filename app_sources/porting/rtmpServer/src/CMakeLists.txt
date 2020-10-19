cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(${PKG_NAME})

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os -D_WIN_API_")
else()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -Os")
endif()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-write-strings -Wno-deprecated-declarations -DUSE_PIPE_BUFFER=1 -Wfatal-errors")
set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_C_FLAGS}")


include_directories(${CMAKE_INCLUDE_PATH})
file(GLOB_RECURSE PROJECT_SOURCES
	*.cc
)

link_directories(${CMAKE_LIBRARY_PATH})

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_link_libraries(${PROJECT_NAME}

)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "bin")
