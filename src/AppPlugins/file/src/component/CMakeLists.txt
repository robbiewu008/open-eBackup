MESSAGE(STATUS "Current path: ${CMAKE_CURRENT_SOURCE_DIR}")
add_definitions(-fPIC)
file(GLOB_RECURSE component_srcs *.cpp)

add_library(component_obj
    OBJECT
    ${component_srcs}
)

target_include_directories(component_obj
    PRIVATE
    ${PLUGIN_FRAMEWORK_PATH}/inc
    ${PLUGIN_SRC_PATH}
    ${PLUGIN_SRC_PATH}/component
    ${PLUGIN_SRC_PATH}/common
    ${PLUGIN_ROOT_PATH}
    ${OPEN_SRC_PATH}/jsoncpp_rel/include
    ${MODULE_ROOT_PATH}/src
    ${OPEN_SRC_PATH}/boost_rel/include
)

target_link_libraries(component_obj
    PRIVATE
    common_dep
)
