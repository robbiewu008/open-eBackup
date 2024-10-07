MESSAGE(STATUS "Current path: ${CMAKE_CURRENT_SOURCE_DIR}")
add_definitions(-fPIC)
file(GLOB_RECURSE plugin_srcs *.cpp)

add_library(application_obj
    OBJECT
    ${plugin_srcs}
)

target_include_directories(application_obj
    PRIVATE
    ${PLUGIN_FRAMEWORK_PATH}/inc
    ${PLUGIN_SRC_PATH}/common/constant
    ${PLUGIN_SRC_PATH}/common/application
    ${MODULE_ROOT_PATH}/src
    ${PLUGIN_ROOT_PATH}
    ${OPENSRC_THRIFT_INC_PATH}
    ${OPEN_SRC_PATH}/boost_rel/include
    ${OPEN_SRC_PATH}/jsoncpp_rel/include
)

target_link_libraries(application_obj
    PRIVATE
    common_dep
)
