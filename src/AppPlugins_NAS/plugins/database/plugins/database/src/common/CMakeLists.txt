file(GLOB_RECURSE sqlserver_obj_srcs *.cpp)
add_library(common_obj
    OBJECT
    ${sqlserver_obj_srcs}
)

MESSAGE("${MODULE_BASE_PATH}/src/define")
target_include_directories(common_obj
    PUBLIC
    ${MODULE_BASE_PATH}/src
    ${MODULE_SECUREC_INC_PATH}
    ${MODULE_OPENSRC_PATH}/boost_rel/include
    ${AGENT_SDK_PATH}/include
    ${MODULE_SECUREC_INC_PATH}
)

target_link_libraries(common_obj
    PRIVATE
    database_inc
)