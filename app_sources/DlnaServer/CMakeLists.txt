cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(DCSApp LANGUAGES CXX)
project(${PKG_NAME})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC -Os")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-write-strings -Wno-deprecated-declarations -DUSE_PIPE_BUFFER=1 -Wfatal-errors")
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WIN_API_")
endif()
if(BUILD_DLNA)
    add_definitions(-DDUEROS_DLNA)
    set(DLNA_SOURCE_FILES 
            src/DlnaDmrOutputFfmpeg.cpp
            src/Dlna/DlnaDmrOutput.cpp
            src/Dlna/DlnaDmrSdk.cpp 
            src/Dlna/DlnaDmrSongMetaData.cpp 
            src/Dlna/DlnaDmrUpnp.cpp 
            src/Dlna/DlnaDmrUpnpConnMgr.cpp 
            src/Dlna/DlnaDmrUpnpControl.cpp 
            src/Dlna/DlnaDmrUpnpDevice.cpp 
            src/Dlna/DlnaDmrUpnpRender.cpp 
            src/Dlna/DlnaDmrUpnpTransport.cpp 
            src/Dlna/DlnaDmrVariableContainer.cpp 
            src/Dlna/DlnaDmrWebServer.cpp 
            src/Dlna/DlnaDmrXmlDoc.cpp 
            src/Dlna/DlnaDmrXmlEscape.cpp
			src/MediaPlayer/Impl/DlnaPlayerImpl.cpp
        )
    set(DLNA_SDK_LIB ixml upnp)
    set(DLNA_INCLUDE_DIR "${SOURCE_DIR}/include/Dlna")
endif()

add_executable(${PROJECT_NAME}
	src/main.cpp
	${DLNA_SOURCE_FILES}
	)

target_include_directories(${PROJECT_NAME} PUBLIC
    "${SOURCE_DIR}/include"
	"${SOURCE_DIR}/include/MediaPlayer/Impl"
	"${SOURCE_DIR}/include/MediaPlayer/Interface"
	"${SOURCE_DIR}/include/MediaPlayer/Listener"
	"${SOURCE_DIR}/include/MediaPlayer/Proxy"
	"${DLNA_INCLUDE_DIR}"
	"${CMAKE_INCLUDE_PATH}"	
    )

target_link_libraries(${PROJECT_NAME}
        "-L${CMAKE_LIBRARY_PATH}"
        asound
        avcodec
        avutil
        avformat
        swresample
        pthread
		lzUtils
        rt
        m		
        "${DLNA_SDK_LIB}"
)


install(TARGETS ${PROJECT_NAME} DESTINATION "bin")
