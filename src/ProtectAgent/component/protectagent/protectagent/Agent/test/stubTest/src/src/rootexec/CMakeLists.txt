aux_source_directory(. SRC_LIST)
add_library(testRootexecobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(testRootexecobj
    PRIVATE
    safe_cmplib
    agent_platform
)