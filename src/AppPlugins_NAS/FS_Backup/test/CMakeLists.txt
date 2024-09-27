cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
include(CMakePrintHelpers)
project(backup_test)
add_definitions(-w)
add_definitions(-D_NAS)
add_definitions(-D_OBS)
add_definitions(-DBOOST_ASIO_DISABLE_STD_ALIGNED_ALLOC)

# 全局参数配置
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(BACKUP_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/..)
SET(BACKUP_SRC_PATH ${BACKUP_ROOT_PATH}/src)
SET(DT_UTILS_PATH ${MODULE_ROOT_PATH}/dt_utils)
SET(GOOGLE_TEST_PATH ${DT_UTILS_PATH}/gmock/googletest-release)
SET(MOCKCPP_PATH ${DT_UTILS_PATH}/mockcpp/build/mockcpp)
SET(MODULE_SRC_PATH ${MODULE_ROOT_PATH}/src)
SET(OPEN_SRC_PATH ${MODULE_ROOT_PATH}/third_open_src)
SET(PLATFORM_PATH ${MODULE_ROOT_PATH}/platform)

# 定义全局的编译选项
add_compile_options(-fPIC)
add_compile_options(-fno-access-control)
SET(LLT ON)

aux_source_directory(src src_test)
aux_source_directory(src/common common_test)
aux_source_directory(src/common/backup_layout common_backup_layout_test)
aux_source_directory(src/engines/archive_client archive_client_test)
aux_source_directory(src/engines/libsmb libsmb_test)
aux_source_directory(src/engines/libnfs libnfs_test)
aux_source_directory(src/engines/libnfs/interface libnfs_interface_test)
aux_source_directory(src/engines/libnfs/requests request_test)
aux_source_directory(src/engines/libsmb/interface interface_test)
aux_source_directory(src/engines/posix posix_test)
aux_source_directory(src/service/copy copy_test)
aux_source_directory(src/service/delete delete_test)
aux_source_directory(src/service/dir dir_test)
aux_source_directory(src/service/hardlink hardlink_test)
aux_source_directory(src/service/antiransomware antiransomware_test)
aux_source_directory(src/engines/object object_test)
aux_source_directory(src/bare_metal_recovery bare_metal_recovery_test)

add_executable(backup_test
    ${src_test}
    ${common_test}
    ${common_backup_layout_test}
    ${archive_client_test}
    ${libsmb_test}
    ${libnfs_test}
    ${libnfs_interface_test}
    ${request_test}
    ${interface_test}
    ${copy_test}
    ${delete_test}
    ${dir_test}
    ${hardlink_test}
    ${posix_test}
    ${object_test}
    ${antiransomware_test}
    ${bare_metal_recovery_test}
)

add_subdirectory(../ backup_src)
# 定义gtest的使用
add_library(gtest_lib_interface
    INTERFACE
    IMPORTED
    GLOBAL
)

target_include_directories(gtest_lib_interface
    INTERFACE
    # google test folder
    ${GOOGLE_TEST_PATH}/googletest/include
    ${GOOGLE_TEST_PATH}/googlemock/include

    # llt stub folder
    ${DT_UTILS_PATH}
    ${DT_UTILS_PATH}/llt_stub

    # mockcpp
    ${MOCKCPP_PATH}/include
)

target_link_directories(gtest_lib_interface
    INTERFACE
    ${GOOGLE_TEST_PATH}/lib

    # mockcpp
    ${MOCKCPP_PATH}/lib
)

target_link_libraries(gtest_lib_interface
    INTERFACE
    gmock
    gtest
    gtest_main
    gmock_main
    gcov
    mockcpp
)

target_include_directories(backup_test
    PUBLIC
    # src folder
    ${BACKUP_SRC_PATH}
    ${BACKUP_SRC_PATH}/model
    ${BACKUP_SRC_PATH}/engines
    ${BACKUP_SRC_PATH}/engines/posix
    ${BACKUP_SRC_PATH}/engines/libnfs
    ${BACKUP_SRC_PATH}/engines/libnfs/interface
    ${BACKUP_SRC_PATH}/engines/libnfs/requests
    ${BACKUP_SRC_PATH}/engines/archive_client
    ${BACKUP_SRC_PATH}/engines/object
    ${BACKUP_SRC_PATH}/bare_metal_recovery
    ${BACKUP_SRC_PATH}/engines/hostcommon
    ${BACKUP_SRC_PATH}/service
    ${BACKUP_SRC_PATH}/service/copy
    ${BACKUP_SRC_PATH}/service/delete
    ${BACKUP_SRC_PATH}/service/hardlink
    ${BACKUP_SRC_PATH}/service/antiransomware
    ${BACKUP_SRC_PATH}/service/dir
    ${BACKUP_SRC_PATH}/common/backup_layout
    ${OPEN_SRC_PATH}/boost_rel/include
    ${OPEN_SRC_PATH}/esdk_rel/include
    ${OPEN_SRC_PATH}/lnfs_rel/include
    ${OPEN_SRC_PATH}/lsmb2_rel/include
    ${OPEN_SRC_PATH}/acl_rel/include
    ${OPEN_SRC_PATH}/sqlite_rel/sqlite-autoconf/include
    ${OPEN_SRC_PATH}/7z_rel/include
    ${OPEN_SRC_PATH}/7z_rel/include/CPP
    ${OPEN_SRC_PATH}/7z_rel/include/CPP/7zip
    ${OPEN_SRC_PATH}/7z_rel/include/CPP/include_windows
    ${OPEN_SRC_PATH}/jsoncpp_rel/include
    ${PLATFORM_PATH}/SecureCLib_rel/include

    ${MODULE_SRC_PATH}/
    ${MODULE_SRC_PATH}/libnfs_ctx
    ${MODULE_SRC_PATH}/libobs_ctx
    ${MODULE_SRC_PATH}/threadpool
)


target_link_directories(backup_test
    PRIVATE
    ${OPEN_SRC_PATH}/boost_rel/lib
    ${OPEN_SRC_PATH}/esdk_rel/lib
    ${OPEN_SRC_PATH}/openssl_rel/lib
    ${OPEN_SRC_PATH}/icu_rel/libs/lib
    ${OPEN_SRC_PATH}/libiconv_rel/lib
    ${PLATFORM_PATH}/SecureCLib_rel/lib
    ${BACKUP_ROOT_PATH}/build-cmake
    ${OPEN_SRC_PATH}/lnfs_rel/lib
    ${OPEN_SRC_PATH}/lsmb2_rel/lib
    ${OPEN_SRC_PATH}/jsoncpp_rel/libs
    ${OPEN_SRC_PATH}/acl_rel/lib
    ${MODULE_ROOT_PATH}/lib
)

set(THREADS_PREFER_PTHRED_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(backup_test
    pthread
    Threads::Threads

    # 开源库
    boost_chrono
    boost_atomic
    boost_thread
    boost_filesystem
    boost_system
    boost_regex
    boost_log
    boost_date_time
    icudata
    icui18n
    icuuc
    jsoncpp
    uuid
    iconv

    ssl
    crypto
    nfs
    smb2
    gtest_lib_interface
    # 源码的obj
    log_util
    common_util
    threadpool
    parser
    Backup
    securec
    config_reader_util
    system

    smb_ctx
    nfs_ctx
    obs_ctx
)
