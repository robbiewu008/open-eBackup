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
    ${CMAKE_CURRENT_SOURCE_DIR}/../
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../framework/inc/job/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../framework/inc/thrift_interface
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../framework/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../framework/inc/client
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../framework/inc/common
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../framework/inc/rpc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../framework/inc/rpc/certificateservice


    ${PLUGIN_SRC_PATH}/utils


)

target_link_libraries(host_restore_obj
    PRIVATE
    host_obj
    common_dep
)