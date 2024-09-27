MESSAGE("Compiling log, PATH is ${CMAKE_CURRENT_SOURCE_DIR}")
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../build/cmake/global.cmake)
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../build/cmake/path.cmake)
SET(MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/..)
SET(PLATFORM ${MODULE_PATH}/../platform)
# 查看目标库是否存在
if(TARGET log_util)
    MESSAGE(STATUS "log_util existed")
else()
    add_subdirectory(${MODULE_PATH}/log log_util)
endif()
if(TARGET config_reader_util)
    MESSAGE(STATUS "config_reader_util existed")
else()
    add_subdirectory(${MODULE_PATH}/config_reader config_reader_util)
endif()

add_library(system
    SHARED
    System.cpp
    basesystem/basesystem.cpp
)
target_include_directories(system
    PRIVATE
    ${PLATFORM}/SecureCLib_rel/include
    ${OPEN_SRC}/boost_rel/include

    ${MODULE_PATH}
)

target_link_directories(system
    PRIVATE
    ${PLATFORM}/SecureCLib_rel/lib
    ${OPEN_SRC}/boost_rel/lib
    )
target_link_libraries(system
    PRIVATE
    safe_cmplib
    securec
    boost_chrono
    boost_atomic
    boost_thread
    boost_filesystem
    boost_system
    boost_regex
    boost_log
    boost_date_time
    log_util
    config_reader_util
)