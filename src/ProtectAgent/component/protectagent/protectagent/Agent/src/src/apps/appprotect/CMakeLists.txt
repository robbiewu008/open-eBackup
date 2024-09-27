add_subdirectory(plugininterface)
if(LINUX_COMPILE)
    add_library(appprotectobj
        OBJECT
        AppProtectService.cpp
    )

    target_link_libraries(appprotectobj
        PRIVATE
        safe_cmplib
        agent_platform
    )
else()
    add_library(appprotectobj
        INTERFACE
    )
endif()