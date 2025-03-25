add_subdirectory(app)
add_subdirectory(host)
add_subdirectory(appprotect)
add_subdirectory(dws)
add_subdirectory(vmwarenative)

add_library(pluginobj
    OBJECT
    DataPathProcessClient.cpp
    DataProcessClientHandler.cpp
    ServicePlugin.cpp
)

target_link_libraries(pluginobj
    PRIVATE
    safe_cmplib
    agent_platform
)