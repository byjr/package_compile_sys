cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(lzUtils)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os -D_WIN_API_")
else()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -Os")
endif()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-write-strings -Wno-deprecated-declarations -DUSE_PIPE_BUFFER=1 -Wfatal-errors")

include_directories(${CMAKE_INCLUDE_PATH})

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
file(GLOB_RECURSE PROJECT_SOURCES
	autom/*.c
	ccsrb/*.c
	common/*.c
	csrb/*.c
	slog/*.c
	vad_op/*.c
	dfq_fifo/*.c
)
else()
file(GLOB_RECURSE PROJECT_SOURCES
	autom/*.c
	ccsrb/*.c
	common/*.c
	csrb/*.c
	shrb/*.c
	slog/*.c
	trd_timer/*.c
	un_fifo_ops/*.c
	user_fifo/*.c
	user_timer/*.c
	ut_socket/*.c
	vad_op/*.c
	dfq_fifo/*.c
)
endif()

link_directories(
	${CMAKE_LIBRARY_PATH}
)
add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES})

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
target_link_libraries(${PROJECT_NAME}
	pthread
)
else()
target_link_libraries(${PROJECT_NAME}
	asound
)
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION "lib")
install(PROGRAMS base.h DESTINATION "include/${PROJECT_NAME}")

set(last_dir "null")
foreach(all_path ${PROJECT_SOURCES})
	string(REGEX REPLACE "(.+)\/(.+)\/.+\.c" "\\2" cur_dir ${all_path})
	# message(cur_dir:${cur_dir},last_dir:${last_dir})
	if(NOT ${last_dir} STREQUAL ${cur_dir})
		set(PROJECT_INCS "${PROJECT_INCS};${cur_dir}")
		# message(PROJECT_INCS:${PROJECT_INCS})
	endif()
	set(last_dir ${cur_dir})
endforeach(all_path)
unset(last_dir)

# message(PROJECT_INCS:${PROJECT_INCS})
install(DIRECTORY ${PROJECT_INCS}
		DESTINATION "include/${PROJECT_NAME}"
		FILES_MATCHING PATTERN "*.h"
)