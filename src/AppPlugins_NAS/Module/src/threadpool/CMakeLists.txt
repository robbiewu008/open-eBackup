file(GLOB_RECURSE threadpool_srcs *.cpp)
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../build/cmake/global.cmake)
set(OPENSRC_INC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../third_open_src")
set(BOOST_PATH "${OPENSRC_INC_PATH}/boost_rel")

MESSAGE(${BOOST_PATH})

add_library(threadpool
    SHARED
    ${threadpool_srcs}
)
if ("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
    target_compile_options(threadpool
        PUBLIC
        "-fstack-protector-strong" "-Wl,-z,now" "-Wl,-z,relro"
        "-fstack-check" "-D_FORTIFY_SOURCE=2 -O2" "-flto" "-freg-struct-return" "-s"
    )
endif()
target_include_directories(threadpool
    PRIVATE
    ${BOOST_PATH}/include/
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/
    ${CMAKE_CURRENT_SOURCE_DIR}/../log
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

MESSAGE("${BOOST_PATH}/lib")
target_link_directories(threadpool
    PUBLIC
    ${BOOST_PATH}/lib/
)

target_link_libraries(threadpool
    PUBLIC
    log_util
    safe_cmplib
    log_util
    boost_chrono
    boost_atomic
    boost_thread
    boost_filesystem
    boost_system
    boost_thread
    boost_log
    boost_date_time
)