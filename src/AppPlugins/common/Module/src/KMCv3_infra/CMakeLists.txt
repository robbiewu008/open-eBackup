#编译说明：
#该代码在infrastructure-om/infrastructure中编译使用，DT用例使用Framework/test工程。
 
#1. cmake version，指定cmake版本
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
include(CMakePrintHelpers)
 
#2. project name，指定项目的名称及全局编译选项
project(kmc_archive_test)
add_compile_options(
    -fPIC
    -O2
    -DWSEC_COMPILE_CAC_OPENSSL
    -DWSEC_AES_GCM_SUPPORT
    -DWSEC_USE_OPENSSL_110
    -DWSEC_COMPILE_MIP_OTHER
)
message(${CMAKE_CURRENT_SOURCE_DIR})
message(${CMAKE_CURRENT_SOURCE_DIR}/../../platform/KMCV3_rel/include/)
#3. head file path，头文件目录
INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../platform/SecureCLib_rel/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../platform/SecureCLib_rel/include/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../platform/KMCV3_rel/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../platform/KMCV3_rel/include/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../platform/KMCV3_rel/common/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../platform/KMCV3_rel/lib/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../platform/KMCV3_rel/sdp/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../platform/KMCV3_rel/cac/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../third_open_src/openssl_rel/include
)
 
#4. 编译libkmcv3.so
#4.1 添加KMC编译代码
AUX_SOURCE_DIRECTORY(kmcv3_src KMCV3_SRCS)
message("KMCV3_SRCS::${KMCV3_SRCS}")
SET(KMCV3_CMAKE
    ${KMCV3_SRCS}
)
#4.2 编译目标文件
ADD_LIBRARY(kmcv3 SHARED ${KMCV3_CMAKE})
#4.3 编译选项配置
target_compile_options(kmcv3
    PRIVATE
    -Wl,-z,now
    -Wl,-Bsymbolic
    -Wl,-z,relro
    -Wl,-z,noexecstack
    -fPIC
    -fstack-protector-all
)
#4.4 库文件配置
set(CMAKE_SKIP_RPATH TRUE)
target_link_directories(kmcv3
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../platform/SecureCLib_rel/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../../platform/KMCV3_rel/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../../third_open_src/openssl_rel/lib
)
target_include_directories(kmcv3
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/kmcv3_src
 
)
TARGET_LINK_LIBRARIES(kmcv3
    PRIVATE
    -Wl,--whole-archive
    -Wl,-z,now
    -s
    crypto securec KMC SDP
    -Wl,--no-whole-archive
    dl c
)
 
#5. 设置输出目录
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
