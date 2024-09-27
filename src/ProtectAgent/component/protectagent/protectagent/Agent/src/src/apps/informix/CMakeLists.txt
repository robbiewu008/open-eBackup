if(LINUX_COMPILE)
    set(informix_head_lists
      "${CMAKE_SOURCE_DIR}/src/inc/xbsaclientcomm"
      "${CMAKE_SOURCE_DIR}/src/inc/xbsa")
    include_directories(${informix_head_lists})

    add_subdirectory(XBSAClient)
endif()