aux_source_directory(. SRC_LIST)
add_library(pluginsOracleNativeobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(pluginsOracleNativeobj
    PRIVATE
    safe_cmplib
    agent_platform
)