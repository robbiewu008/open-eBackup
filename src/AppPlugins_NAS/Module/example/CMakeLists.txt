cmake_minimum_required(VERSION 3.0)
project(App VERSION 1.0)

SET(MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../src)

# 全量
# add_subdirectory(${MODULE_PATH} output_src)
# 可选
add_subdirectory(${MODULE_PATH}/common output_common)
add_subdirectory(${MODULE_PATH}/log output_log)
add_subdirectory(${MODULE_PATH}/config_reader output_config_reader)

link_libraries(
    log_util
    # kmc_util
    common_util
    config_reader_util
)

add_executable(test
    main.cpp
)

target_include_directories(test
    PUBLIC
    ${MODULE_PATH}
    ${MODULE_PATH}/define
    ${MODULE_PATH}/log
    ${MODULE_PATH}/config_reader
)

target_link_libraries(test
    PUBLIC
    stdc++
)

install(
    TARGETS test log_util config_reader_util
    LIBRARY DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/lib
    ARCHIVE DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/lib
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin
)