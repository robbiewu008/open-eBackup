MESSAGE("Compiling plugin main code, PATH is ${CMAKE_CURRENT_SOURCE_DIR}")

SET(INC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../inc)
SET(DME_SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../dme_src)
if (NOT DEFINED EXEC_FILE_NAME)
    SET(EXEC_FILE_NAME AgentPlugin)
endif()

# 递归添加thrift目录下所有的cpp文件
file(GLOB_RECURSE plugin_main_srcs *.cpp)

add_executable(${EXEC_FILE_NAME}
    ${plugin_main_srcs}
)

target_include_directories(${EXEC_FILE_NAME}
    PUBLIC
    ${INC_PATH}/startup
    ${INC_PATH}/rpc
    ${INC_PATH}/common
    ${INC_PATH}/rpc/certificateservice
    ${INC_PATH}/rpc/thriftservice
    ${INC_PATH}/rpc/servicefactory
    ${INC_PATH}/job_mgr
    ${INC_PATH}/job
    ${INC_PATH}/utils
    ${MODULE_PATH}
    ${AGENT_SDK_PATH}/include
    ${PLATFORM_PATH}/SecureCLib_rel/include
)

target_link_directories(${EXEC_FILE_NAME}
    PUBLIC
    ${AGENT_SDK_PATH}/lib
    ${OPEN_SRC_PATH}/openssl_rel/lib
    ${MODULE_ROOT_PATH}/lib
)

target_link_libraries(${EXEC_FILE_NAME}
    PUBLIC
    safe_cmpexec
    pthread
    stdc++
    thrift_interface
    pluginsdk
    job_mgr_obj
    rpc
    ssl
    crypto
    param_checker
    utils
)

if ("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
    target_link_directories(${EXEC_FILE_NAME}
        PUBLIC
        ${OPEN_SRC_PATH}/icu_rel/libs/lib
        ${KMC_LIB_PATH}
    )

    target_link_libraries(${EXEC_FILE_NAME}
        PUBLIC
        ${KMC_LIB_FILES}
        boost_chrono
        boost_atomic
        boost_thread
        boost_filesystem
        boost_system
        boost_thread
        boost_regex
        boost_log
        boost_date_time
        icudata
        icuuc
        icui18n
    )
endif()