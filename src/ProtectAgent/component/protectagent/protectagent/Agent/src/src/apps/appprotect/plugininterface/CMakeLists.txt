if(LINUX_COMPILE)
    aux_source_directory(. SRC_LIST)
    add_library(plugininterfaceobj
        OBJECT
        ${SRC_LIST}
    )
    set(plugininterface_include_list
      "${CMAKE_SOURCE_DIR}/src/inc/apps/appprotect/plugininterface")
    include_directories(${plugininterface_include_list})

    target_link_libraries(plugininterfaceobj
        PRIVATE
        safe_cmplib
        agent_platform
    )
else()
    # AIX add empty flag
    add_library(plugininterfaceobj
        INTERFACE
    )
endif()