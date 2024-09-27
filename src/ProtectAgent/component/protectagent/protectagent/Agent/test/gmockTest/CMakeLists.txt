cmake_minimum_required(VERSION 3.15)

# NAME
project(AgentTest)

add_subdirectory(./googletest EXCLUDE_FROM_ALL)

# source file lists
aux_source_directory(./llt COMMON_SRC_LIST)
aux_source_directory(./llt/common COMMON_SRC_LIST)
aux_source_directory(../../open_src/jsoncpp/src/lib_json COMMON_SRC_LIST)
aux_source_directory(../../src/src/common COMMON_SRC_LIST)
aux_source_directory(../../src/src/message/rest COMMON_SRC_LIST)
aux_source_directory(../../src/src/message/tcp COMMON_SRC_LIST)
aux_source_directory(../../src/src/plugins COMMON_SRC_LIST)
aux_source_directory(../../platform/securec/src COMMON_SRC_LIST)

aux_source_directory(./llt/oraclenativebackup ORA_NATIVE_SRC_LIST)
aux_source_directory(../../src/src/host ORA_NATIVE_SRC_LIST)
aux_source_directory(../../src/src/array ORA_NATIVE_SRC_LIST)
aux_source_directory(../../src/src/alarm ORA_NATIVE_SRC_LIST)
aux_source_directory(../../src/src/taskmanager ORA_NATIVE_SRC_LIST)
aux_source_directory(../../src/src/apps/oracle ORA_NATIVE_SRC_LIST)
aux_source_directory(../../src/src/plugins/oraclenative ORA_NATIVE_SRC_LIST)
aux_source_directory(../../src/src/apps/oraclenative ORA_NATIVE_SRC_LIST)

Message(${COMMON_SRC_LIST})
Message(${ORA_NATIVE_SRC_LIST})

# include files
include_directories("./googletest/googlemock/include" "./googletest/googletest/include" "../../src/inc" "../../open_src") 
include_directories("../../open_src/openssl/include" "../../platform/securec/include" "../../platform/kmc" "../../platform/kmc/include") 
include_directories("../../open_src/tinyxml/" "../../open_src/jsoncpp/include" "../../open_src/snmp++/include", "../../open_src/snmp++")
include_directories("../../open_src/sqlite/")

# link files directory
link_directories("../../open_src/openssl" "../../open_src/snmp++/src/.libs" "../../open_src/curl/lib/.libs" "../../open_src/fcgi/libfcgi/.libs")
link_directories("../../platform/kmc/lib" "../../open_src/sqlite" "../../open_src/tinyxml")

# line file
link_libraries(curl sqlite3 tinyxml2 KMC snmp++ fcgi uuid crypto ssl pthread dl)

# execute file
add_executable(AgentCommonTest ${COMMON_SRC_LIST})
add_executable(AgentOracleNativeTest ${COMMON_SRC_LIST} ${ORA_NATIVE_SRC_LIST})

# link file
target_link_libraries(AgentCommonTest gtest_main gmock_main)
target_link_libraries(AgentOracleNativeTest gtest_main gmock_main)

# add test
# add_test(NAME AgentCommonTest COMMAND AgentCommonTest)

set(CMAKE_CXX_FLAGS   "-std=c++11")             # c++11
set(CMAKE_CXX_FLAGS   "-g")                     # 调试信息
set(CMAKE_CXX_FLAGS   "-Wall")                  # 开启所有警告
set(CMAKE_CXX_FLAGS_DEBUG   "-O0" )             # 调试包不优化
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG" )   # release包优化
set(CMAKE_CXX_FLAGS   "-DLINUX -DWITH_OPENSSL -DWSEC_COMPILE_CAC_OPENSSL -DWSEC_OPENSSL101 -DWSEC_LINUX -DKMC_MULTIPROCESS -DWSEC_ERR_CODE_BASE=0")
set(CMAKE_C_COMPILER g++)

