aux_source_directory(. SRC_LIST)
add_library(testPluginsOracleNativeobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(testPluginsOracleNativeobj
    PRIVATE
    safe_cmplib
    agent_platform
)