aux_source_directory(. SRC_LIST)
add_library(testArrayobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(testArrayobj
    PRIVATE
    safe_cmplib
    agent_platform
)