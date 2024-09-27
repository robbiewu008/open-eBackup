aux_source_directory(. SRC_LIST)
add_library(testOracleobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(testOracleobj
    PRIVATE
    safe_cmplib
    agent_platform
)