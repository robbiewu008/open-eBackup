if(LINUX_COMPILE AND NOT CMAKE_SYSTEM_NAME MATCHES "AIX" AND NOT ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "SunOS")
    aux_source_directory(. SRC_LIST)
    add_library(vmwarenativeobj
        OBJECT
        ${SRC_LIST}
    )

    target_link_libraries(vmwarenativeobj
        PRIVATE
        safe_cmplib
        agent_platform
    )
else()
    add_library(vmwarenativeobj
        INTERFACE
    )
endif()