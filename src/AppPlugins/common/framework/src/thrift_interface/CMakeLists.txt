MESSAGE("Compiling thirft interface code, PATH is ${CMAKE_CURRENT_SOURCE_DIR}")

SET(OPENSRC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../third_open_src)
SET(INC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../inc)
SET(EVENT_LIB_BUILD_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../third_open_src/libevent_rel/lib)
SET(THRIFT_LIB_BUILD_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../third_open_src/thrift_rel/lib)

# 递归添加thrift目录下所有的cpp文件
file(GLOB_RECURSE thrift_interface_srcs *.cpp)
add_library(thrift_interface
    SHARED
    ${thrift_interface_srcs}
)

target_include_directories(thrift_interface
    PUBLIC
    ${OPEN_SRC_PATH}/libevent_rel/include
    ${OPEN_SRC_PATH}/thrift_rel/include
    ${OPEN_SRC_PATH}/boost_rel/include
    ${OPEN_SRC_PATH}/openssl_rel/include
    ${INC_PATH}/thrift_interface
)
target_link_directories(thrift_interface
    PUBLIC
    ${OPEN_SRC_PATH}/libevent_rel/lib
    ${OPEN_SRC_PATH}/thrift_rel/lib
    ${OPEN_SRC_PATH}/openssl_rel/lib
)

# 将event相关库以静态方式链接
# if (WIN32 OR MSVC)
#     find_library(EVENT_EXTRA_LIB event_extra.lib ${EVENT_LIB_BUILD_PATH})
#     find_library(EVENT_PTHREADS_LIB event_pthreads.lib ${EVENT_LIB_BUILD_PATH})
#     find_library(EVENT_CORE_LIB event_core.lib ${EVENT_LIB_BUILD_PATH})
#     find_library(EVENT_LIB event.lib ${EVENT_LIB_BUILD_PATH})
#     find_library(THRIFTNB_LIB thriftnb.lib ${THRIFT_LIB_BUILD_PATH})
#     find_library(THRIFTZ_LIB thriftz.lib ${THRIFT_LIB_BUILD_PATH})
#     find_library(THRIFT_LIB thrift.lib ${THRIFT_LIB_BUILD_PATH})
# elseif(UNIX)
#     find_library(EVENT_EXTRA_LIB libevent_extra.a ${EVENT_LIB_BUILD_PATH})
#     find_library(EVENT_PTHREADS_LIB libevent_pthreads.a ${EVENT_LIB_BUILD_PATH})
#     find_library(EVENT_CORE_LIB libevent_core.a ${EVENT_LIB_BUILD_PATH})
#     find_library(EVENT_LIB libevent.a ${EVENT_LIB_BUILD_PATH})
#     find_library(THRIFTNB_LIB libthriftnb.a ${THRIFT_LIB_BUILD_PATH})
#     find_library(THRIFTZ_LIB libthriftz.a ${THRIFT_LIB_BUILD_PATH})
#     find_library(THRIFT_LIB libthrift.a ${THRIFT_LIB_BUILD_PATH})
# endif()

target_link_libraries(thrift_interface
    PUBLIC
    pthread
    # ${EVENT_EXTRA_LIB}
    # ${EVENT_PTHREADS_LIB}
    # ${EVENT_CORE_LIB}
    # ${EVENT_LIB}
    # ${THRIFTNB_LIB}
    # ${THRIFTZ_LIB}
    # ${THRIFT_LIB}
    ${SLIB_THRIFTZ}
    ${SLIB_THRIFTNB}
    ${SLIB_THRIFT}
    ${SLIB_EVENT}
    ssl
)

if ("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "AIX")
target_link_directories(thrift_interface
    PUBLIC
    ${OPEN_SRC_PATH}/libz_rel/lib
    ${OPEN_SRC_PATH}/openssl_rel/lib
)
target_link_libraries(thrift_interface
    PUBLIC
    crypto
    z
)
endif()