cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(lzUtils)

file(GLOB_RECURSE lzUtils_SOURCES
	alsa_ctrl/*.c
	autom/*.c
	ccsrb/*.c
	common/*.c
	csrb/*.c
	slog/*.c
	trd_timer/*.c
	un_fifo_ops/*.c
	user_fifo/*.c
	user_timer/*.c
	ut_socket/*.c
	vad_op/*.c
	dfq_fifo/*.c
)

add_library(lzUtils SHARED ${lzUtils_SOURCES})

target_include_directories(lzUtils PUBLIC
	"."
)

target_link_libraries(lzUtils
	asound
)

install(TARGETS lzUtils DESTINATION "lib")

install(DIRECTORY "./"
	DESTINATION "include/lzUtils"
	FILES_MATCHING PATTERN "*.h"
)