aux_source_directory(. SRC_LIST)
add_library(jobqosmanagerobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(jobqosmanagerobj
    PRIVATE
    safe_cmplib
    agent_platform
)