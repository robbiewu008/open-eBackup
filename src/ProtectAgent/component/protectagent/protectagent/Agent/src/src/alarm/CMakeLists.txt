aux_source_directory(. SRC_LIST)
add_library(alarmobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(alarmobj
    PRIVATE
    safe_cmplib
    agent_platform
)