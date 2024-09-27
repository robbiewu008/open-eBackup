if(LINUX_COMPILE)
    aux_source_directory(. SRC_LIST)
    add_library(xbsaclientcomobj
        OBJECT
        ${SRC_LIST}
    )

    target_link_libraries(xbsaclientcomobj
        PRIVATE
        safe_cmplib
        agent_platform
    )

    target_compile_definitions(xbsaclientcomobj
        PRIVATE
        SUPPORT_SSL
    )
endif()