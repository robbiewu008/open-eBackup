MESSAGE("Compiling threadpool code, PATH is ${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB_RECURSE threadpool_test_srcs *.cpp)

add_library(threadpool_test_obj
    OBJECT
    ${threadpool_test_srcs}
)

target_include_directories(threadpool_test_obj
    PUBLIC
    ${MODULE_ROOT_PATH}/src
    ${PLATFORM_PATH}/SecureCLib_rel/include
    ${DT_UTILS_PATH}/llt_stub
    ${OPEN_SRC_PATH}/boost_rel/include
)

target_link_directories(threadpool_test_obj
    PUBLIC
    ${PLATFORM_PATH}/SecureCLib_rel/lib
)

target_link_libraries(threadpool_test_obj
    PUBLIC
    securec
    log_util
)
