include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PLUGIN_FRAMEWORK_INC}
    ${PROTECT_ROOT_DIR}/framework/dep/agent_sdk/include/message/archivestream
    ${PROTECT_ROOT_DIR}/framework/dep/agent_sdk/include
)

add_subdirectory(job_controller)
add_subdirectory(protect_engines)
add_subdirectory(repository_handlers)
add_subdirectory(common)
add_subdirectory(volume_handlers)