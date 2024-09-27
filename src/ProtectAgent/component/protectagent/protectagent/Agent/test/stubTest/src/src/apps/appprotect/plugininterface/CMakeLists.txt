aux_source_directory(. SRC_LIST)
add_library(testPluginInterfaceobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(testPluginInterfaceobj
    PRIVATE
    safe_cmplib
    agent_platform
)