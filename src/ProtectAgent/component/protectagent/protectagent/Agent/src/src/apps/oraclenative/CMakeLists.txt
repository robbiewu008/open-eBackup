if(LINUX_COMPILE AND NOT CMAKE_SYSTEM_NAME MATCHES "AIX" AND NOT ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "SunOS")
    aux_source_directory(. SRC_LIST)
    add_library(oraclenativeobj
        OBJECT
        ${SRC_LIST}
    )

    target_link_libraries(oraclenativeobj
        PRIVATE
        safe_cmplib
        agent_platform
    )
endif()