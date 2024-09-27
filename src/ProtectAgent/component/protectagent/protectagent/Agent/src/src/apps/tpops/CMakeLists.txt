if(LINUX_COMPILE AND NOT ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "SunOS")
    set(dws_head_list
      "${CMAKE_SOURCE_DIR}/src/inc/apps/dws/XBSAClient"
      "${CMAKE_SOURCE_DIR}/src/inc/apps/dws/XBSAServer"
      "${CMAKE_SOURCE_DIR}/src/inc/apps/dws"
      "${CMAKE_SOURCE_DIR}/src/inc/xbsaclientcomm"
      "${CMAKE_SOURCE_DIR}/src/inc/xbsa")
    include_directories(${dws_head_list})

    add_subdirectory(XBSAClient)
    add_subdirectory(XBSACom)
    add_subdirectory(XBSAServer)
endif()