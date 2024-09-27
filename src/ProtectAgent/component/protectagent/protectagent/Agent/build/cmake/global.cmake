# check the compilation version.
message( STATUS "CMAKE_C_COMPILER_VERSION = " ${CMAKE_C_COMPILER_VERSION})
message( STATUS "CMAKE_CXX_COMPILER_VERSION = " ${CMAKE_CXX_COMPILER_VERSION})
if (${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS "7.3.0")
    message(FATAL_ERROR "Insufficient gcc version, C=${CMAKE_C_COMPILER_VERSION},CXX=${CMAKE_CXX_COMPILER_VERSION}, Agent requires at least 7.3.0")
endif ()

# Initialization parameters
option(DP "Compile in DP mode" OFF)
option(XBSA "Compile in XBSA mode" OFF)
option(AGENT "Compile in AGENT mode" OFF)
option(LLT "Compile in LLT mode" OFF)
option(REST_PUBLISH "Compile in REST_PUBLISH mode" OFF)
option(PLUGIN_SDK "Compile in PLUGIN_SDK mode" OFF)

# Secure Compilation Options
set(NX_OPT "-Wl,-z,noexecstack")
set(SP_OPT "-fstack-protector-strong")
set(RELRO_OPT "-Wl,-z,relro")
set(RPATH_OPT "-Wl,--disable-new-dtags")
set(BIND_NOW_OPT "-Wl,-z,now")
set(PIE_OPT "-pie")

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    option(LINUX_COMPILE "Compile in Linux mode" ON)
    set(LIB_STD "-static-libstdc++")

    set(OS_NAME "$ENV{OS_NAME}")
    add_library(safe_cmplib INTERFACE)
    target_compile_options(safe_cmplib INTERFACE -fPIC -pipe ${SP_OPT})
    target_link_options(safe_cmplib INTERFACE ${RELRO_OPT} ${RPATH_OPT} ${BIND_NOW_OPT} ${NX_OPT})

    add_library(safe_cmpexec INTERFACE)
    target_compile_options(safe_cmpexec INTERFACE -fPIC -pipe ${SP_OPT})
    target_link_options(safe_cmpexec INTERFACE -fpie ${RELRO_OPT} ${RPATH_OPT} ${BIND_NOW_OPT} ${NX_OPT} -pie)
    add_definitions("-s -Wall -Wconversion -fpic -fstack-protector-strong -Wextra -Wfloat-equal")
    add_definitions(-DLINUX -DFRAME_SIGN -DSTDCXX_98_HEADERS -D${OS_NAME})

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_definitions(-DDEBUG -g)
    endif()

    if($ENV{FREEZE_SUPPORT})
        add_definitions(-DLIN_FRE_SUPP)
    endif()

    if(REST_PUBLISH)
        add_definitions(-DREST_PUBLISH)
    endif()
elseif (CMAKE_SYSTEM_NAME MATCHES "AIX")
    message(STATUS "AIX compile.")
    option(LINUX_COMPILE "Compile in Linux mode" ON)
    set(LIB_STD "-static-libstdc++")
    set(LIB_GCC "-static-libgcc")

    # special treatment: set agent deployment dir
    set(AGENT_DEPLOYMENT_DIR "/opt/DataBackup/ProtectClient/ProtectClient-E/bin")

    set(OS_NAME "$ENV{OS_NAME}")
    add_library(safe_cmplib INTERFACE)
    target_compile_options(safe_cmplib INTERFACE -fPIC -pipe -Wl,-b64 -shared)
    target_link_options(safe_cmplib INTERFACE -Wl,-bbigtoc)

    add_library(safe_cmpexec INTERFACE)
    target_compile_options(safe_cmpexec INTERFACE -fPIC -pipe -Wl,-b64 -shared)
    target_link_options(safe_cmpexec INTERFACE -fpie -Wl,-bbigtoc -pie)
    add_definitions("-s -Wall -Wconversion -fpic -Wextra -Wfloat-equal")
    add_definitions(-DAIX -DFRAME_SIGN -DSTDCXX_98_HEADERS)

    set (CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} -maix64 -pthread")
    set (CMAKE_CXX_FLAGS   "${CMAKE_CXX_FLAGS} -maix64 -pthread")

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_definitions(-DDEBUG -g)
    endif()
elseif (CMAKE_SYSTEM_NAME MATCHES "SunOS")
    set(LIB_STD "-static-libstdc++")
    set(LIB_GCC "-static-libgcc")
    message(STATUS "SunOS compile.")
    option(LINUX_COMPILE "Compile in Linux mode" ON)
    add_library(safe_cmplib INTERFACE)
    add_library(safe_cmpexec INTERFACE)
    set(AGENT_DEPLOYMENT_DIR "/opt/DataBackup/ProtectClient/ProtectClient-E/bin")
    target_compile_options(safe_cmplib INTERFACE -fPIC -pipe -Wl,-b64 -shared)
    target_link_options(safe_cmpexec INTERFACE -fpie -lsocket -lnsl)
    target_link_options(safe_cmplib INTERFACE -fpie -lsocket -lnsl)
    add_definitions(-DSOLARIS -DFRAME_SIGN -DSTDCXX_98_HEADERS)
endif()

if(LLT)
    target_compile_options(safe_cmplib INTERFACE --coverage)
    target_link_options(safe_cmplib INTERFACE --coverage)
    target_compile_options(safe_cmpexec INTERFACE --coverage)
    target_link_options(safe_cmpexec INTERFACE --coverage)
#    add_definitions(-fno-access-control)
endif()

# stanard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set plugin dynamic library name
set(AGENT_VERSION "$ENV{AGENT_BUILD_NUM}")
set(HOST "host-${AGENT_VERSION}")
set(DEVICE "device-${AGENT_VERSION}")
set(ORACLE "oracle-${AGENT_VERSION}")
set(RESTORE "restore-${AGENT_VERSION}")
set(CLUSTER "cluster-${AGENT_VERSION}")
set(APP "app-${AGENT_VERSION}")
set(ORACLENATIVE "oraclenative-${AGENT_VERSION}")
set(VMWARENATIVE "vmwarenative-${AGENT_VERSION}")
set(APPPROTECT "appprotect-${AGENT_VERSION}")
set(DWS "dws-${AGENT_VERSION}")
