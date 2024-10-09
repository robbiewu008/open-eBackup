MESSAGE("Enter into executor, start compling.")
file(GLOB_RECURSE executor_obj_srcs *.cpp)

add_library(executor_obj
    OBJECT
    ${executor_obj_srcs}
)

target_include_directories(executor_obj
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../common
    ${CMAKE_CURRENT_SOURCE_DIR}/../job
    ${MODULE_THRIFT_INC_PATH}
    ${FRAMEWORK_INC_PATH}
    ${MODULE_SECUREC_INC_PATH}
    ${FRAMEWORK_INC_PATH}/common
    ${FRAMEWORK_INC_PATH}/job
    ${FRAMEWORK_BASE_PATH}/src/inc
    ${FRAMEWORK_INC_PATH}/rpc
    ${FRAMEWORK_INC_PATH}/rpc/certificateservice
    ${MODULE_BASE_PATH}/src
    ${MODULE_BASE_PATH}/common
    ${MODULE_BASE_PATH}/src/common
    ${MODULE_OPENSRC_PATH}/boost_rel/include
    ${AGENT_SDK_PATH}/include
    ${MODULE_SECUREC_INC_PATH}
)

target_link_libraries(executor_obj
    PRIVATE
    database_inc
)