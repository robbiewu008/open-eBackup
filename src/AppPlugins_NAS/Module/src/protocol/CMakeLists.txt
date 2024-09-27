SET(MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/..)
SET(MODULE_ROOT_PATH ${MODULE_PATH}/..)
set(CMAKE_CXX_STANDARD 14)
MESSAGE("Start generate proto code")
execute_process(COMMAND sh ${MODULE_ROOT_PATH}/build/make-gencode.sh ${MODULE_ROOT_PATH}/build RESULT_VARIABLE GEN_RC)

if (${GEN_RC} EQUAL 0)
    message("generate code success")
else()
    message( FATAL_ERROR "generate code failed, rc:" ${GEN_RC} )
endif()
add_library(protocol
        SHARED
        ${MODULE_ROOT_PATH}/var/protocol/HeartBeatCommand.pb.cc
        ${MODULE_ROOT_PATH}/var/protocol/EnvironmentProxy.pb.cc
        ${MODULE_ROOT_PATH}/var/protocol/BackupNodeReplication.pb.cc
        ${MODULE_ROOT_PATH}/var/protocol/BackupNodeManager.pb.cc
        ${MODULE_ROOT_PATH}/var/protocol/BasicTypes.pb.cc
        ${MODULE_ROOT_PATH}/var/protocol/BasicCommands.pb.cc
        ${MODULE_ROOT_PATH}/var/protocol/Tasks.pb.cc
        ${MODULE_ROOT_PATH}/var/protocol/EventCommand.pb.cc
        ${MODULE_ROOT_PATH}/var/protocol/CommandTypes.pb.cc
        ${MODULE_ROOT_PATH}/var/protocol/StatisticCommand.pb.cc
        ${MODULE_ROOT_PATH}/var/protocol/ChainEngine.pb.cc
        ${MODULE_ROOT_PATH}/var/protocol/ManageBricksConnectivity.pb.cc
        )

target_include_directories(protocol
        PUBLIC
        ${MODULE_ROOT_PATH}/var
        ${MODULE_ROOT_PATH}/var/protocol
        ${MODULE_ROOT_PATH}/third_open_src/protobuf_rel/include/
        )

target_link_directories(protocol
        PUBLIC
        ${MODULE_ROOT_PATH}/third_open_src/protobuf_rel/lib/
        )

target_link_libraries(protocol
        PRIVATE
        protoc
        safe_cmplib
        )