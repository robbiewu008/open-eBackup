aux_source_directory(. SRC_LIST)
add_library(testVmwareNativeobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(testVmwareNativeobj
    PRIVATE
    safe_cmplib
    agent_platform
)