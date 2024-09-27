aux_source_directory(. SRC_LIST)
add_library(datareadwriteobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(datareadwriteobj
    PRIVATE
    safe_cmplib
    agent_platform
)