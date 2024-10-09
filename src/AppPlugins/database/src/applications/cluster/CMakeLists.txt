file(GLOB_RECURSE cluster_obj_srcs *.cpp)
add_library(cluster_obj
    OBJECT
    ${cluster_obj_srcs}
)

MESSAGE("${MODULE_BASE_PATH}")
target_include_directories(cluster_obj
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../common
    ${CMAKE_CURRENT_SOURCE_DIR}/../../executor
    ${MODULE_THRIFT_INC_PATH}
    ${MODULE_OPENSRC_PATH}/jsoncpp_rel/include
    ${MODULE_OPENSRC_PATH}/boost_rel/include
    ${FRAMEWORK_INC_PATH}/common
    ${MODULE_BASE_PATH}/src
    ${MODULE_BASE_PATH}/src/common
    ${FRAMEWORK_INC_PATH}/job
    ${AGENT_SDK_PATH}/include
    ${MODULE_SECUREC_INC_PATH}
)

target_link_libraries(cluster_obj
    PRIVATE
    database_inc
)