project(${PKG_NAME})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC -Os")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-write-strings -Wno-deprecated-declarations -DUSE_PIPE_BUFFER=1 -Wfatal-errors")
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WIN_API_")
endif()
if(BUILD_DLNA)
    add_definitions(-DDUEROS_DLNA)
    set(DLNA_SOURCE_FILES 
            DlnaDmrOutputFfmpeg.cpp
            Dlna/DlnaDmrOutput.cpp
            Dlna/DlnaDmrSdk.cpp 
            Dlna/DlnaDmrSongMetaData.cpp 
            Dlna/DlnaDmrUpnp.cpp 
            Dlna/DlnaDmrUpnpConnMgr.cpp 
            Dlna/DlnaDmrUpnpControl.cpp 
            Dlna/DlnaDmrUpnpDevice.cpp 
            Dlna/DlnaDmrUpnpRender.cpp 
            Dlna/DlnaDmrUpnpTransport.cpp 
            Dlna/DlnaDmrVariableContainer.cpp 
            Dlna/DlnaDmrWebServer.cpp 
            Dlna/DlnaDmrXmlDoc.cpp 
            Dlna/DlnaDmrXmlEscape.cpp
            MediaPlayer/Impl/DlnaPlayerImpl.cpp
			MediaPlayer/Impl/Mp3UrlPlayerImpl.cpp
			MediaPlayer/proxy/MediaPlayerProxy.cpp
        )
    set(DLNA_SDK_LIB ixml upnp)
    set(DLNA_INCLUDE_DIR "${SOURCE_DIR}/include/Dlna")
endif()

add_executable(duer_linux
	main.cpp
	${DLNA_SOURCE_FILES}
	)

target_include_directories(duer_linux PUBLIC
    "${SOURCE_DIR}/include"
	"${SOURCE_DIR}/include/MediaPlayer/Impl"
	"${SOURCE_DIR}/include/MediaPlayer/Interface"
	"${SOURCE_DIR}/include/MediaPlayer/Listener"
	"${SOURCE_DIR}/include/MediaPlayer/Proxy"
	"${DLNA_INCLUDE_DIR}"
	"${CMAKE_INCLUDE_PATH}"	
    )

target_link_libraries(duer_linux
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

