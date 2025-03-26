cmake_minimum_required(VERSION 3.14)
project(FS_Scanner)
MESSAGE(STATUS "Current config path: ${CMAKE_CURRENT_SOURCE_DIR}")
# 查看目标库是否存在
if(TARGET safe_cmplib)
    MESSAGE(STATUS "safe_cmplib existed")
else()
    include(${CMAKE_CURRENT_SOURCE_DIR}/build/cmake/global.cmake)
endif()

if(FILE OR NAS OR OBS)
    # 打开编译告警
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    add_subdirectory(localhost_src)
    add_library(Scanner
        SHARED
    )
    if (FILE)
        if ("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "SunOS")
            target_link_libraries(Scanner
                PRIVATE
                sec
            )
        endif()
    endif()
    if (NAS)
        add_definitions(-D NAS_SNAPDIFF)
        target_link_libraries(Scanner
            PRIVATE
            kmcv3
        )
    endif()
    if (OBS)
        target_link_libraries(Scanner
            PRIVATE
            obs_src_obj
        )
    endif()
    target_link_libraries(Scanner
        PRIVATE
        common_obj
        interface_obj
        module_dep
        service_obj
        utils_obj
        common_util
        opensrc_dep
        stats_obj
        filter_obj
        checkpnt_obj
    )
else()
    link_libraries(
        safe_cmplib
        dme_all
        framework_lib
    )
   add_subdirectory(src)
   add_library(Scanner
    SHARED
   )

    target_link_libraries(Scanner
        PRIVATE
        interface_obj
        config_obj
        parser_obj
        scanner_obj
    )

    add_executable(NasScanner
    )

    target_link_libraries(NasScanner
        executable_obj
        interface_obj
        config_obj
        parser_obj
        scanner_obj
    )
endif()