file(GLOB_RECURSE general_obj_srcs *.cpp)
add_library(general_obj
    OBJECT
    ${general_obj_srcs}
)

target_include_directories(general_obj
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../common
    ${CMAKE_CURRENT_SOURCE_DIR}/../../executor
    ${FRAMEWORK_INC_PATH}/common
    ${FRAMEWORK_INC_PATH}/thrift_interface
    ${FRAMEWORK_INC_PATH}/rpc
    ${FRAMEWORK_INC_PATH}/rpc/certificateservice
    ${FRAMEWORK_INC_PATH}
    ${FRAMEWORK_INC_PATH}/job
    ${MODULE_THRIFT_INC_PATH}
    ${MODULE_BASE_PATH}/src
    ${MODULE_BASE_PATH}/src/common
    ${MODULE_OPENSRC_PATH}/boost_rel/include
    ${AGENT_SDK_PATH}/include
    ${MODULE_SECUREC_INC_PATH}
)

target_link_libraries(general_obj
    PRIVATE
    database_inc
)