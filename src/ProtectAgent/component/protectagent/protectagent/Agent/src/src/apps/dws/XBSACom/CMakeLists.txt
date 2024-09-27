if(LINUX_COMPILE AND NOT ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "SunOS")
    aux_source_directory(. SRC_LIST)
    add_library(xbsacomobj
        OBJECT
        ${SRC_LIST}
    )

    target_link_libraries(xbsacomobj
        PRIVATE
        safe_cmplib
        agent_platform
    )

    target_compile_definitions(xbsacomobj
        PRIVATE
        SUPPORT_SSL
    )
endif()