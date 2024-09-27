if(LINUX_COMPILE)
    aux_source_directory(. SRC_LIST)
    add_library(pluginsDwsobj
        OBJECT
        ${SRC_LIST}
    )

    set(dwsplugin_head_list
    "${CMAKE_SOURCE_DIR}/src/inc/apps/dws/XBSAClient"
    "${CMAKE_SOURCE_DIR}/src/inc/apps/dws/XBSAServer"
    "${CMAKE_SOURCE_DIR}/src/inc/apps/dws")
    include_directories(${dwsplugin_head_list})

    target_link_libraries(pluginsDwsobj
        PRIVATE
        safe_cmplib
        agent_platform
    )

    target_compile_definitions(pluginsDwsobj
        PRIVATE
        SUPPORT_SSL
    )
else()
    add_library(pluginsDwsobj
        INTERFACE
    )
endif()