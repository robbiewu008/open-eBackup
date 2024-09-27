add_subdirectory(libvmfs)
add_subdirectory(libvmfs6)

aux_source_directory(. SRC_LIST)
add_library(ioschedulerobj
    OBJECT
    ${SRC_LIST}
)

target_link_libraries(ioschedulerobj
    PRIVATE
    safe_cmplib
    agent_platform
    vmfsobj
    vmfs6obj
)