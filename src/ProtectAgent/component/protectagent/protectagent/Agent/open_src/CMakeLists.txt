function(SetStaticLibsPath)
    if (WIN32 OR MSVC)
        set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
    elseif (UNIX)
        set(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
    endif()

    set(CMAKE_PREFIX_PATH 
        ${CMAKE_SOURCE_DIR}/open_src/boost/.libs/lib
        ${CMAKE_SOURCE_DIR}/open_src/thrift/.libs/lib
        ${CMAKE_SOURCE_DIR}/open_src/tinyxml
        ${CMAKE_SOURCE_DIR}/open_src/sqlite
        ${CMAKE_SOURCE_DIR}/open_src/curl/lib/.libs
        ${CMAKE_SOURCE_DIR}/open_src/curl/lib64
        ${CMAKE_SOURCE_DIR}/open_src/openssl/.openssl/lib
        ${CMAKE_SOURCE_DIR}/open_src/snmp++/src/.libs
        ${CMAKE_SOURCE_DIR}/open_src/fcgi/libfcgi/.libs
        ${CMAKE_SOURCE_DIR}/open_src/gperftools/tcmalloc/lib
        ${CMAKE_SOURCE_DIR}/open_src/libuuid/lib
        ${CMAKE_SOURCE_DIR}/open_src/libevent/.libs/lib
        ${CMAKE_SOURCE_DIR}/open_src/jsoncpp/build/lib
        ${CMAKE_SOURCE_DIR}/open_src/tinyxml/build
        ${CMAKE_SOURCE_DIR}/open_src/libuuid/lib
        ${CMAKE_SOURCE_DIR}/open_src/zlib/.libs/lib/
        ${CMAKE_SOURCE_DIR}/test/stubTest/obj/gtestlib
        ${CMAKE_SOURCE_DIR}/platform/kmc/lib
    )

    if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    find_library(SLIB_CURL NAMES curl PATHS ${CMAKE_SOURCE_DIR}/open_src/curl/lib/.libs ${CMAKE_SOURCE_DIR}/open_src/curl/lib64 NO_DEFAULT_PATH)
        find_library(SLIB_SNMP NAMES snmp++)
        find_library(SLIB_FCGI NAMES fcgi)
        find_library(SLIB_SSL NAMES ssl)
        find_library(SLIB_CRYPTO NAMES crypto)
        find_library(SLIB_KMC NAMES KMC)
        find_library(SLIB_SDP NAMES SDP)
        find_library(SLIB_SECUREC NAMES securec)
        find_library(SLIB_THRIFTZ NAMES thriftz)
        find_library(SLIB_THRIFTNB NAMES thriftnb)
        find_library(SLIB_THRIFT NAMES thrift)
        find_library(SLIB_EVENT NAMES event)
        find_library(SLIB_UUID NAMES uuid)
        find_library(SLIB_TCMALLOC NAMES tcmalloc)
        find_library(SLIB_GTEST NAMES gtest)
    elseif (CMAKE_SYSTEM_NAME MATCHES "SunOS")
        find_library(SLIB_CURL NAMES curl PATHS ${CMAKE_SOURCE_DIR}/open_src/curl/lib/.libs NO_DEFAULT_PATH)
        find_library(SLIB_SNMP NAMES snmp++)
        find_library(SLIB_FCGI NAMES fcgi)
        find_library(SLIB_SSL NAMES ssl)
        find_library(SLIB_CRYPTO NAMES crypto)
        find_library(SLIB_KMC NAMES KMC)
        find_library(SLIB_SDP NAMES SDP)
        find_library(SLIB_SECUREC NAMES securec)
        find_library(SLIB_THRIFTZ NAMES thriftz)
        find_library(SLIB_THRIFTNB NAMES thriftnb)
        find_library(SLIB_THRIFT NAMES thrift)
        find_library(SLIB_EVENT NAMES event)
        find_library(SLIB_UUID NAMES uuid PATHS /lib/64 NO_DEFAULT_PATH)
        find_library(SLIB_GTEST NAMES gtest)
    else()
        find_library(SLIB_CURL NAMES curl PATHS ${CMAKE_SOURCE_DIR}/open_src/curl/lib/.libs NO_DEFAULT_PATH)
        find_library(SLIB_SNMP NAMES snmp++)
        find_library(SLIB_FCGI NAMES fcgi)
        find_library(SLIB_SSL NAMES ssl)
        find_library(SLIB_CRYPTO NAMES crypto)
        find_library(SLIB_KMC NAMES KMC)
        find_library(SLIB_SDP NAMES SDP)
        find_library(SLIB_SECUREC NAMES securec)
        find_library(SLIB_THRIFTZ NAMES thriftz)
        find_library(SLIB_THRIFTNB NAMES thriftnb)
        find_library(SLIB_THRIFT NAMES thrift)
        find_library(SLIB_EVENT NAMES event)
        find_library(SLIB_ZLIB NAMES z)
        find_library(SLIB_GTEST NAMES gtest)
    endif()

    message(STATUS "find library curl = " ${SLIB_CURL})
    message(STATUS "find library tcmalloc = " ${SLIB_TCMALLOC})
    message(STATUS "find library event = " ${SLIB_EVENT})
    message(STATUS "find library fcgi = " ${SLIB_FCGI})
    message(STATUS "find library ssl = " ${SLIB_SSL})
    message(STATUS "find library crypto = " ${SLIB_CRYPTO})
    message(STATUS "find library thriftz = " ${SLIB_THRIFTZ})
    message(STATUS "find library thriftnb = " ${SLIB_THRIFTNB})
    message(STATUS "find library thrift = " ${SLIB_THRIFT})
    message(STATUS "find library KMC = " ${SLIB_KMC})
    message(STATUS "find library SDP = " ${SLIB_SDP})
    message(STATUS "find library securec = " ${SLIB_SECUREC})
    message(STATUS "find library uuid = " ${SLIB_UUID})
    message(STATUS "find library libz = " ${SLIB_ZLIB})
    message(STATUS "find LLT gtest = " ${SLIB_GTEST})
endfunction()

function(AddSqliteObj)
    if(PLUGIN_SDK AND NOT ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "SunOS")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
    endif()

    # Separately generate sqlite3.o objects
    add_library(sqliteobj
        OBJECT
        ${CMAKE_SOURCE_DIR}/open_src/sqlite/sqlite3.h
        ${CMAKE_SOURCE_DIR}/open_src/sqlite/sqlite3ext.h
        ${CMAKE_SOURCE_DIR}/open_src/sqlite/sqlite3.c
    )

    target_link_libraries(sqliteobj
        PRIVATE
        safe_cmplib
    )

    target_compile_definitions(sqliteobj
        PRIVATE
        WITH_OPENSSL
    )

    # jsoncpp object
    file(GLOB SRC_LIST "${CMAKE_SOURCE_DIR}/open_src/jsoncpp/src/lib_json/*.cpp")
    add_library(jsoncppobj
        OBJECT
        ${SRC_LIST}
    )

    target_link_libraries(jsoncppobj
        PRIVATE
        safe_cmplib
    )

    # tinyxml2 object
    file(GLOB SRC_LIST "${CMAKE_SOURCE_DIR}/open_src/tinyxml/tiny*.cpp")
    add_library(tinyxml2obj
        OBJECT
        ${SRC_LIST}
    )

    target_link_libraries(tinyxml2obj
        PRIVATE
        safe_cmplib
    )
endfunction()

function(GenerateOpensrcLib)
    # opensrc Static Library
    add_library(agentOpenSrcLib
        STATIC
    )

    target_link_libraries(agentOpenSrcLib
        PUBLIC
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
        ${SLIB_THRIFTZ}
        ${SLIB_THRIFTNB}
        ${SLIB_THRIFT}
        ${SLIB_EVENT}
        ${SLIB_UUID}
        ${SLIB_TCMALLOC}

        # system lib
        pthread
        rt
        dl

        # compile option
        safe_cmpexec
    )
endfunction()

SetStaticLibsPath()

AddSqliteObj()

# AIX do not compile LLT yet
if(LINUX_COMPILE)
    GenerateOpensrcLib()
endif()

