MESSAGE("Compiling parser code, PATH is ${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB_RECURSE parser_test_srcs *.cpp)

add_library(parser_test_obj
    OBJECT
    ${parser_test_srcs}
)

target_include_directories(parser_test_obj
    PUBLIC
    ${MODULE_ROOT_PATH}/src
    ${MODULE_ROOT_PATH}/src/parser
    ${PLATFORM_PATH}/SecureCLib_rel/include
    ${DT_UTILS_PATH}/llt_stub
    ${OPEN_SRC_PATH}/boost_rel/include
)

target_link_directories(parser_test_obj
    PUBLIC
    ${PLATFORM_PATH}/SecureCLib_rel/lib
)

target_link_libraries(parser_test_obj
    PUBLIC
    securec
    log_util
)
