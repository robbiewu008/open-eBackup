aux_source_directory(. SRC_LIST)
add_library(serviceTimerServiceobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(serviceTimerServiceobj
    PRIVATE
    safe_cmplib
    agent_platform
)