aux_source_directory(. SRC_LIST)
add_library(testPluginsVmwareNativeobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(testPluginsVmwareNativeobj
    PRIVATE
    safe_cmplib
    agent_platform
)