aux_source_directory(. SRC_LIST)
add_library(testCurlclientobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(testCurlclientobj
    PRIVATE
    safe_cmplib
    agent_platform
)