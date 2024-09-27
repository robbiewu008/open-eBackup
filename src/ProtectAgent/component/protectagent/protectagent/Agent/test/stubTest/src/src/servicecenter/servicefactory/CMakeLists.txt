aux_source_directory(. SRC_LIST)
add_library(testServiceFactoryobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(testServiceFactoryobj
    PRIVATE
    safe_cmplib
    agent_platform
)