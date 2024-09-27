aux_source_directory(. SRC_LIST)
add_library(testOracleNativeobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(testOracleNativeobj
    PRIVATE
    safe_cmplib
    agent_platform
)