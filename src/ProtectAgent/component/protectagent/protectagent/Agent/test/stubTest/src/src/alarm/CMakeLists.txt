aux_source_directory(. SRC_LIST)
add_library(testAlarmobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(testAlarmobj
    PRIVATE
    safe_cmplib
    agent_platform
)