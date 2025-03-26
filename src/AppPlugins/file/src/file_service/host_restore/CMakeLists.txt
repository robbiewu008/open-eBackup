if ("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
    add_library(host_restore_obj
        OBJECT
        ${CMAKE_CURRENT_SOURCE_DIR}/HostRestore.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/OsRestore.cpp
    )
else()
    add_library(host_restore_obj
        OBJECT
        ${CMAKE_CURRENT_SOURCE_DIR}/HostRestore.cpp
    )
endif()

target_include_directories(host_restore_obj
    PRIVATE
    ${PLUGIN_SRC_PATH}
    ${PLUGIN_FRAMEWORK_PATH}/inc/job/
    ${PLUGIN_FRAMEWORK_PATH}/inc/thrift_interface
    ${PLUGIN_FRAMEWORK_PATH}/inc
    ${PLUGIN_FRAMEWORK_PATH}/inc/client
    ${PLUGIN_FRAMEWORK_PATH}/inc/common
    ${PLUGIN_FRAMEWORK_PATH}/inc/rpc
    ${PLUGIN_FRAMEWORK_PATH}/inc/rpc/certificateservice


    ${PLUGIN_SRC_PATH}/utils


)

target_link_libraries(host_restore_obj
    PRIVATE
    host_obj
    common_dep
)