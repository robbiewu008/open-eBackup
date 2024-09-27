add_subdirectory(plugininterface)
add_library(testAppprotectobj
    OBJECT
    AppProtectServiceTest.cpp
)

target_link_libraries(testAppprotectobj
    PRIVATE
    safe_cmplib
    agent_platform
)