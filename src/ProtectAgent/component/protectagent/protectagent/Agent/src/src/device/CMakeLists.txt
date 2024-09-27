aux_source_directory(. SRC_LIST)
add_library(deviceobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(deviceobj
    PRIVATE
    safe_cmplib
    agent_platform
)