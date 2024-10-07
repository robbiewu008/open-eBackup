/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#include <iostream>
#include <sstream>
#include "utils/PluginConfig.h"
#include "log/Log.h"
#include "common/Path.h"
#include "common/JsonUtils.h"
#include "common/EnvVarManager.h"
#include "RPCInterface.h"
#include "PluginThriftServer.h"
#include "PluginThriftClient.h"
#include "ExternalPluginSDK.h"
#include "config_reader/ConfigIniReader.h"

using namespace GeneralDB;
namespace {
const mp_string GENERALDB_RPC_TOOL_LOG = "dbrpctool.log";
const mp_int32 TOOL_BIN_PATH = 0;
const mp_int32 INTERFACE_NAME = 1;
const mp_int32 INPUT_FILE_PATH = 2;
const mp_int32 OUTPUT_FILE_PATH = 3;
const mp_int32 LOG_PATH = 4;
const mp_int32 THRIFT_SERVER_PORT = 5;
const mp_int32 TOOL_ARG_MAX = 6;

struct RPCToolStartParam {
    mp_string toolBinPath;
    mp_string interfaceName;
    mp_string inputFilePath;
    mp_string outputFilePath;
    mp_string logPath;
    mp_string thriftServerPort;
};

mp_int32 GetArgs(mp_int32 argc, mp_char** argv, RPCToolStartParam &startParam)
{
    if (argc != TOOL_ARG_MAX) {
        return MP_FAILED;
    }
    startParam.toolBinPath = argv[TOOL_BIN_PATH];
    startParam.interfaceName = argv[INTERFACE_NAME];
    startParam.inputFilePath = argv[INPUT_FILE_PATH];
    startParam.outputFilePath = argv[OUTPUT_FILE_PATH];
    startParam.logPath = argv[LOG_PATH];
    startParam.thriftServerPort = argv[THRIFT_SERVER_PORT];
    return MP_SUCCESS;
}
}

/*------------------------------------------------------------
Function Name: main函数
               Usage: dbrpctool [-r <RPC_Interface>] [-i <Input_File>] [-o <Output_File>] [-l <Log_Path>]
                   [-t <Thrift_Server_Port>]
               -r <RPC_Interface>               RPC接口名称
               -i <Input_File>                  RPC接口输入参数文件
               -o <Output_File>                 RPC接口输出参数文件
               -l <Log_Path>                    RPC工具日志路径
               -t <Thrift_Server_Port>          thrift服务端口
-------------------------------------------------------- */
mp_int32 main(mp_int32 argc, mp_char** argv)
{
    RPCToolStartParam startParam;
    mp_int32 ret = GetArgs(argc, argv, startParam);
    if (ret != MP_SUCCESS) {
        std::cout <<"Get args failed.\n";
        return ret;
    }
    if (startParam.interfaceName.empty() || startParam.inputFilePath.empty() || startParam.outputFilePath.empty() ||
        startParam.thriftServerPort.empty()) {
        printf("Usage: dbrpctool [-r <RPC_Interface>] [-i <Input_File>] [-o <Output_File>] [-l <Log_Path>] \
            [-t <Thrift_Server_Port>]\n");
        return MP_FAILED;
    }
    // 初始化日志文件
    Module::CPath::GetInstance().Init(startParam.toolBinPath);
    mp_int32 iLogLevel = Module::ConfigReader::getInt("General", "LogLevel");
    mp_int32 iLogCount = Module::ConfigReader::getInt("General", "LogCount");
    mp_int32 iLogMaxSize = Module::ConfigReader::getInt("General", "LogMaxSize");
    Module::CLogger::GetInstance().Init(
        GENERALDB_RPC_TOOL_LOG.c_str(), startParam.logPath, iLogLevel, iLogCount, iLogMaxSize);
    INFOLOG("Interface=%s, InputFile=%s, OutputFile=%s.", startParam.interfaceName.c_str(),
        startParam.inputFilePath.c_str(), startParam.outputFilePath.c_str());
    Module::ConfigReader::SetUpdateCnf(false);  // rpctool不能刷新配置文件，防止进程退出，配置文件刷新失败，导致配置文件损坏
    // 设置为外置Agent
#ifdef INTERNAL_PLUGIN
    PluginConfig::GetInstance().m_scene = PluginUsageScene::INTERNAL;
    Module::ConfigReader::ReadLogLevelFromPM(true);
#else
    PluginConfig::GetInstance().m_scene = PluginUsageScene::EXTERNAL;
#endif

#ifdef WIN32
    const mp_string PROTECT_CLIENT_PATH = R"(\DataBackup\ProtectClient\ProtectClient-E)";
#else
    const mp_string PROTECT_CLIENT_PATH = "/DataBackup/ProtectClient/ProtectClient-E";
#endif
    string protectClientEPath = Module::EnvVarManager::GetInstance()->GetAgentHomePath() + PROTECT_CLIENT_PATH;
    OpaInitialize(protectClientEPath);
    uint32_t agentThriftPort = static_cast<uint32_t>(atoi(startParam.thriftServerPort.c_str()));
    INFOLOG("Agent thrift server port is : %d.", agentThriftPort);
    // 设置thrift客户端ip及服务端端口
    startup::PluginThriftClientConfig::GetInstance().Configure("127.0.0.1", agentThriftPort);
    // 调用RPC接口
    RPCInterface rpcInterface;
    return rpcInterface.Call(startParam.interfaceName, startParam.inputFilePath, startParam.outputFilePath);
}
