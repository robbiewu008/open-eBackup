aux_source_directory(. SRC_LIST)
add_library(dpvmwarenativeobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(dpvmwarenativeobj
    PRIVATE
    safe_cmplib
    agent_platform
)