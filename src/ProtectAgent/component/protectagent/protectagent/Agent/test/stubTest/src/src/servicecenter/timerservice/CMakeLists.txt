aux_source_directory(. SRC_LIST)
add_library(testTimerServiceobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(testTimerServiceobj
    PRIVATE
    safe_cmplib
    agent_platform
)