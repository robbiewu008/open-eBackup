aux_source_directory(. SRC_LIST)
add_library(testDataprocessobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(testDataprocessobj
    PRIVATE
    safe_cmplib
    agent_platform
)