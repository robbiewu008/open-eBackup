aux_source_directory(. SRC_LIST)
add_library(oracleobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(oracleobj
    PRIVATE
    safe_cmplib
    agent_platform
)