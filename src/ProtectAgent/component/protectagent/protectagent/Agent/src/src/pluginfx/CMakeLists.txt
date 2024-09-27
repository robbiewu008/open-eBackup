if(LINUX_COMPILE)
    if(PLUGIN_SDK)
        find_package(JNI REQUIRED)
        include_directories(${JAVA_INCLUDE_PATH})
        include_directories(${JAVA_INCLUDE_PATH2})

        FILE(GLOB SDK_SRC_LIST ExternalPluginSDK.cpp 
            com_huawei_oceanprotect_sdk_agent_plugin_lib_security_ability_definition_SecurityUtil.cpp
            com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility.cpp)

        add_library(pluginfx_sdk_obj
            OBJECT
            ${SDK_SRC_LIST}
        )

        target_link_libraries(pluginfx_sdk_obj
            PRIVATE
            safe_cmplib
            agent_platform
        )
        if (CMAKE_SYSTEM_NAME MATCHES "Linux")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
        endif()
        return()
    else()
        aux_source_directory(. SRC_LIST)
        list(REMOVE_ITEM SRC_LIST "./ExternalPluginSDK.cpp")
        list(REMOVE_ITEM SRC_LIST "./com_huawei_oceanprotect_sdk_agent_plugin_lib_security_ability_definition_SecurityUtil.cpp")
        list(REMOVE_ITEM SRC_LIST "./com_huawei_oceanprotect_sdk_agent_plugin_lib_archive_ability_definition_ArchiveStreamAbility.cpp")

        add_library(pluginfxobj
            OBJECT
            ${SRC_LIST}
        )
    endif()
else()
    add_library(pluginfxobj
        OBJECT
        PluginCfgParse.cpp
        PluginManager.cpp
    )
endif()

target_link_libraries(pluginfxobj
    PRIVATE
    safe_cmplib
    agent_platform
)

