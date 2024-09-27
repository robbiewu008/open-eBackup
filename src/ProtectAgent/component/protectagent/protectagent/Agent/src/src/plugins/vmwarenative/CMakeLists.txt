if(LINUX_COMPILE)
    aux_source_directory(. SRC_LIST)
    add_library(pluginsVmwareNativeobj
        OBJECT
        ${SRC_LIST}
    )

    target_link_libraries(pluginsVmwareNativeobj
        PRIVATE
        safe_cmplib
        agent_platform
    )
else()
    add_library(pluginsVmwareNativeobj
        INTERFACE
    )
endif()