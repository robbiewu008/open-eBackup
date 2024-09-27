if(LINUX_COMPILE)
    aux_source_directory(. SRC_LIST)
    add_library(iffclientobj
        OBJECT
        ${SRC_LIST}
    )

    target_link_libraries(iffclientobj
        PRIVATE
        safe_cmplib
        agent_platform
    )

    target_compile_definitions(iffclientobj
        PRIVATE
        SUPPORT_SSL
    )
endif()