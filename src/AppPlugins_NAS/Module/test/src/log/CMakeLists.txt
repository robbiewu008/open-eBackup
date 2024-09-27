MESSAGE("Compiling config_reader code, PATH is ${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB_RECURSE config_reader_test_srcs *.cpp)

add_library(log_test_obj
    OBJECT
    ${config_reader_test_srcs}
)

target_include_directories(log_test_obj
    PUBLIC
    ${MODULE_ROOT_PATH}/src
    ${PLATFORM_PATH}/SecureCLib_rel/include
    ${DT_UTILS_PATH}/llt_stub
)

target_link_directories(log_test_obj
    PUBLIC
    ${PLATFORM_PATH}/SecureCLib_rel/lib
)

target_link_libraries(log_test_obj
    PUBLIC
    securec
    log_util
)
