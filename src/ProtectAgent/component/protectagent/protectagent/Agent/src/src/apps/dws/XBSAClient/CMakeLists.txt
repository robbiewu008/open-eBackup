if(LINUX_COMPILE AND NOT ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "SunOS")
    aux_source_directory(. SRC_LIST)
    add_library(xbsaclientobj
        OBJECT
        ${SRC_LIST}
    )

    target_link_libraries(xbsaclientobj
        PRIVATE
        safe_cmplib
        agent_platform
    )


    target_compile_definitions(xbsaclientobj
        PRIVATE
        SUPPORT_SSL
    )
endif()