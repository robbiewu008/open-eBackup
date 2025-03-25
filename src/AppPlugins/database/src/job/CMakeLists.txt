file(GLOB_RECURSE job_obj_srcs *.cpp)

add_library(job_obj
    OBJECT
    ${job_obj_srcs}
)
MESSAGE("framework_path:${FRAMEWORK_INC_PATH}")
target_include_directories(job_obj
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../applications/cluster
    ${CMAKE_CURRENT_SOURCE_DIR}/../applications/general
    ${CMAKE_CURRENT_SOURCE_DIR}/../common
    ${CMAKE_CURRENT_SOURCE_DIR}/../executor
    ${FRAMEWORK_INC_PATH}/job
    ${FRAMEWORK_INC_PATH}/rpc
    ${FRAMEWORK_INC_PATH}/common
    ${FRAMEWORK_INC_PATH}/rpc/certificateservice
    ${FRAMEWORK_INC_PATH}/thrift_interface
    ${FRAMEWORK_INC_PATH}
    ${MODULE_BASE_PATH}/src
    ${MODULE_BASE_PATH}/common
    ${MODULE_THRIFT_INC_PATH}
    ${MODULE_OPENSRC_PATH}/boost_rel/include
    ${MODULE_OPENSRC_PATH}/jsoncpp_rel/include
    ${AGENT_SDK_PATH}/include
    ${MODULE_SECUREC_INC_PATH}
)
target_link_libraries(job_obj
    PRIVATE
    safe_cmplib
    # database_inc
)