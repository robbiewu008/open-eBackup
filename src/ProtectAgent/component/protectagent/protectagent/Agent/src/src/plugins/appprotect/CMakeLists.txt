if(LINUX_COMPILE)
    aux_source_directory(. SRC_LIST)
    add_library(pluginsAppprotectobj
        OBJECT
        ${SRC_LIST}
    )

    target_link_libraries(pluginsAppprotectobj
        PRIVATE
        safe_cmplib
        agent_platform
    )
else()
    add_library(pluginsAppprotectobj
        INTERFACE
    )
endif()