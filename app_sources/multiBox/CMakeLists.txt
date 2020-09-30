cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(multiBox)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os -D_WIN_API_")
else()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -Os")
endif()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-write-strings -Wno-deprecated-declarations -DUSE_PIPE_BUFFER=1 -Wfatal-errors")
set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_C_FLAGS}")


if (DEFINED Misc_ENABLE)
	set(Misc_SOURCES src/Misc/*.cpp src/Misc/*.c)
	add_definitions(-D Misc_ENABLE)
	message("enable Misc_SOURCES")
endif()

if (DEFINED adcDetect_ENABLE)
	set(adcDetect_SOURCES src/adcDetect/*.cpp src/adcDetect/*.c)
	add_definitions(-D adcDetect_ENABLE)
	message("enable adcDetect_SOURCES")
endif()

if (DEFINED adcKey_ENABLE)
	set(adcKey_SOURCES src/adcKey/*.cpp src/adcKey/*.c)
	add_definitions(-D adcKey_ENABLE)
	message("enable adcKey_SOURCES")
endif()

if (DEFINED allPlay_ENABLE)
	set(allPlay_SOURCES src/allPlay/*.cpp src/allPlay/*.c)
	set(allPlay_EX_LIBS avcodec avformat avutil swresample)
	add_definitions(-D allPlay_ENABLE)
	message("enable allPlay_SOURCES")
endif()

if (DEFINED Base64Tool_ENABLE)
	set(Base64Tool_SOURCES src/Base64Tool/*.cpp src/Base64Tool/*.c)
	add_definitions(-D Base64Tool_ENABLE)
	message("enable Base64Tool_SOURCES")
endif()

if (DEFINED BurnFw_ENABLE)
	set(BurnFw_SOURCES src/BurnFw/*.cpp src/BurnFw/*.c)
	add_definitions(-D BurnFw_ENABLE)
	message("enable BurnFw_SOURCES")
endif()

if (DEFINED CrcTool_ENABLE)
	set(CrcTool_SOURCES src/CrcTool/*.cpp src/CrcTool/*.c)
	add_definitions(-D CrcTool_ENABLE)
	message("enable CrcTool_SOURCES")
endif()

if (DEFINED gyrosensor_ENABLE)
	set(gyrosensor_SOURCES src/gyrosensor/*.cpp src/gyrosensor/*.c)
	add_definitions(-D gyrosensor_ENABLE)
	message("enable gyrosensor_SOURCES")
endif()

if (DEFINED hotplug_ENABLE)
	set(hotplug_SOURCES src/hotplug/*.cpp src/hotplug/*.c)
	add_definitions(-D hotplug_ENABLE)
	message("enable hotplug_SOURCES")
endif()

if (DEFINED i2cClient_ENABLE)
	set(i2cClient_SOURCES src/i2cClient/*.cpp src/i2cClient/*.c)
	add_definitions(-D i2cClient_ENABLE)
	message("enable i2cClient_SOURCES")
endif()

if (DEFINED lockCtrl_ENABLE)
	set(lockCtrl_SOURCES src/lockCtrl/*.cpp src/lockCtrl/*.c)
	add_definitions(-D lockCtrl_ENABLE)
	message("enable lockCtrl_SOURCES")
endif()

if (DEFINED mixer_ENABLE)
	set(mixer_SOURCES src/mixer/*.cpp src/mixer/*.c)
	add_definitions(-D mixer_ENABLE)
	message("enable mixer_SOURCES")
endif()

if (DEFINED PaOption_ENABLE)
	set(PaOption_SOURCES src/PaOption/*.cpp src/PaOption/*.c)
	add_definitions(-D PaOption_ENABLE)
	message("enable PaOption_SOURCES")
endif()

if (DEFINED sensor_ENABLE)
	set(sensor_SOURCES src/sensor/*.cpp src/sensor/*.c)
	add_definitions(-D sensor_ENABLE)
	message("enable sensor_SOURCES")
endif()

if (DEFINED SockServ_ENABLE)
	set(SockServ_SOURCES src/SockServ/*.cpp src/SockServ/*.c)
	add_definitions(-D SockServ_ENABLE)
	message("enable SockServ_SOURCES")
endif()

if (DEFINED uartd_ENABLE)
	set(uartd_SOURCES src/uartd/*.cpp src/uartd/*.c)
	add_definitions(-D uartd_ENABLE)
	# set(uartd_EX_LIBS bootloader_message)
	message("enable uartd_SOURCES")
endif()

if (DEFINED AppManager_ENABLE)
	set(AppManager_SOURCES src/AppManager/*.cpp src/AppManager/*.c)
	add_definitions(-D AppManager_ENABLE)
	message("enable AppManager_SOURCES")
endif()

if (DEFINED ntop_ENABLE)
	set(ntop_SOURCES src/ntop/*.cpp src/ntop/*.c)
	add_definitions(-D ntop_ENABLE)
	message("enable ntop_SOURCES")
endif()

if (DEFINED RtPlayer_ENABLE)
	set(RtPlayer_SOURCES src/RtPlayer/*.cpp src/RtPlayer/*.c)
	add_definitions(-D RtPlayer_ENABLE)
	message("enable RtPlayer_SOURCES")
endif()

if (DEFINED Broadcast_ENABLE)
	set(Broadcast_SOURCES src/Broadcast/*.cpp src/Broadcast/*.c)
	add_definitions(-D Broadcast_ENABLE)
	message("enable Broadcast_SOURCES")
endif()

if (DEFINED RecvBc_ENABLE)
	set(RecvBc_SOURCES src/RecvBc/*.cpp src/RecvBc/*.c)
	add_definitions(-D RecvBc_ENABLE)
	message("enable RecvBc_SOURCES")
endif()

if (DEFINED HttpCli_ENABLE)
	set(HttpCli_SOURCES src/HttpCli/*.cpp src/HttpCli/*.c)
	add_definitions(-D HttpCli_ENABLE)
	message("enable HttpCli_SOURCES")
endif()

include_directories(${CMAKE_INCLUDE_PATH})
file(GLOB_RECURSE PROJECT_SOURCES
	src/main.cpp
	${adcDetect_SOURCES}
	${adcKey_SOURCES}
	${allPlay_SOURCES}
	${Base64Tool_SOURCES}
	${BurnFw_SOURCES}
	${CrcTool_SOURCES}
	${gyrosensor_SOURCES}
	${hotplug_SOURCES}
	${i2cClient_SOURCES}
	${lockCtrl_SOURCES}
	${Misc_SOURCES}
	${mixer_SOURCES}
	${PaOption_SOURCES}
	${sensor_SOURCES}
	${SockServ_SOURCES}
	${uartd_SOURCES}
	${AppManager_SOURCES}
	${ntop_SOURCES}
	${RtPlayer_SOURCES}
	${Broadcast_SOURCES}
	${RecvBc_SOURCES}
	${HttpCli_SOURCES}
)

link_directories(${CMAKE_LIBRARY_PATH})

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_link_libraries(${PROJECT_NAME}
	cppUtils
	lzUtils
	asound
	pthread
	rt
	${allPlay_EX_LIBS}
	${uartd_EX_LIBS}
)

install(DIRECTORY "files/"
	DESTINATION "bin"
	FILES_MATCHING PATTERN "*.sh"
)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "bin")
