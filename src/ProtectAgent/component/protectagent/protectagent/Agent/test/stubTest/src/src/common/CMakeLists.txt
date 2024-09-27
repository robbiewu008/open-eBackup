aux_source_directory(. SRC_LIST)
add_library(testCommonobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(testCommonobj
    PRIVATE
    safe_cmplib
    agent_platform
)