add_subdirectory(app)
#add_subdirectory(appprotect)  # TODO
add_subdirectory(cunitpub)
add_subdirectory(device)
add_subdirectory(host)
add_subdirectory(oracle)
add_subdirectory(oraclenative)
add_subdirectory(restore)
add_subdirectory(vmwarenative)

add_library(testPluginobj
    OBJECT
    DataPathProcessClientTest.cpp
    DataProcessClientHandlerTest.cpp
    ServicePluginTest.cpp
)

target_link_libraries(testPluginobj
    PRIVATE
    safe_cmplib
    agent_platform
)