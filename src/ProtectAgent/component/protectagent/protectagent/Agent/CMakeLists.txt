cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
project(agent)

include(build/cmake/global.cmake)
add_definitions(-w)
# agent opensrc
if(LINUX_COMPILE)
    include_directories(${CMAKE_SOURCE_DIR}/open_src/libuuid/include/)
    include_directories(${CMAKE_SOURCE_DIR}/open_src/libuuid/include/uuid)
    include_directories(${CMAKE_SOURCE_DIR}/open_src/gperftools/tcmalloc/include)
    include_directories(${CMAKE_SOURCE_DIR}/open_src/boost/.libs/include)
    include_directories(${CMAKE_SOURCE_DIR}/open_src/libevent/.libs/include/)
    include_directories(${CMAKE_SOURCE_DIR}/open_src/thrift/.libs/include)
endif()
include_directories(${CMAKE_SOURCE_DIR}/open_src/sqlite)
include_directories(${CMAKE_SOURCE_DIR}/open_src)
include_directories(${CMAKE_SOURCE_DIR}/open_src/openssl/include)
include_directories(${CMAKE_SOURCE_DIR}/open_src/snmp++/include)
include_directories(${CMAKE_SOURCE_DIR}/open_src/snmp++)
include_directories(${CMAKE_SOURCE_DIR}/open_src/fcgi/include)
include_directories(${CMAKE_SOURCE_DIR}/open_src/jsoncpp/include)
include_directories(${CMAKE_SOURCE_DIR}/open_src/tinyxml)
include_directories(${CMAKE_SOURCE_DIR}/open_src/curl/include/)
include_directories(${CMAKE_SOURCE_DIR}/open_src/zlib/.libs/include)
include_directories(${CMAKE_SOURCE_DIR}/test/stubTest/src/gtest/include)
include_directories(${CMAKE_SOURCE_DIR}/test/stubTest/src/gtest)
include_directories(${CMAKE_SOURCE_DIR}/obj/libsecurec/include)
include_directories(${CMAKE_SOURCE_DIR}/platform/kmc/include)
include_directories(${CMAKE_SOURCE_DIR}/platform/kmc)
include_directories(${CMAKE_SOURCE_DIR}/platform/kmc/src/sdp)
include_directories(${CMAKE_SOURCE_DIR}/platform/securec/include)

# agent source headers
include_directories(${CMAKE_SOURCE_DIR}/src/inc)
include_directories(${CMAKE_SOURCE_DIR}/src/inc/servicecenter)
include_directories(${CMAKE_SOURCE_DIR}/src/src)
include_directories(${CMAKE_SOURCE_DIR}/src/inc/apps/appprotect/plugininterface)
include_directories(${CMAKE_SOURCE_DIR}/src/inc/apps/dws/XBSACom)
include_directories(${CMAKE_SOURCE_DIR}/src/inc/apps/dws/XBSAServer)
include_directories(${CMAKE_SOURCE_DIR}/src/inc/apps/dws)
include_directories(${CMAKE_SOURCE_DIR}/src/inc/apps/appprotect/plugininterface)
if(LLT)
    include_directories(${CMAKE_SOURCE_DIR}/test/stubTest/src/gtest/include)
    include_directories(${CMAKE_SOURCE_DIR}/test/stubTest/src/stub)
    include_directories(${CMAKE_SOURCE_DIR}/test/stubTest/src/src)
    include_directories(${CMAKE_SOURCE_DIR}/test/stubTest/src/inc/servicecenter)
    include_directories(${CMAKE_SOURCE_DIR}/test/stubTest/src/inc)
    include_directories(${CMAKE_SOURCE_DIR}/test/stubTest/src/inc/apps/appprotect/plugininterface)
    include_directories(${CMAKE_SOURCE_DIR}/test/stubTest/src/inc/apps/dws/XBSAServer)
endif()

# AIX/Linux public open source library
link_directories(${CMAKE_SOURCE_DIR}/open_src/curl/lib/.libs/)
link_directories(${CMAKE_SOURCE_DIR}/open_src/fcgi/libfcgi/.libs/)
link_directories(${CMAKE_SOURCE_DIR}/open_src/openssl/.openssl/lib/)
link_directories(${CMAKE_SOURCE_DIR}/platform/kmc/lib/)
link_directories(${CMAKE_SOURCE_DIR}/open_src/snmp++/src/.libs)

add_subdirectory(src)
add_subdirectory(open_src)
add_subdirectory(platform)

if(LLT)
    message(STATUS "Begin compile LLT.")
    add_subdirectory(test)
endif()

###########################################################################
##################### Generate Dynamic Library ############################
###########################################################################
function(GenDynamicLib)
    # libcommon.so
    add_library(common
        SHARED
    )

    target_link_libraries(common
        PUBLIC
        commonobj
        sqliteobj

        jsoncppobj
        tinyxml2obj

        # opensrc
        ${SLIB_CURL}
        ${SLIB_SNMP}
        ${SLIB_FCGI}
        ${SLIB_SSL}
        ${SLIB_CRYPTO}
        ${SLIB_KMC}
        ${SLIB_SDP}
        ${SLIB_SECUREC}
        ${SLIB_EVENT}
        ${SLIB_UUID}
        ${SLIB_ZLIB}

        # system library
        dl
        pthread
        rt

        # compile option
        safe_cmplib
    )

    # libsecurecom.so
    add_library(securecom
        SHARED
    )

    target_link_libraries(securecom
        PUBLIC
        securecomobj
        restobj
        tcpsslobj
        arrayobj

        common

        # opensrc
        ${SLIB_TCMALLOC}

        # system lib
        pthread
        rt
        dl

        # compile option
        safe_cmplib
    )

    # libhost-${AGENT_BUILD_NUM}.so
    add_library(${HOST}
        SHARED
    )

    target_link_libraries(${HOST}
        PUBLIC
        hostobj
        pluginobj
        pluginfxobj
        plugininterfaceobj
        pluginsHostobj
        curlclientobj
        taskmanagerobj
        datamessageobj
        alarmobj

        ebkservicecenter
        securecom
        common
        ${SLIB_TCMALLOC}
        ${SLIB_THRIFTZ}
        ${SLIB_THRIFTNB}
        ${SLIB_THRIFT}

        # system lib
        rt

        # compile option
        safe_cmplib
    )

    # libdevice-${AGENT_BUILD_NUM}.so
    add_library(${DEVICE}
        SHARED
    )

    target_link_libraries(${DEVICE}
        PUBLIC
        deviceobj
        pluginobj
        datamessageobj

        securecom
        common
        ${SLIB_TCMALLOC}

        # system lib
        rt

        # compile option
        safe_cmplib
    )

    # liboracle-${AGENT_BUILD_NUM}.so
    add_library(${ORACLE}
        SHARED
    )

    target_link_libraries(${ORACLE}
        PUBLIC
        oracleobj
        pluginobj
        pluginsOracleobj
        datamessageobj
        alarmobj
        curlclientobj

        securecom
        common
        ${SLIB_TCMALLOC}

        # compile option
        safe_cmplib
    )

    # libapp-${AGENT_BUILD_NUM}.so
    add_library(${APP}
        SHARED
    )

    target_link_libraries(${APP}
        PUBLIC
        oracleobj
        pluginobj
        datamessageobj
        pluginsAppobj
        appobj
        alarmobj
        hostobj
        curlclientobj

        securecom
        common
        ${SLIB_TCMALLOC}

        # compile option
        safe_cmplib
    )

if(LINUX_COMPILE AND NOT CMAKE_SYSTEM_NAME MATCHES "AIX" AND NOT ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "SunOS")
    # liboraclenative-${AGENT_BUILD_NUM}.so
    add_library(${ORACLENATIVE}
        SHARED
    )

    target_link_libraries(${ORACLENATIVE}
        PUBLIC
        oraclenativeobj
        pluginobj
        pluginsOracleNativeobj
        datamessageobj
        alarmobj
        curlclientobj
        plugininterfaceobj
        pluginfxobj
        oracleobj
        taskmanagerobj
        deviceobj
        hostobj

        ebkservicecenter
        securecom
        common
        ${SLIB_TCMALLOC}

        # compile option
        safe_cmplib
    )

    # libvmwarenative-${AGENT_BUILD_NUM}.so
    add_library(${VMWARENATIVE}
        SHARED
    )

    target_link_libraries(${VMWARENATIVE}
        PUBLIC
        pluginobj
        pluginsVmwareNativeobj
        curlclientobj
        datamessageobj
        alarmobj
        plugininterfaceobj
        pluginfxobj

        taskmanagerobj
        hostobj
        vmwarenativeobj

        ebkservicecenter
        securecom
        common
        ${SLIB_TCMALLOC}

        # compile option
        safe_cmplib
    )
endif()

    # libappprotect-${AGENT_BUILD_NUM}.so
    add_library(${APPPROTECT}
        SHARED
    )

    target_link_libraries(${APPPROTECT}
        PUBLIC
        pluginobj
        pluginfxobj
        pluginsAppprotectobj
        plugininterfaceobj
        alarmobj
        appprotectobj
        taskmanagerobj
        hostobj
        datamessageobj
        curlclientobj

        ebkservicecenter
        securecom
        common
        ${SLIB_TCMALLOC}

        ${SLIB_THRIFTZ}
        ${SLIB_THRIFTNB}
        ${SLIB_THRIFT}
        # compile option
        safe_cmplib
    )

if(LINUX_COMPILE AND NOT ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "SunOS")
    # libxbsa64.so
    add_library(xbsa64
        SHARED
    )

    target_link_libraries(xbsa64
        PUBLIC
        xbsaclientobj
        xbsacomobj
        xbsaclientcomobj
        tcpsslobj
        archivestreamobj

        securecom
        common
        ${SLIB_TCMALLOC}
        ${SLIB_THRIFTZ}
        ${SLIB_THRIFTNB}
        ${SLIB_THRIFT}

        # compile option
        safe_cmplib
    )

    # libxbsa64iif.so
    add_library(xbsa64iif
        SHARED
    )

    target_link_libraries(xbsa64iif
        PUBLIC
        iffclientobj
        xbsacomobj
        xbsaclientcomobj
        tcpsslobj
        archivestreamobj

        securecom
        common
        ${SLIB_TCMALLOC}
        ${SLIB_THRIFTZ}
        ${SLIB_THRIFTNB}
        ${SLIB_THRIFT}

        # compile option
        safe_cmplib
    )

    # libdws-${AGENT_BUILD_NUM}.so
    add_library(${DWS}
        SHARED
    )

    target_link_libraries(${DWS}
        PUBLIC
        archivestreamobj
        pluginobj
        pluginsDwsobj
        xbsacomobj
        xbsaclientcomobj
        pluginfxobj
        plugininterfaceobj
        curlclientobj
        taskmanagerobj
        datamessageobj
        alarmobj

        hostobj
        xbsaserverobj

        ebkservicecenter
        securecom
        common
        ${SLIB_TCMALLOC}
        ${SLIB_THRIFTZ}
        ${SLIB_THRIFTNB}
        ${SLIB_THRIFT}

        # compile option
        safe_cmplib
    )
endif()
endfunction()

###########################################################################
##################### Generate Executable Binary ##########################
###########################################################################
function(GenExecutable)
    # rdagent
    add_executable(rdagent)
    target_link_libraries(rdagent
        rdagentobj
        plugininterfaceobj
        appprotectobj
        pluginfxobj
        pluginobj
        pluginsHostobj
        curlclientobj
        datamessageobj
        alarmobj
        taskmanagerobj

        hostobj

        ebkservicecenter
        common
        securecom
        ${SLIB_TCMALLOC}
        ${SLIB_THRIFTZ}
        ${SLIB_THRIFTNB}
        ${SLIB_THRIFT}

        # system lib
        pthread
        rt
        dl

        # compile option
        safe_cmpexec
    )

if(LINUX_COMPILE)
    # rootexec[Linux -static-stdc++
    # rootexec cannot use the LD_LIBRARY_PATH variable due to privilege escalation
    add_executable(rootexec)
    target_link_libraries(rootexec
        rootexecobj
        commonobj
        securecomobj
        curlclientobj
        alarmobj
        tcpsslobj
        arrayobj
        sqliteobj

        jsoncppobj
        tinyxml2obj

        ${SLIB_CURL}
        ${SLIB_SNMP}
        ${SLIB_FCGI}
        ${SLIB_SSL}
        ${SLIB_CRYPTO}
        ${SLIB_KMC}
        ${SLIB_SDP}
        ${SLIB_SECUREC}
        ${SLIB_UUID}
        ${LIB_STD}
        ${LIB_GCC}
        ${SLIB_ZLIB}

        # system lib
        pthread
        rt
        dl

        # compile option
        safe_cmpexec
    )
else()
    message(STATUS "AGENT_DEPLOYMENT_DIR = " ${AGENT_DEPLOYMENT_DIR})
    # AIX libgcc
    add_executable(rootexec)
    target_link_libraries(rootexec
        rootexecobj
        commonobj
        securecomobj
        curlclientobj
        alarmobj
        tcpsslobj
        arrayobj
        sqliteobj

        jsoncppobj
        tinyxml2obj

        ${SLIB_CURL}
        ${SLIB_SNMP}
        ${SLIB_FCGI}
        ${SLIB_SSL}
        ${SLIB_CRYPTO}
        ${SLIB_KMC}
        ${SLIB_SDP}
        ${SLIB_SECUREC}
        ${SLIB_UUID}
        ${AGENT_DEPLOYMENT_DIR}/libstdc++.a
        ${AGENT_DEPLOYMENT_DIR}/libgcc_s.a

        # system lib
        pthread
        rt
        dl

        # compile option
        safe_cmpexec
    )
endif()

    # ${SLIB_CRYPTO}
    add_executable(crypto)
    target_link_libraries(crypto
        cryptoobj

        common
        securecom
        ${SLIB_TCMALLOC}

        # system lib
        pthread
        rt
        dl

        # compile option
        safe_cmpexec
    )

    # scriptsign
    add_executable(scriptsign)
    target_link_libraries(scriptsign
        scriptsignobj

        common
        securecom
        ${SLIB_TCMALLOC}

        # system lib
        pthread
        rt
        dl

        # compile option
        safe_cmpexec
    )

    # monitor
    add_executable(monitor)
    target_link_libraries(monitor
        monitorobj
        curlclientobj
        alarmobj

        common
        securecom
        ${SLIB_TCMALLOC}

        # system lib
        pthread
        rt
        dl

        # compile option
        safe_cmpexec
    )

    # xmlcfg
    add_executable(xmlcfg)
    target_link_libraries(xmlcfg
        xmlcfgobj

        common
        securecom
        ${SLIB_TCMALLOC}

        # system lib
        pthread
        rt
        dl

        # compile option
        safe_cmpexec
    )

    # agentcli
    add_executable(agentcli)
    target_link_libraries(agentcli
        agentcliobj
        curlclientobj
        archivestreamobj
        alarmobj

        hostobj

        # ebkservicecenter
        common
        securecom
        ${SLIB_TCMALLOC}

        # system lib
        pthread
        rt
        dl

        # compile option
        safe_cmpexec
    )

    # getinput
    add_executable(getinput)
    target_link_libraries(getinput
        getinputobj

        common
        securecom
        ${SLIB_TCMALLOC}

        # system lib
        pthread
        rt
        dl

        # compile option
        safe_cmpexec
    )

if(LINUX_COMPILE AND NOT CMAKE_SYSTEM_NAME MATCHES "AIX" AND NOT ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "SunOS")
    # dataprocess
    add_executable(dataprocess)
    target_link_libraries(dataprocess
        tcpobj

        ebkdataprocess
        common
        ${SLIB_TCMALLOC}
        ${LIB_STD}

        # system lib
        pthread
        rt
        dl

        # compile option
        safe_cmpexec
    )
endif()

endfunction()

# geneate sdk library file
function(GenPluginSDKLib)
    # libpluginsdk.so
    add_library(pluginsdk
        # STATIC
        SHARED
    )

    target_link_libraries(pluginsdk
        PUBLIC
        commonobj
        securecomobj
        tcpsslobj
        jsoncppobj
        sqliteobj
        archivestreamobj
        pluginfx_sdk_obj
        tinyxml2obj

        ${LIB_STD}
        ${SLIB_SSL}
        ${SLIB_CRYPTO}
        ${SLIB_KMC}
        ${SLIB_SDP}
        ${SLIB_UUID}
        ${SLIB_SECUREC}
        # compile option
        safe_cmplib
        pthread
    )
    target_compile_definitions(pluginsdk
        PUBLIC
        -Wl,-Bsymbolic
    )
    target_link_options(pluginsdk
        PUBLIC
        -Wl,-s
    )
endfunction()

if(PLUGIN_SDK)
    GenPluginSDKLib()
    set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
else()
    GenDynamicLib()

    if(LLT)
        message( STATUS "LLT mode, not generate executable object.")
    else()
        GenExecutable()
    endif()

    if(LINUX_COMPILE)
        set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
        set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
    endif()
endif()


