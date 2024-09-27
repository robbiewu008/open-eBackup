aux_source_directory(. SRC_LIST)
add_library(testXbsaobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(testXbsaobj
    PRIVATE
    safe_cmplib
    agent_platform
)