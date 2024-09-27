aux_source_directory(. SRC_LIST)
add_library(testTcpobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(testTcpobj
    PRIVATE
    safe_cmplib
    agent_platform
)